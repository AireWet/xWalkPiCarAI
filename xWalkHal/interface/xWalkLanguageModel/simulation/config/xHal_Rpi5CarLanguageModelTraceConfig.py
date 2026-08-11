#!/usr/bin/env python3
"""Validate and merge the persistent trace catalogue for xWalkLanguageModel."""
from __future__ import annotations
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET
PREFIX = "xWalkHal/interface/xWalkLanguageModel/"
VALID = {"enable", "disable"}
def states(path: Path):
    if not path.is_file(): return None, {}, {}
    try: root = ET.parse(path).getroot()
    except (ET.ParseError, OSError): return None, {}, {}
    modules, traces = {}, {}
    for module in root.findall("./module"):
        name, state = module.get("name"), module.get("defaultState")
        if name is not None and state in VALID: modules[name] = state
        for trace in module.findall("trace"):
            uid, state = trace.get("fullId"), trace.get("defaultState")
            if uid is not None and state in VALID: traces[uid] = state
    global_state = root.get("defaultState")
    return global_state if global_state in VALID else None, modules, traces
def generate(input_path: Path, output_path: Path) -> int:
    document = ET.parse(input_path)
    root = document.getroot()
    if root.tag != "xwalkTraceCatalogue" or root.get("version") != "1.0":
        raise RuntimeError("Invalid trace catalogue root")
    prior_global, prior_modules, prior_traces = states(output_path)
    global_state = root.get("defaultState", "disable")
    if global_state not in VALID: raise RuntimeError("Invalid global trace state")
    root.set("defaultState", prior_global or global_state)
    count = 0
    for module in root.findall("./module"):
        state = module.get("defaultState")
        if state not in VALID: raise RuntimeError("Invalid module trace state")
        module.set("defaultState", prior_modules.get(module.get("name", ""), state))
    for trace in root.findall("./module/trace"):
        state = trace.get("defaultState")
        if state not in VALID: raise RuntimeError("Invalid trace state")
        trace.set("defaultState", prior_traces.get(trace.get("fullId", ""), state))
        count += int(trace.get("sourceFile", "").startswith(PREFIX))
    if count == 0: raise RuntimeError("No xWalkLanguageModel traces found")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = output_path.with_suffix(output_path.suffix + ".tmp")
    ET.indent(document, space="  ")
    document.write(temporary, encoding="UTF-8", xml_declaration=True)
    temporary.replace(output_path)
    return count
def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    arguments = parser.parse_args()
    count = generate(arguments.input, arguments.output)
    print(f"Validated {count} persistent xWalkLanguageModel trace(s)")
    return 0
if __name__ == "__main__": raise SystemExit(main())
