#!/usr/bin/env python3
"""Validate and merge the persistent trace catalogue for xWalkSpi."""

from __future__ import annotations

import argparse
from pathlib import Path
import xml.etree.ElementTree as ElementTree


MODULE_SOURCE_PREFIX = "xWalkHal/interface/xWalkSpi/"
VALID_STATES = {"enable", "disable"}


def existingStates(output_path: Path) -> tuple[str | None, dict[str, str], dict[str, str]]:
    """Return valid global, module, and UID states from an existing output."""
    if not output_path.is_file():
        return None, {}, {}
    try:
        root = ElementTree.parse(output_path).getroot()
    except (ElementTree.ParseError, OSError):
        return None, {}, {}

    global_state = root.get("defaultState")
    modules: dict[str, str] = {}
    traces: dict[str, str] = {}
    for module in root.findall("./module"):
        name = module.get("name")
        state = module.get("defaultState")
        if name is not None and state in VALID_STATES:
            modules[name] = state
        for trace in module.findall("trace"):
            uid = trace.get("fullId")
            trace_state = trace.get("defaultState")
            if uid is not None and trace_state in VALID_STATES:
                traces[uid] = trace_state
    return global_state if global_state in VALID_STATES else None, modules, traces


def generateConfiguration(input_path: Path, output_path: Path) -> int:
    """Validate inventory metadata and preserve known persistent output states."""
    document = ElementTree.parse(input_path)
    root = document.getroot()
    if root.tag != "xwalkTraceCatalogue" or root.get("version") != "1.0":
        raise RuntimeError("The trace inventory has an invalid catalogue root")

    prior_global, prior_modules, prior_traces = existingStates(output_path)
    input_global = root.get("defaultState", "disable")
    if input_global not in VALID_STATES:
        raise RuntimeError("The trace inventory has an invalid global state")
    root.set("defaultState", prior_global or input_global)

    trace_count = 0
    for module in root.findall("./module"):
        module_state = module.get("defaultState")
        if module_state not in VALID_STATES:
            raise RuntimeError("Every trace module must use enable or disable")
        module_name = module.get("name", "")
        module.set("defaultState", prior_modules.get(module_name, module_state))
    for trace in root.findall("./module/trace"):
        source_file = trace.get("sourceFile", "")
        module_trace = source_file.startswith(MODULE_SOURCE_PREFIX)
        trace_state = trace.get("defaultState")
        if trace_state not in VALID_STATES:
            raise RuntimeError("Every normal trace must use enable or disable")
        uid = trace.get("fullId", "")
        trace.set("defaultState", prior_traces.get(uid, trace_state))
        trace_count += int(module_trace)

    if trace_count == 0:
        raise RuntimeError("The trace inventory contains no xWalkSpi traces")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = output_path.with_suffix(output_path.suffix + ".tmp")
    ElementTree.indent(document, space="  ")
    document.write(temporary_path, encoding="UTF-8", xml_declaration=True)
    temporary_path.replace(output_path)
    return trace_count


def main() -> int:
    """Parse command-line paths and generate the simulation configuration."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    arguments = parser.parse_args()
    trace_count = generateConfiguration(arguments.input, arguments.output)
    print(f"Validated {trace_count} persistent xWalkSpi trace(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

