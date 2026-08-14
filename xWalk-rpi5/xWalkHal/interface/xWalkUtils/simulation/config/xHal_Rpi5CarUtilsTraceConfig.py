#!/usr/bin/env python3
"""Validate and merge the persistent trace catalogue for xWalkUtils."""
from __future__ import annotations
import argparse
from pathlib import Path
import xml.etree.ElementTree as ElementTree
MODULE_SOURCE_PREFIX = "xWalkHal/interface/xWalkUtils/"
VALID_STATES = {"enable", "disable"}
def existingStates(path: Path) -> tuple[str | None, dict[str, str], dict[str, str]]:
    if not path.is_file():
        return None, {}, {}
    try:
        root = ElementTree.parse(path).getroot()
    except (ElementTree.ParseError, OSError):
        return None, {}, {}
    modules: dict[str, str] = {}
    traces: dict[str, str] = {}
    for module in root.findall("./module"):
        name, state = module.get("name"), module.get("defaultState")
        if name is not None and state in VALID_STATES:
            modules[name] = state
        for trace in module.findall("trace"):
            uid, trace_state = trace.get("fullId"), trace.get("defaultState")
            if uid is not None and trace_state in VALID_STATES:
                traces[uid] = trace_state
    state = root.get("defaultState")
    return state if state in VALID_STATES else None, modules, traces
def generateConfiguration(input_path: Path, output_path: Path) -> int:
    document = ElementTree.parse(input_path)
    root = document.getroot()
    if root.tag != "xwalkTraceCatalogue" or root.get("version") != "1.0":
        raise RuntimeError("The trace inventory has an invalid catalogue root")
    prior_global, prior_modules, prior_traces = existingStates(output_path)
    global_state = root.get("defaultState", "disable")
    if global_state not in VALID_STATES:
        raise RuntimeError("The trace inventory has an invalid global state")
    root.set("defaultState", prior_global or global_state)
    count = 0
    for module in root.findall("./module"):
        state = module.get("defaultState")
        if state not in VALID_STATES:
            raise RuntimeError("Every trace module must use enable or disable")
        module.set("defaultState", prior_modules.get(module.get("name", ""), state))
    for trace in root.findall("./module/trace"):
        state = trace.get("defaultState")
        if state not in VALID_STATES:
            raise RuntimeError("Every normal trace must use enable or disable")
        trace.set("defaultState", prior_traces.get(trace.get("fullId", ""), state))
        count += int(trace.get("sourceFile", "").startswith(MODULE_SOURCE_PREFIX))
    if count == 0:
        raise RuntimeError("The trace inventory contains no xWalkUtils traces")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = output_path.with_suffix(output_path.suffix + ".tmp")
    ElementTree.indent(document, space="  ")
    document.write(temporary, encoding="UTF-8", xml_declaration=True)
    temporary.replace(output_path)
    return count
def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    arguments = parser.parse_args()
    count = generateConfiguration(arguments.input, arguments.output)
    print(f"Validated {count} persistent xWalkUtils trace(s)")
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
