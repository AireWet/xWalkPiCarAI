#!/usr/bin/env python3
"""Host tests for the xWalk tagged-trace scanner and XML generator."""

from __future__ import annotations

from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ElementTree


SCRIPT_PATH = (
    Path(__file__).resolve().parents[1]
    / "pre-compiler"
    / "xHal_Rpi5CarTracePreCompiler.py"
)
sys.path.insert(0, str(SCRIPT_PATH.parent))
import xHal_Rpi5CarTracePreCompiler as TRACE_GENERATOR  # noqa: E402


class XWalkTracePreCompilerTest(unittest.TestCase):
    """Exercises tokenization, validation, uniqueness, and XML preservation."""

    def test_valid_hal_and_ctrl_identifiers(self) -> None:
        source = """
XWALK_HAL_TRACE_UID0(RPI.1001, "HAL ready");
XWALK_CTRL_TRACE_UID3(CTRL.2001, "CTRL value: %u", value);
"""
        traces = TRACE_GENERATOR.scan_source(source, "src/sample.cpp")
        self.assertEqual([trace.uid for trace in traces], ["RPI.1001", "CTRL.2001"])
        self.assertEqual([trace.priority for trace in traces], [0, 3])

    def test_invalid_identifier_forms(self) -> None:
        invalid_uids = ["RPI.Camera", "CTRL.20A1", "OTHER.1001", "RPI.", "CTRL."]
        for uid in invalid_uids:
            with self.subTest(uid=uid), self.assertRaises(TRACE_GENERATOR.ScannerError):
                TRACE_GENERATOR.scan_source(
                    f'XWALK_HAL_TRACE_UID0({uid}, "invalid");', "invalid.cpp"
                )

    def test_component_tag_mismatch(self) -> None:
        cases = [
            'XWALK_HAL_TRACE_UID0(CTRL.1001, "invalid");',
            'XWALK_CTRL_TRACE_UID0(RPI.2001, "invalid");',
        ]
        for source in cases:
            with self.subTest(source=source), self.assertRaisesRegex(
                TRACE_GENERATOR.ScannerError, "trace macros require"
            ):
                TRACE_GENERATOR.scan_source(source, "mismatch.cpp")

    def test_leading_zero_policy(self) -> None:
        traces = TRACE_GENERATOR.scan_source(
            'XWALK_HAL_TRACE_UID1(RPI.001, "valid");\n'
            'XWALK_CTRL_TRACE_UID1(CTRL.0, "valid");',
            "leading-zero.cpp",
        )
        self.assertEqual([trace.uid for trace in traces], ["RPI.001", "CTRL.0"])

    def test_multiline_nested_arguments_and_invocation_line(self) -> None:
        source = """int value = 0;
XWALK_HAL_TRACE_UID2(
    RPI.1002,
    "Nested values: %d %d",
    outer(inner(value, 2)),
    another(3));
"""
        trace = TRACE_GENERATOR.scan_source(source, "src/nested.cpp")[0]
        self.assertEqual(trace.source_line, 2)
        self.assertEqual(trace.trace_format, "Nested values: %d %d")

    def test_comments_strings_and_definitions_are_ignored(self) -> None:
        source = r'''
// XWALK_HAL_TRACE_UID0(RPI.3001, "comment")
/* XWALK_CTRL_TRACE_UID0(CTRL.3002, "comment") */
const char* text = "XWALK_HAL_TRACE_UID0(RPI.3003, ignored)";
#define XWALK_HAL_TRACE_UID0(UID, ...) replacement
XWALK_HAL_TRACE_UID0(RPI.3004, "real");
'''
        traces = TRACE_GENERATOR.scan_source(source, "src/comments.cpp")
        self.assertEqual([trace.uid for trace in traces], ["RPI.3004"])

    def test_adjacent_format_literals(self) -> None:
        source = 'XWALK_CTRL_TRACE_UID1(CTRL.4001, "first " "second");'
        trace = TRACE_GENERATOR.scan_source(source, "src/format.cpp")[0]
        self.assertEqual(trace.trace_format, "first second")

    def test_unsupported_priority(self) -> None:
        with self.assertRaisesRegex(TRACE_GENERATOR.ScannerError, "Unsupported trace priority"):
            TRACE_GENERATOR.scan_source(
                'XWALK_HAL_TRACE_UID4(RPI.5001, "invalid");', "priority.cpp"
            )

    def test_duplicate_uid_in_one_file_and_across_priorities(self) -> None:
        traces = TRACE_GENERATOR.scan_source(
            'XWALK_HAL_TRACE_UID0(RPI.6001, "first");\n'
            'XWALK_HAL_TRACE_UID3(RPI.6001, "second");',
            "duplicate.cpp",
        )
        with self.assertRaisesRegex(TRACE_GENERATOR.ScannerError, "Duplicate trace identifier"):
            TRACE_GENERATOR.validate_uniqueness(traces)

    def test_duplicate_uid_across_files(self) -> None:
        traces = TRACE_GENERATOR.scan_source(
            'XWALK_CTRL_TRACE_UID0(CTRL.7001, "first");', "first.cpp"
        )
        traces += TRACE_GENERATOR.scan_source(
            'XWALK_CTRL_TRACE_UID1(CTRL.7001, "second");', "second.cpp"
        )
        with self.assertRaisesRegex(TRACE_GENERATOR.ScannerError, "second.cpp"):
            TRACE_GENERATOR.validate_uniqueness(traces)

    def test_xml_generation_escaping_defaults_and_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            source = source_root / "sample.cpp"
            source.write_text(
                'XWALK_HAL_TRACE_UID0(RPI.8001, "value < %d & \\\"quoted\\\"");\n',
                encoding="utf-8",
            )
            output = root / "generated" / "xWalkTrace.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            xml_root = ElementTree.parse(output).getroot()
            priority_values = [
                priority.get("enabled")
                for priority in xml_root.findall("./priorities/priority")
            ]
            trace = xml_root.find("./traces/trace")
            assert trace is not None
            self.assertEqual(priority_values, ["false", "false", "false", "false"])
            self.assertEqual(trace.get("file"), "src/sample.cpp")
            self.assertEqual(trace.get("line"), "1")
            self.assertEqual(trace.get("format"), 'value < %d & "quoted"')
            self.assertEqual(trace.get("enabled"), "false")
            self.assertNotIn("timestamp", trace.attrib)
            self.assertNotIn("elapsed", trace.attrib)

    def test_existing_flags_are_preserved_and_removed_uids_disappear(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            source = source_root / "sample.cpp"
            source.write_text(
                'XWALK_HAL_TRACE_UID0(RPI.8101, "first");\n'
                'XWALK_HAL_TRACE_UID1(RPI.8102, "remove");\n',
                encoding="utf-8",
            )
            output = root / "xWalkTrace.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            contents = output.read_text(encoding="utf-8")
            contents = contents.replace(
                '<priority level="2" enabled="false"/>',
                '<priority level="2" enabled="true"/>',
            ).replace('uid="RPI.8101"\n            priority', 'uid="RPI.8101"\n            priority')
            contents = contents.replace(
                'uid="RPI.8101"\n            priority="0"\n            enabled="false"',
                'uid="RPI.8101"\n            priority="0"\n            enabled="true"',
            )
            output.write_text(contents, encoding="utf-8")
            source.write_text(
                '\nXWALK_HAL_TRACE_UID0(RPI.8101, "moved");\n'
                'XWALK_HAL_TRACE_UID3(RPI.8103, "new");\n',
                encoding="utf-8",
            )
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            xml_root = ElementTree.parse(output).getroot()
            priorities = {
                node.get("level"): node.get("enabled")
                for node in xml_root.findall("./priorities/priority")
            }
            traces = {node.get("uid"): node for node in xml_root.findall("./traces/trace")}
            self.assertEqual(priorities["2"], "true")
            self.assertEqual(traces["RPI.8101"].get("enabled"), "true")
            self.assertEqual(traces["RPI.8101"].get("line"), "2")
            self.assertEqual(traces["RPI.8101"].get("format"), "moved")
            self.assertEqual(traces["RPI.8103"].get("enabled"), "false")
            self.assertNotIn("RPI.8102", traces)

    def test_unchanged_xml_is_not_rewritten(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            (source_root / "sample.cpp").write_text(
                'XWALK_CTRL_TRACE_UID0(CTRL.8201, "stable");\n', encoding="utf-8"
            )
            output = root / "xWalkTrace.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            original_timestamp = output.stat().st_mtime_ns
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            self.assertEqual(output.stat().st_mtime_ns, original_timestamp)

    def test_malformed_existing_xml_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "xWalkTrace.xml"
            output.write_text("<xwalkTrace>", encoding="utf-8")
            with self.assertRaisesRegex(TRACE_GENERATOR.ScannerError, "malformed"):
                TRACE_GENERATOR.generate_xml([], output)

    def test_assertion_signal_type_is_checked_at_compile_time(self) -> None:
        compiler = shutil.which(os.environ.get("CXX", "c++"))
        self.assertIsNotNone(compiler)
        project_root = SCRIPT_PATH.parents[2]
        include_arguments = [
            "-I",
            str(project_root / "xWalkTrace" / "include"),
            "-I",
            str(project_root / "xWalkLibrary" / "common" / "include"),
        ]
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "assertion.cpp"
            source.write_text(
                '#include "xHal_Rpi5CarTrace.h"\n'
                "void valid() { XWALK_HAL_ASSERT(100); }\n"
                'void invalid() { XWALK_HAL_ASSERT("100"); }\n',
                encoding="utf-8",
            )
            result = subprocess.run(
                [compiler, "-std=c++17", "-fsyntax-only", *include_arguments, str(source)],
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("xWalk assertion signal must be numeric", result.stderr)


if __name__ == "__main__":
    unittest.main()
