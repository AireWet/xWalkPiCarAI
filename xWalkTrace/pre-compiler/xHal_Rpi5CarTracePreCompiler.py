#!/usr/bin/env python3
"""Validate xWalk tagged-trace calls and generate deterministic XML metadata."""

from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import re
import sys
from xml.sax.saxutils import quoteattr


MACRO_PATTERN = re.compile(r"XWALK_(HAL|CTRL)_TRACE_UID([0-9]+)$")
UID_PATTERN = re.compile(r"^(RPI|CTRL)\.([0-9]+)$")
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"}
EXCLUDED_DIRECTORY_NAMES = {
    ".git",
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


def _skipQuoted(text: str, position: int) -> int:
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


def _isDigitSeparator(text: str, position: int) -> bool:
    """Report whether an apostrophe separates digits in a C++ number."""

    return (
        text[position] == "'"
        and position > 0
        and position + 1 < len(text)
        and text[position - 1].isalnum()
        and text[position + 1].isalnum()
    )


def _skipRawString(text: str, position: int) -> int | None:
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


def _skipSpaceAndComments(text: str, position: int) -> int:
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


def _parseArguments(text: str, open_parenthesis: int) -> tuple[list[str], int]:
    """Parse balanced macro arguments, preserving nested expressions."""

    arguments: list[str] = []
    argument_start = open_parenthesis + 1
    position = argument_start
    parentheses = 1
    brackets = 0
    braces = 0
    while position < len(text):
        raw_end = _skipRawString(text, position)
        if raw_end is not None:
            position = raw_end
            continue
        character = text[position]
        if character == '"' or (character == "'" and not _isDigitSeparator(text, position)):
            position = _skipQuoted(text, position)
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


def _stripComments(text: str) -> str:
    """Remove comments from one compact macro argument."""

    without_blocks = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", without_blocks).strip()


def _decodeFormat(expression: str, location: str) -> str:
    """Decode one or more adjacent ordinary C++ string literals."""

    position = 0
    values: list[str] = []
    while True:
        position = _skipSpaceAndComments(expression, position)
        if position >= len(expression):
            break
        prefix_match = re.match(r"(?:u8|u|U|L)?", expression[position:])
        assert prefix_match is not None
        position += len(prefix_match.group(0))
        if position >= len(expression) or expression[position] != '"':
            raise ScannerError(f"Trace format must be a string literal at {location}")
        literal_end = _skipQuoted(expression, position)
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


def _lineIsMacroDefinition(text: str, token_start: int) -> bool:
    """Report whether a token occurs on a preprocessor definition line."""

    line_start = text.rfind("\n", 0, token_start) + 1
    return text[line_start:token_start].lstrip().startswith("#")


def scanSource(text: str, source_file: str) -> list[TraceOccurrence]:
    """Extract and validate tagged trace invocations from one source string."""

    occurrences: list[TraceOccurrence] = []
    position = 0
    while position < len(text):
        raw_end = _skipRawString(text, position)
        if raw_end is not None:
            position = raw_end
            continue
        character = text[position]
        if character == '"' or (character == "'" and not _isDigitSeparator(text, position)):
            position = _skipQuoted(text, position)
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
            if _lineIsMacroDefinition(text, token_start):
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
            open_parenthesis = _skipSpaceAndComments(text, position)
            if open_parenthesis >= len(text) or text[open_parenthesis] != "(":
                raise ScannerError(f"Missing invocation parentheses for {token} at {location}")
            arguments, position = _parseArguments(text, open_parenthesis)
            if len(arguments) < 2:
                raise ScannerError(f"{token} requires a UID and format string at {location}")
            uid = re.sub(r"\s+", "", _stripComments(arguments[0]))
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
            trace_format = _decodeFormat(arguments[1], location)
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


def _isExcluded(path: Path) -> bool:
    """Report whether one path belongs to generated, build, or external data."""

    for part in path.parts:
        if part in EXCLUDED_DIRECTORY_NAMES:
            return True
        if part == "build" or part.startswith("build-") or part.startswith("cmake-build-"):
            return True
    return False


def collectSources(source_roots: list[Path], project_root: Path) -> list[Path]:
    """Collect deterministic project source paths from explicit roots."""

    sources: set[Path] = set()
    for source_root in source_roots:
        if not source_root.is_dir():
            raise ScannerError(f"Trace source root does not exist: {source_root}")
        for path in source_root.rglob("*"):
            if path.is_file() and path.suffix in SOURCE_SUFFIXES and not _isExcluded(path):
                sources.add(path.resolve())
    return sorted(sources, key=lambda path: path.relative_to(project_root).as_posix())


def validateUniqueness(occurrences: list[TraceOccurrence]) -> None:
    """Reject every complete UID appearing more than once in the project."""

    occurrences_by_uid: dict[str, list[TraceOccurrence]] = {}
    for occurrence in occurrences:
        occurrences_by_uid.setdefault(occurrence.uid, []).append(occurrence)
    duplicates = {
        uid: declarations
        for uid, declarations in occurrences_by_uid.items()
        if len(declarations) > 1
    }
    if not duplicates:
        return

    lines = ["Trace validation error: non-unique trace IDs are used.", ""]
    for uid in sorted(duplicates):
        lines.append(f"Duplicate trace ID: {uid}")
        for declaration in duplicates[uid]:
            lines.append(
                f"  Declared at: {declaration.source_file}:{declaration.source_line} "
                f"({declaration.macro_name})"
            )
        lines.append("")
    lines.append("Compilation stopped because trace IDs must be unique.")
    raise ScannerError("\n".join(lines))


def generateXml(occurrences: list[TraceOccurrence], _output_path: Path) -> str:
    """Render one deterministic immutable trace catalogue."""

    modules: dict[str, list[TraceOccurrence]] = {}
    for occurrence in occurrences:
        modules.setdefault(occurrence.tag, []).append(occurrence)
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        '<xwalkTraceCatalogue version="1.0">',
    ]
    for module_name in sorted(modules):
        lines.append(
            f'  <module name={quoteattr(module_name)} defaultState="disable">'
        )
        ordered = sorted(
            modules[module_name],
            key=lambda trace: (int(trace.numeric_id), trace.numeric_id),
        )
        for trace in ordered:
            attributes = [
                ("id", trace.numeric_id),
                ("fullId", trace.uid),
                ("defaultState", "disable"),
                ("name", trace.trace_format),
                ("sourceFile", trace.source_file),
                ("sourceLine", str(trace.source_line)),
                ("priority", str(trace.priority)),
                ("owningComponent", trace.component),
                ("format", trace.trace_format),
                ("macro", trace.macro_name),
            ]
            lines.append("    <trace")
            for name, value in attributes:
                lines.append(f"      {name}={quoteattr(value)}")
            lines[-1] += " />"
        lines.append("  </module>")
    lines.extend(["</xwalkTraceCatalogue>", ""])
    return "\n".join(lines)


def writeIfChanged(output_path: Path, contents: str) -> bool:
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
        for source_path in collectSources(self.source_roots, self.project_root):
            relative_path = source_path.relative_to(self.project_root).as_posix()
            source_text = source_path.read_text(encoding="utf-8")
            try:
                occurrences.extend(scanSource(source_text, relative_path))
            except ScannerError as error:
                raise ScannerError(f"{relative_path}: {error}") from error
        occurrences.sort(
            key=lambda trace: (trace.source_file, trace.source_line, trace.macro_name)
        )
        validateUniqueness(occurrences)
        writeIfChanged(
            self.output_path, generateXml(occurrences, self.output_path)
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
