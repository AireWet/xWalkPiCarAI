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
    """Exercises tokenization, uniqueness, and deterministic XML generation."""

    def testValidModuleIdentifiers(self) -> None:
        source = """
XWALK_HAL_TRACE_UID0(RPI.1001, "HAL ready");
XWALK_CTRL_TRACE_UID1(CTRL.2001, "CTRL value: %u", value);
XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.3001, "Agent ready");
XWALK_LIB_TRACE_UID0(LIB.4001, "Library ready");
"""
        priorities = {"RPI.1001": 0, "CTRL.2001": 3, "RPIAGENT.3001": 1, "LIB.4001": 2}
        traces = TRACE_GENERATOR.scanSource(source, "src/sample.cpp", priorities)
        self.assertEqual(
            [trace.uid for trace in traces],
            ["RPI.1001", "CTRL.2001", "RPIAGENT.3001", "LIB.4001"],
        )
        self.assertEqual([trace.priority for trace in traces], [0, 3, 1, 2])
        self.assertEqual([trace.format_argument_count for trace in traces], [0, 1, 0, 0])

    def testSameNumericIdInDistinctTagsIsValid(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.001, "HAL");\n'
            'XWALK_CTRL_TRACE_UID0(CTRL.001, "CTRL");\n'
            'XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.001, "Agent");\n'
            'XWALK_LIB_TRACE_UID0(LIB.001, "Library");',
            "distinct-modules.cpp",
        )
        TRACE_GENERATOR.validateUniqueness(traces)
        self.assertEqual(
            [trace.uid for trace in traces],
            ["RPI.001", "CTRL.001", "RPIAGENT.001", "LIB.001"],
        )

    def testEquivalentNumericValuesInOneTagAreRejected(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.001, "first");\n'
            'XWALK_HAL_TRACE_UID0(RPI.1, "second");',
            "equivalent-values.cpp",
        )
        with self.assertRaisesRegex(
            TRACE_GENERATOR.ScannerError,
            "Duplicate numeric trace value in RPI: 1",
        ):
            TRACE_GENERATOR.validateUniqueness(traces)

    def testInvalidIdentifierForms(self) -> None:
        invalid_uids = [
            "RPI.Camera",
            "CTRL.20A1",
            "RPIAGENT.Agent",
            "LIB.",
            "OTHER.1001",
            "RPI.",
            "CTRL.",
        ]
        for uid in invalid_uids:
            with self.subTest(uid=uid), self.assertRaises(TRACE_GENERATOR.ScannerError):
                TRACE_GENERATOR.scanSource(
                    f'XWALK_HAL_TRACE_UID0({uid}, "invalid");', "invalid.cpp"
                )

    def testComponentTagMismatch(self) -> None:
        cases = [
            'XWALK_HAL_TRACE_UID0(CTRL.1001, "invalid");',
            'XWALK_CTRL_TRACE_UID0(RPI.2001, "invalid");',
            'XWALK_RPIAGENT_TRACE_UID0(LIB.3001, "invalid");',
            'XWALK_LIB_TRACE_UID0(RPIAGENT.4001, "invalid");',
        ]
        for source in cases:
            with self.subTest(source=source), self.assertRaisesRegex(
                TRACE_GENERATOR.ScannerError, "trace macros require"
            ):
                TRACE_GENERATOR.scanSource(source, "mismatch.cpp")

    def testSourceTreeRequiresItsOwnedMacroFamily(self) -> None:
        cases = [
            ("xWalkHal/src/hal.cpp", 'XWALK_CTRL_TRACE_UID0(CTRL.1, "bad");'),
            (
                "xWalkController/src/controller.cpp",
                'XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.1, "bad");',
            ),
            ("xWalkAgent/src/agent.cpp", 'XWALK_HAL_TRACE_UID0(RPI.1, "bad");'),
            ("xWalkLibrary/src/library.cpp", 'XWALK_CTRL_TRACE_UID0(CTRL.1, "bad");'),
        ]
        for source_file, source in cases:
            with self.subTest(source_file=source_file), self.assertRaisesRegex(
                TRACE_GENERATOR.ScannerError, "Invalid trace macro ownership"
            ):
                TRACE_GENERATOR.scanSource(source, source_file)

        valid_agent = TRACE_GENERATOR.scanSource(
            'XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.1, "valid");',
            "xWalkAgent/src/agent.cpp",
        )
        valid_library = TRACE_GENERATOR.scanSource(
            'XWALK_LIB_TRACE_UID0(LIB.1, "valid");',
            "xWalkLibrary/src/library.cpp",
        )
        self.assertEqual(valid_agent[0].uid, "RPIAGENT.1")
        self.assertEqual(valid_library[0].uid, "LIB.1")

    def testLeadingZeroPolicy(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.001, "valid");\n'
            'XWALK_CTRL_TRACE_UID0(CTRL.0, "valid");',
            "leading-zero.cpp",
        )
        self.assertEqual([trace.uid for trace in traces], ["RPI.001", "CTRL.0"])

    def testMultilineNestedArgumentsAndInvocationLine(self) -> None:
        source = """int value = 0;
XWALK_HAL_TRACE_UID2(
    RPI.1002,
    "Nested values: %d %d",
    outer(inner(value, 2)),
    another(3));
"""
        trace = TRACE_GENERATOR.scanSource(source, "src/nested.cpp")[0]
        self.assertEqual(trace.source_line, 2)
        self.assertEqual(trace.trace_format, "Nested values: %d %d")

    def testCommentsStringsAndDefinitionsAreIgnored(self) -> None:
        source = r'''
// XWALK_HAL_TRACE_UID0(RPI.3001, "comment")
/* XWALK_CTRL_TRACE_UID0(CTRL.3002, "comment") */
const char* text = "XWALK_HAL_TRACE_UID0(RPI.3003, ignored)";
#define XWALK_HAL_TRACE_UID0(UID, ...) replacement
XWALK_HAL_TRACE_UID0(RPI.3004, "real");
'''
        traces = TRACE_GENERATOR.scanSource(source, "src/comments.cpp")
        self.assertEqual([trace.uid for trace in traces], ["RPI.3004"])

    def testAdjacentFormatLiterals(self) -> None:
        source = 'XWALK_CTRL_TRACE_UID0(CTRL.4001, "first " "second");'
        trace = TRACE_GENERATOR.scanSource(source, "src/format.cpp")[0]
        self.assertEqual(trace.trace_format, "first second")

    def testUnsupportedArgumentCount(self) -> None:
        with self.assertRaisesRegex(TRACE_GENERATOR.ScannerError, "Unsupported trace argument count"):
            TRACE_GENERATOR.scanSource(
                'XWALK_HAL_TRACE_UID6(RPI.5001, "%d %d %d %d %d %d", 1, 2, 3, 4, 5, 6);',
                "argument-count.cpp",
            )

    def testDuplicateUidInOneFileAndAcrossArgumentCounts(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.6001, "first");\n'
            'XWALK_HAL_TRACE_UID1(RPI.6001, "second %d", 2);',
            "duplicate.cpp",
        )
        with self.assertRaisesRegex(
            TRACE_GENERATOR.ScannerError,
            "Duplicate trace ID: RPI.6001",
        ):
            TRACE_GENERATOR.validateUniqueness(traces)

    def testDuplicateUidAcrossFiles(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_CTRL_TRACE_UID0(CTRL.7001, "first");', "first.cpp"
        )
        traces += TRACE_GENERATOR.scanSource(
            'XWALK_CTRL_TRACE_UID0(CTRL.7001, "second");', "second.cpp"
        )
        with self.assertRaisesRegex(
            TRACE_GENERATOR.ScannerError,
            "non-unique trace IDs are used",
        ):
            TRACE_GENERATOR.validateUniqueness(traces)

    def testDuplicateUidCliFailsMetadataGeneration(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            (source_root / "first.cpp").write_text(
                'XWALK_HAL_TRACE_UID0(RPI.7002, "first");\n', encoding="utf-8"
            )
            (source_root / "second.cpp").write_text(
                'XWALK_HAL_TRACE_UID0(RPI.7002, "second");\n', encoding="utf-8"
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT_PATH),
                    "--project-root",
                    str(root),
                    "--source-root",
                    str(source_root),
                    "--output",
                    str(root / "generated" / "xwalk-traces.xml"),
                ],
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(
                "Duplicate trace ID: RPI.7002", result.stderr
            )
            self.assertIn("src/first.cpp:1", result.stderr)
            self.assertIn("src/second.cpp:1", result.stderr)
            self.assertFalse((root / "generated" / "xwalk-traces.xml").exists())

    def testXmlGenerationEscapingDefaultsAndMetadata(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            source = source_root / "sample.cpp"
            source.write_text(
                'XWALK_HAL_TRACE_UID1(RPI.8001, "value < %d & \\\"quoted\\\"", value);\n',
                encoding="utf-8",
            )
            output = root / "generated" / "xwalk-traces.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            xml_root = ElementTree.parse(output).getroot()
            trace = xml_root.find("./module/trace")
            assert trace is not None
            self.assertEqual(xml_root.tag, "xwalkTraceCatalogue")
            self.assertEqual(trace.get("sourceFile"), "src/sample.cpp")
            self.assertEqual(trace.get("sourceLine"), "1")
            self.assertEqual(trace.get("format"), 'value < %d & "quoted"')
            self.assertEqual(trace.get("formatArgumentCount"), "1")
            self.assertEqual(trace.get("priority"), "3")
            self.assertEqual(trace.get("defaultState"), "disable")
            self.assertNotIn("timestamp", trace.attrib)
            self.assertNotIn("elapsed", trace.attrib)

    def testCatalogueRegenerationRemovesAbsentUids(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            source = source_root / "sample.cpp"
            source.write_text(
                'XWALK_HAL_TRACE_UID0(RPI.8101, "first");\n'
                'XWALK_HAL_TRACE_UID0(RPI.8102, "remove");\n',
                encoding="utf-8",
            )
            output = root / "xwalk-traces.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            document = ElementTree.parse(output)
            persisted_root = document.getroot()
            persisted_module = persisted_root.find("./module")
            assert persisted_module is not None
            persisted_root.set("defaultState", "enable")
            persisted_module.set("defaultState", "enable")
            persisted_trace = persisted_module.find("trace")
            assert persisted_trace is not None
            persisted_trace.set("defaultState", "enable")
            document.write(output, encoding="UTF-8", xml_declaration=True)
            source.write_text(
                '\nXWALK_HAL_TRACE_UID0(RPI.8101, "moved");\n'
                'XWALK_HAL_TRACE_UID0(RPI.8103, "new");\n',
                encoding="utf-8",
            )
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            xml_root = ElementTree.parse(output).getroot()
            self.assertEqual(xml_root.get("defaultState"), "enable")
            self.assertEqual(xml_root.find("./module").get("defaultState"), "enable")
            traces = {
                node.get("fullId"): node
                for node in xml_root.findall("./module/trace")
            }
            self.assertEqual(traces["RPI.8101"].get("defaultState"), "enable")
            self.assertEqual(traces["RPI.8101"].get("sourceLine"), "2")
            self.assertEqual(traces["RPI.8101"].get("format"), "moved")
            self.assertEqual(traces["RPI.8103"].get("defaultState"), "disable")
            self.assertNotIn("RPI.8102", traces)

    def testUnchangedXmlIsNotRewritten(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_root = root / "src"
            source_root.mkdir()
            (source_root / "sample.cpp").write_text(
                'XWALK_CTRL_TRACE_UID0(CTRL.8201, "stable");\n', encoding="utf-8"
            )
            output = root / "xwalk-traces.xml"
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            original_timestamp = output.stat().st_mtime_ns
            TRACE_GENERATOR.XWalkTracePreCompiler(root, [source_root], output).run()
            self.assertEqual(output.stat().st_mtime_ns, original_timestamp)

    def testXmlModulesAndNumericIdsAreSorted(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.010, "ten");\n'
            'XWALK_CTRL_TRACE_UID0(CTRL.002, "two");\n'
            'XWALK_HAL_TRACE_UID0(RPI.003, "three");\n'
            'XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.004, "agent");\n'
            'XWALK_LIB_TRACE_UID0(LIB.005, "library");',
            "sorting.cpp",
        )
        xml_text = TRACE_GENERATOR.generateXml(traces, Path("unused.xml"))
        root = ElementTree.fromstring(xml_text)
        modules = root.findall("./module")
        self.assertEqual(
            [module.get("name") for module in modules],
            ["CTRL", "LIB", "RPI", "RPIAGENT"],
        )
        self.assertEqual(
            [trace.get("id") for trace in modules[2].findall("./trace")],
            ["003", "010"],
        )

    def testMultipleDuplicateIdsAreReportedTogether(self) -> None:
        traces = TRACE_GENERATOR.scanSource(
            'XWALK_HAL_TRACE_UID0(RPI.9001, "one");\n'
            'XWALK_CTRL_TRACE_UID0(CTRL.9001, "one");\n'
            'XWALK_HAL_TRACE_UID1(RPI.9001, "two %d", 2);\n'
            'XWALK_CTRL_TRACE_UID1(CTRL.9001, "two %d", 2);',
            "duplicates.cpp",
        )
        with self.assertRaises(TRACE_GENERATOR.ScannerError) as context:
            TRACE_GENERATOR.validateUniqueness(traces)
        message = str(context.exception)
        self.assertIn("Duplicate trace ID: RPI.9001", message)
        self.assertIn("Duplicate trace ID: CTRL.9001", message)
        self.assertIn("RPI.9001 declared at: duplicates.cpp:", message)
        self.assertIn("CTRL.9001 declared at: duplicates.cpp:", message)
        self.assertEqual(message.count("declared at: duplicates.cpp:"), 4)

    def testAssertionSignalTypeIsCheckedAtCompileTime(self) -> None:
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
