#!/usr/bin/env python3
"""Validate xWalk tagged-trace calls and generate deterministic XML metadata."""

from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ElementTree
from xml.sax.saxutils import quoteattr


MACRO_PATTERN = re.compile(r"XWALK_(HAL|CTRL)_TRACE_UID([0-9]+)$")
UID_PATTERN = re.compile(r"^(RPI|CTRL)\.([0-9]+)$")
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"}
EXCLUDED_DIRECTORY_NAMES = {
    ".git",
    "auto-gen",
    "external",
    "third_party",
    "third-party",
}


class ScannerError(RuntimeError):
    """Reports one actionable source or XML validation failure."""


@dataclass(frozen=True)
class TraceOccurrence:
    """Metadata extracted from one tagged-trace macro invocation."""

    component: str
    tag: str
    numeric_id: str
    uid: str
    priority: int
    trace_format: str
    source_file: str
    source_line: int
    macro_name: str


def _skip_quoted(text: str, position: int) -> int:
    """Return the first position after one C++ quoted literal."""

    quote = text[position]
    position += 1
    while position < len(text):
        if text[position] == "\\":
            position += 2
            continue
        if text[position] == quote:
            return position + 1
        position += 1
    raise ScannerError("Unterminated string or character literal")


def _is_digit_separator(text: str, position: int) -> bool:
    """Report whether an apostrophe separates digits in a C++ number."""

    return (
        text[position] == "'"
        and position > 0
        and position + 1 < len(text)
        and text[position - 1].isalnum()
        and text[position + 1].isalnum()
    )


def _skip_raw_string(text: str, position: int) -> int | None:
    """Skip a C++ raw string beginning at `R\"`, when present."""

    if not text.startswith('R"', position):
        return None
    delimiter_end = text.find("(", position + 2)
    if delimiter_end < 0:
        raise ScannerError("Malformed raw string literal")
    delimiter = text[position + 2 : delimiter_end]
    terminator = ")" + delimiter + '"'
    literal_end = text.find(terminator, delimiter_end + 1)
    if literal_end < 0:
        raise ScannerError("Unterminated raw string literal")
    return literal_end + len(terminator)


def _skip_space_and_comments(text: str, position: int) -> int:
    """Skip whitespace and comments without consuming source tokens."""

    while position < len(text):
        if text[position].isspace():
            position += 1
        elif text.startswith("//", position):
            newline = text.find("\n", position + 2)
            position = len(text) if newline < 0 else newline + 1
        elif text.startswith("/*", position):
            comment_end = text.find("*/", position + 2)
            if comment_end < 0:
                raise ScannerError("Unterminated block comment")
            position = comment_end + 2
        else:
            break
    return position


def _parse_arguments(text: str, open_parenthesis: int) -> tuple[list[str], int]:
    """Parse balanced macro arguments, preserving nested expressions."""

    arguments: list[str] = []
    argument_start = open_parenthesis + 1
    position = argument_start
    parentheses = 1
    brackets = 0
    braces = 0
    while position < len(text):
        raw_end = _skip_raw_string(text, position)
        if raw_end is not None:
            position = raw_end
            continue
        character = text[position]
        if character == '"' or (character == "'" and not _is_digit_separator(text, position)):
            position = _skip_quoted(text, position)
            continue
        if text.startswith("//", position):
            newline = text.find("\n", position + 2)
            position = len(text) if newline < 0 else newline + 1
            continue
        if text.startswith("/*", position):
            comment_end = text.find("*/", position + 2)
            if comment_end < 0:
                raise ScannerError("Unterminated block comment in macro invocation")
            position = comment_end + 2
            continue
        if character == "(":
            parentheses += 1
        elif character == ")":
            parentheses -= 1
            if parentheses == 0:
                arguments.append(text[argument_start:position].strip())
                return arguments, position + 1
        elif character == "[":
            brackets += 1
        elif character == "]":
            brackets -= 1
        elif character == "{":
            braces += 1
        elif character == "}":
            braces -= 1
        elif character == "," and parentheses == 1 and brackets == 0 and braces == 0:
            arguments.append(text[argument_start:position].strip())
            argument_start = position + 1
        position += 1
    raise ScannerError("Unterminated tagged-trace macro invocation")


def _strip_comments(text: str) -> str:
    """Remove comments from one compact macro argument."""

    without_blocks = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", without_blocks).strip()


def _decode_format(expression: str, location: str) -> str:
    """Decode one or more adjacent ordinary C++ string literals."""

    position = 0
    values: list[str] = []
    while True:
        position = _skip_space_and_comments(expression, position)
        if position >= len(expression):
            break
        prefix_match = re.match(r"(?:u8|u|U|L)?", expression[position:])
        assert prefix_match is not None
        position += len(prefix_match.group(0))
        if position >= len(expression) or expression[position] != '"':
            raise ScannerError(f"Trace format must be a string literal at {location}")
        literal_end = _skip_quoted(expression, position)
        literal = expression[position:literal_end]
        try:
            decoded = ast.literal_eval(literal)
        except (SyntaxError, ValueError) as error:
            raise ScannerError(f"Invalid trace format string at {location}: {error}") from error
        values.append(decoded)
        position = literal_end
    if not values:
        raise ScannerError(f"Missing trace format string at {location}")
    return "".join(values)


def _line_is_macro_definition(text: str, token_start: int) -> bool:
    """Report whether a token occurs on a preprocessor definition line."""

    line_start = text.rfind("\n", 0, token_start) + 1
    return text[line_start:token_start].lstrip().startswith("#")


def scan_source(text: str, source_file: str) -> list[TraceOccurrence]:
    """Extract and validate tagged trace invocations from one source string."""

    occurrences: list[TraceOccurrence] = []
    position = 0
    while position < len(text):
        raw_end = _skip_raw_string(text, position)
        if raw_end is not None:
            position = raw_end
            continue
        character = text[position]
        if character == '"' or (character == "'" and not _is_digit_separator(text, position)):
            position = _skip_quoted(text, position)
            continue
        if text.startswith("//", position):
            newline = text.find("\n", position + 2)
            position = len(text) if newline < 0 else newline + 1
            continue
        if text.startswith("/*", position):
            comment_end = text.find("*/", position + 2)
            if comment_end < 0:
                raise ScannerError(f"Unterminated block comment in {source_file}")
            position = comment_end + 2
            continue
        if character.isalpha() or character == "_":
            token_start = position
            position += 1
            while position < len(text) and (text[position].isalnum() or text[position] == "_"):
                position += 1
            token = text[token_start:position]
            macro_match = MACRO_PATTERN.fullmatch(token)
            if macro_match is None:
                continue
            if _line_is_macro_definition(text, token_start):
                continue
            component = macro_match.group(1)
            priority = int(macro_match.group(2))
            source_line = text.count("\n", 0, token_start) + 1
            location = f"{source_file}:{source_line}"
            if priority not in range(4):
                raise ScannerError(
                    f"Unsupported trace priority in {token}\nFile: {source_file}\n"
                    f"Line: {source_line}"
                )
            open_parenthesis = _skip_space_and_comments(text, position)
            if open_parenthesis >= len(text) or text[open_parenthesis] != "(":
                raise ScannerError(f"Missing invocation parentheses for {token} at {location}")
            arguments, position = _parse_arguments(text, open_parenthesis)
            if len(arguments) < 2:
                raise ScannerError(f"{token} requires a UID and format string at {location}")
            uid = re.sub(r"\s+", "", _strip_comments(arguments[0]))
            uid_match = UID_PATTERN.fullmatch(uid)
            if uid_match is None:
                raise ScannerError(
                    f"Invalid trace identifier: {uid or '<empty>'}\n\n"
                    f"File: {source_file}\nLine: {source_line}\nMacro: {token}\n\n"
                    "Identifiers must match RPI.<number> or CTRL.<number>."
                )
            tag, numeric_id = uid_match.groups()
            required_tag = "RPI" if component == "HAL" else "CTRL"
            if tag != required_tag:
                raise ScannerError(
                    f"Invalid trace identifier: {uid}\n\nFile: {source_file}\n"
                    f"Line: {source_line}\nMacro: {token}\n\n"
                    f"{component} trace macros require an {required_tag}.<number> identifier."
                )
            trace_format = _decode_format(arguments[1], location)
            occurrences.append(
                TraceOccurrence(
                    component=component,
                    tag=tag,
                    numeric_id=numeric_id,
                    uid=uid,
                    priority=priority,
                    trace_format=trace_format,
                    source_file=source_file,
                    source_line=source_line,
                    macro_name=token,
                )
            )
            continue
        position += 1
    return occurrences


def _excluded(path: Path) -> bool:
    """Report whether one path belongs to generated, build, or external data."""

    for part in path.parts:
        if part in EXCLUDED_DIRECTORY_NAMES:
            return True
        if part == "build" or part.startswith("build-") or part.startswith("cmake-build-"):
            return True
    return False


def collect_sources(source_roots: list[Path], project_root: Path) -> list[Path]:
    """Collect deterministic project source paths from explicit roots."""

    sources: set[Path] = set()
    for source_root in source_roots:
        if not source_root.is_dir():
            raise ScannerError(f"Trace source root does not exist: {source_root}")
        for path in source_root.rglob("*"):
            if path.is_file() and path.suffix in SOURCE_SUFFIXES and not _excluded(path):
                sources.add(path.resolve())
    return sorted(sources, key=lambda path: path.relative_to(project_root).as_posix())


def validate_uniqueness(occurrences: list[TraceOccurrence]) -> None:
    """Reject a complete UID appearing more than once in the project."""

    first_by_uid: dict[str, TraceOccurrence] = {}
    for occurrence in occurrences:
        first = first_by_uid.get(occurrence.uid)
        if first is not None:
            raise ScannerError(
                f"Duplicate trace identifier: {occurrence.uid}\n\n"
                "First occurrence:\n"
                f"  Macro: {first.macro_name}\n  Priority: {first.priority}\n"
                f"  File: {first.source_file}\n  Line: {first.source_line}\n\n"
                "Duplicate occurrence:\n"
                f"  Macro: {occurrence.macro_name}\n  Priority: {occurrence.priority}\n"
                f"  File: {occurrence.source_file}\n  Line: {occurrence.source_line}"
            )
        first_by_uid[occurrence.uid] = occurrence


def _existing_flags(output_path: Path) -> tuple[dict[int, bool], dict[str, bool]]:
    """Read user-controlled enable flags from an existing generated XML file."""

    priorities = {0: False, 1: False, 2: False, 3: False}
    traces: dict[str, bool] = {}
    if not output_path.exists():
        return priorities, traces
    try:
        root = ElementTree.parse(output_path).getroot()
    except ElementTree.ParseError as error:
        raise ScannerError(f"Existing trace XML is malformed: {output_path}: {error}") from error
    if root.tag != "xwalkTrace":
        raise ScannerError(f"Existing trace XML has an invalid root: {output_path}")
    for priority in root.findall("./priorities/priority"):
        level_text = priority.get("level", "")
        enabled_text = priority.get("enabled", "")
        priority_valid = (
            level_text.isdigit()
            and int(level_text) in range(4)
            and enabled_text in {"true", "false"}
        )
        if not priority_valid:
            raise ScannerError(f"Existing trace XML has an invalid priority: {output_path}")
        priorities[int(level_text)] = enabled_text == "true"
    for trace in root.findall("./traces/trace"):
        uid = trace.get("uid")
        enabled_text = trace.get("enabled", "")
        if not uid or UID_PATTERN.fullmatch(uid) is None or enabled_text not in {"true", "false"}:
            raise ScannerError(f"Existing trace XML has an invalid trace entry: {output_path}")
        traces[uid] = enabled_text == "true"
    return priorities, traces


def generate_xml(occurrences: list[TraceOccurrence], output_path: Path) -> str:
    """Render deterministic metadata while preserving existing enable flags."""

    priority_flags, trace_flags = _existing_flags(output_path)
    ordered = sorted(
        occurrences,
        key=lambda trace: (trace.component, int(trace.numeric_id), trace.uid),
    )
    lines = ['<?xml version="1.0" encoding="UTF-8"?>', "<xwalkTrace>", "    <priorities>"]
    for level in range(4):
        enabled = "true" if priority_flags[level] else "false"
        lines.append(f'        <priority level="{level}" enabled="{enabled}"/>')
    lines.extend(["    </priorities>", "", "    <traces>"])
    for trace in ordered:
        enabled = "true" if trace_flags.get(trace.uid, False) else "false"
        attributes = [
            ("component", trace.component),
            ("tag", trace.tag),
            ("id", trace.numeric_id),
            ("uid", trace.uid),
            ("priority", str(trace.priority)),
            ("enabled", enabled),
            ("file", trace.source_file),
            ("line", str(trace.source_line)),
            ("format", trace.trace_format),
            ("macro", trace.macro_name),
        ]
        lines.append("        <trace")
        for name, value in attributes:
            lines.append(f"            {name}={quoteattr(value)}")
        lines[-1] += "/>"
    lines.extend(["    </traces>", "</xwalkTrace>", ""])
    return "\n".join(lines)


def write_if_changed(output_path: Path, contents: str) -> bool:
    """Atomically replace XML only when effective contents changed."""

    if output_path.exists() and output_path.read_text(encoding="utf-8") == contents:
        return False
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = output_path.with_suffix(output_path.suffix + ".tmp")
    temporary_path.write_text(contents, encoding="utf-8")
    temporary_path.replace(output_path)
    return True


class XWalkTracePreCompiler:
    """Owns one deterministic source-scan and XML-generation operation."""

    def __init__(
        self, project_root: Path, source_roots: list[Path], output_path: Path
    ) -> None:
        """Retain resolved build inputs without scanning during construction."""

        self.project_root = project_root.resolve()
        self.source_roots = [path.resolve() for path in source_roots]
        self.output_path = output_path.resolve()

    def run(self) -> list[TraceOccurrence]:
        """Scan, validate, and generate metadata for the configured source roots."""

        occurrences: list[TraceOccurrence] = []
        for source_path in collect_sources(self.source_roots, self.project_root):
            relative_path = source_path.relative_to(self.project_root).as_posix()
            source_text = source_path.read_text(encoding="utf-8")
            try:
                occurrences.extend(scan_source(source_text, relative_path))
            except ScannerError as error:
                raise ScannerError(f"{relative_path}: {error}") from error
        occurrences.sort(
            key=lambda trace: (trace.source_file, trace.source_line, trace.macro_name)
        )
        validate_uniqueness(occurrences)
        write_if_changed(
            self.output_path, generate_xml(occurrences, self.output_path)
        )
        return occurrences


def main() -> int:
    """Command-line entry point."""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, action="append", required=True)
    parser.add_argument("--output", type=Path, required=True)
    arguments = parser.parse_args()
    try:
        pre_compiler = XWalkTracePreCompiler(
            arguments.project_root,
            arguments.source_root,
            arguments.output,
        )
        occurrences = pre_compiler.run()
    except (OSError, ScannerError, UnicodeError) as error:
        print(f"xWalk trace metadata generation failed:\n{error}", file=sys.stderr)
        return 1
    print(f"Validated {len(occurrences)} tagged trace identifier(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
