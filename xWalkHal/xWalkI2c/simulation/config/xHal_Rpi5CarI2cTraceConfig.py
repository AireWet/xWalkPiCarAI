#!/usr/bin/env python3
"""Validate and copy the immutable all-disabled trace catalogue for xWalkI2c."""

from __future__ import annotations

import argparse
from pathlib import Path
import xml.etree.ElementTree as ElementTree


MODULE_SOURCE_PREFIX = "xWalkHal/xWalkI2c/"


def generateConfiguration(input_path: Path, output_path: Path) -> int:
    """Validate defaults and atomically copy one generated trace catalogue."""
    document = ElementTree.parse(input_path)
    root = document.getroot()
    if root.tag != "xwalkTraceCatalogue" or root.get("version") != "1.0":
        raise RuntimeError("The trace inventory has an invalid catalogue root")

    enabled_count = 0
    for module in root.findall("./module"):
        if module.get("defaultState") != "disable":
            raise RuntimeError("Every trace module must default to disable")
    for trace in root.findall("./module/trace"):
        source_file = trace.get("sourceFile", "")
        module_trace = source_file.startswith(MODULE_SOURCE_PREFIX)
        if trace.get("defaultState") != "disable":
            raise RuntimeError("Every normal trace must default to disable")
        enabled_count += int(module_trace)

    if enabled_count == 0:
        raise RuntimeError("The trace inventory contains no xWalkI2c traces")

    contents = input_path.read_bytes()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = output_path.with_suffix(output_path.suffix + ".tmp")
    temporary_path.write_bytes(contents)
    temporary_path.replace(output_path)
    return enabled_count


def main() -> int:
    """Parse command-line paths and generate the simulation configuration."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    arguments = parser.parse_args()
    enabled_count = generateConfiguration(arguments.input, arguments.output)
    print(f"Validated {enabled_count} all-disabled xWalkI2c trace(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
