#!/usr/bin/env python3
"""Validate xWalk tagged-trace calls and generate deterministic XML metadata."""

from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ElementTree
from xml.sax.saxutils import quoteattr


MACRO_PATTERN = re.compile(r"XWALK_(HAL|CTRL|RPIAGENT|LIB)_TRACE_UID([0-9]+)$")
UID_PATTERN = re.compile(r"^(RPI|CTRL|RPIAGENT|LIB)\.([0-9]+)$")
COMPONENT_TAGS = {
    "HAL": "RPI",
    "CTRL": "CTRL",
    "RPIAGENT": "RPIAGENT",
    "LIB": "LIB",
}
SOURCE_COMPONENTS = {
    "xWalkHal/": "HAL",
    "xWalkController/": "CTRL",
    "xWalkAgent/": "RPIAGENT",
    "xWalkLibrary/": "LIB",
}
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"}
MAXIMUM_FORMAT_ARGUMENTS = 5
DEFAULT_TRACE_PRIORITY = 3
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
    format_argument_count: int
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


def scanSource(
    text: str, source_file: str, priorities: dict[str, int] | None = None
) -> list[TraceOccurrence]:
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
            format_argument_count = int(macro_match.group(2))
            source_line = text.count("\n", 0, token_start) + 1
            location = f"{source_file}:{source_line}"
            if format_argument_count not in range(MAXIMUM_FORMAT_ARGUMENTS + 1):
                raise ScannerError(
                    f"Unsupported trace argument count in {token}\nFile: {source_file}\n"
                    f"Line: {source_line}"
                )
            open_parenthesis = _skipSpaceAndComments(text, position)
            if open_parenthesis >= len(text) or text[open_parenthesis] != "(":
                raise ScannerError(f"Missing invocation parentheses for {token} at {location}")
            arguments, position = _parseArguments(text, open_parenthesis)
            if len(arguments) < 2:
                raise ScannerError(f"{token} requires a UID and format string at {location}")
            actual_format_argument_count = len(arguments) - 2
            if actual_format_argument_count != format_argument_count:
                raise ScannerError(
                    f"{token} declares {format_argument_count} formatting argument(s), "
                    f"but {actual_format_argument_count} were supplied at {location}"
                )
            uid = re.sub(r"\s+", "", _stripComments(arguments[0]))
            uid_match = UID_PATTERN.fullmatch(uid)
            if uid_match is None:
                raise ScannerError(
                    f"Invalid trace identifier: {uid or '<empty>'}\n\n"
                    f"File: {source_file}\nLine: {source_line}\nMacro: {token}\n\n"
                    "Identifiers must match RPI.<number>, CTRL.<number>, "
                    "RPIAGENT.<number>, or LIB.<number>."
                )
            tag, numeric_id = uid_match.groups()
            required_tag = COMPONENT_TAGS[component]
            if tag != required_tag:
                raise ScannerError(
                    f"Invalid trace identifier: {uid}\n\nFile: {source_file}\n"
                    f"Line: {source_line}\nMacro: {token}\n\n"
                    f"{component} trace macros require an {required_tag}.<number> identifier."
                )
            for source_prefix, required_component in SOURCE_COMPONENTS.items():
                if source_file.startswith(source_prefix) and component != required_component:
                    raise ScannerError(
                        f"Invalid trace macro ownership at {location}\n\n"
                        f"Sources below {source_prefix} must use XWALK_{required_component}_TRACE_UIDn."
                    )
            trace_format = _decodeFormat(arguments[1], location)
            priority = DEFAULT_TRACE_PRIORITY if priorities is None else priorities.get(
                uid, DEFAULT_TRACE_PRIORITY
            )
            occurrences.append(
                TraceOccurrence(
                    component=component,
                    tag=tag,
                    numeric_id=numeric_id,
                    uid=uid,
                    priority=priority,
                    format_argument_count=format_argument_count,
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
    """Reject repeated numeric values within one trace tag."""

    occurrences_by_uid: dict[tuple[str, int], list[TraceOccurrence]] = {}
    for occurrence in occurrences:
        scoped_id = (occurrence.tag, int(occurrence.numeric_id))
        occurrences_by_uid.setdefault(scoped_id, []).append(occurrence)
    duplicates = {
        uid: declarations
        for uid, declarations in occurrences_by_uid.items()
        if len(declarations) > 1
    }
    if not duplicates:
        return

    lines = ["Trace validation error: non-unique trace IDs are used.", ""]
    for tag, numeric_value in sorted(duplicates):
        declarations = duplicates[(tag, numeric_value)]
        numeric_texts = {declaration.numeric_id for declaration in declarations}
        if len(numeric_texts) == 1:
            lines.append(f"Duplicate trace ID: {tag}.{declarations[0].numeric_id}")
        else:
            lines.append(f"Duplicate numeric trace value in {tag}: {numeric_value}")
        for declaration in declarations:
            lines.append(
                f"  {declaration.uid} declared at: "
                f"{declaration.source_file}:{declaration.source_line} "
                f"({declaration.macro_name})"
            )
        lines.append("")
    lines.append("Compilation stopped because trace IDs must be unique.")
    raise ScannerError("\n".join(lines))


def loadPriorities(priority_path: Path) -> dict[str, int]:
    """Load validated per-UID priorities from one project-owned JSON map."""

    try:
        document = json.loads(priority_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as error:
        raise ScannerError(f"Trace priority configuration is invalid: {error}") from error
    if not isinstance(document, dict):
        raise ScannerError("Trace priority configuration root must be an object")
    priorities: dict[str, int] = {}
    for uid, priority in document.items():
        if not isinstance(uid, str) or UID_PATTERN.fullmatch(uid) is None:
            raise ScannerError(f"Invalid trace priority UID: {uid}")
        if not isinstance(priority, int) or priority not in range(4):
            raise ScannerError(f"Invalid trace priority for {uid}: {priority}")
        priorities[uid] = priority
    return priorities


def validatePriorityCoverage(
    occurrences: list[TraceOccurrence], priorities: dict[str, int]
) -> None:
    """Require the priority map and source inventory to contain identical UIDs."""

    source_uids = {occurrence.uid for occurrence in occurrences}
    configured_uids = set(priorities)
    missing = sorted(source_uids - configured_uids)
    obsolete = sorted(configured_uids - source_uids)
    if not missing and not obsolete:
        return
    details = []
    if missing:
        details.append("Missing trace priorities: " + ", ".join(missing))
    if obsolete:
        details.append("Obsolete trace priorities: " + ", ".join(obsolete))
    raise ScannerError("\n".join(details))


def loadExistingStates(
    output_path: Path,
) -> tuple[str, dict[str, str], dict[str, str]]:
    """Load valid persistent states from one existing generated catalogue."""

    if not output_path.is_file():
        return "disable", {}, {}
    try:
        root = ElementTree.parse(output_path).getroot()
    except (ElementTree.ParseError, OSError):
        return "disable", {}, {}
    if root.tag != "xwalkTraceCatalogue" or root.get("version") != "1.0":
        return "disable", {}, {}

    valid_states = {"enable", "disable"}
    global_state = root.get("defaultState", "disable")
    if global_state not in valid_states:
        global_state = "disable"
    module_states: dict[str, str] = {}
    trace_states: dict[str, str] = {}
    for module in root.findall("./module"):
        module_name = module.get("name")
        module_state = module.get("defaultState")
        if module_name is not None and module_state in valid_states:
            module_states[module_name] = module_state
        for trace in module.findall("trace"):
            uid = trace.get("fullId")
            trace_state = trace.get("defaultState")
            if uid is not None and trace_state in valid_states:
                trace_states[uid] = trace_state
    return global_state, module_states, trace_states


def generateXml(occurrences: list[TraceOccurrence], output_path: Path) -> str:
    """Render one deterministic catalogue while preserving known trace states."""

    global_state, module_states, trace_states = loadExistingStates(output_path)
    modules: dict[str, list[TraceOccurrence]] = {}
    for occurrence in occurrences:
        modules.setdefault(occurrence.tag, []).append(occurrence)
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<xwalkTraceCatalogue version="1.0" defaultState={quoteattr(global_state)}>',
    ]
    for module_name in sorted(modules):
        lines.append(
            f'  <module name={quoteattr(module_name)} '
            f'defaultState={quoteattr(module_states.get(module_name, global_state))}>'
        )
        ordered = sorted(
            modules[module_name],
            key=lambda trace: (int(trace.numeric_id), trace.numeric_id),
        )
        for trace in ordered:
            attributes = [
                ("id", trace.numeric_id),
                ("fullId", trace.uid),
                ("defaultState", trace_states.get(trace.uid, "disable")),
                ("name", trace.trace_format),
                ("sourceFile", trace.source_file),
                ("sourceLine", str(trace.source_line)),
                ("priority", str(trace.priority)),
                ("formatArgumentCount", str(trace.format_argument_count)),
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
        self, project_root: Path, source_roots: list[Path], output_path: Path,
        priority_path: Path | None = None,
    ) -> None:
        """Retain resolved build inputs without scanning during construction."""

        self.project_root = project_root.resolve()
        self.source_roots = [path.resolve() for path in source_roots]
        self.output_path = output_path.resolve()
        self.priority_path = priority_path.resolve() if priority_path is not None else None

    def run(self) -> list[TraceOccurrence]:
        """Scan, validate, and generate metadata for the configured source roots."""

        occurrences: list[TraceOccurrence] = []
        priorities = loadPriorities(self.priority_path) if self.priority_path is not None else None
        for source_path in collectSources(self.source_roots, self.project_root):
            relative_path = source_path.relative_to(self.project_root).as_posix()
            source_text = source_path.read_text(encoding="utf-8")
            try:
                occurrences.extend(scanSource(source_text, relative_path, priorities))
            except ScannerError as error:
                raise ScannerError(f"{relative_path}: {error}") from error
        occurrences.sort(
            key=lambda trace: (trace.source_file, trace.source_line, trace.macro_name)
        )
        validateUniqueness(occurrences)
        if priorities is not None:
            validatePriorityCoverage(occurrences, priorities)
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
    parser.add_argument("--priority-config", type=Path)
    arguments = parser.parse_args()
    try:
        pre_compiler = XWalkTracePreCompiler(
            arguments.project_root,
            arguments.source_root,
            arguments.output,
            arguments.priority_config,
        )
        occurrences = pre_compiler.run()
    except (OSError, ScannerError, UnicodeError) as error:
        print(f"xWalk trace metadata generation failed:\n{error}", file=sys.stderr)
        return 1
    print(f"Validated {len(occurrences)} tagged trace identifier(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
