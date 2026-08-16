#!/usr/bin/env python3
"""Validate links owned by the developer-note source tree for the documentation tool."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit


MARKDOWN_LINK = re.compile(
    r"!?\[[^\]]*\]\((?P<target><[^>]+>|[^)\s]+)(?:\s+['\"][^)]*['\"])?\)"
)


def parse_arguments() -> argparse.Namespace:
    """Parse the single source-root argument."""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source_root", type=Path)
    return parser.parse_args()


def local_target(raw_target: str) -> str | None:
    """Return a decoded local path, excluding external and same-page targets."""

    target = raw_target.removeprefix("<").removesuffix(">")
    parsed = urlsplit(target)
    if parsed.scheme or parsed.netloc or not parsed.path:
        return None
    return unquote(parsed.path)


def validate_links(source_root: Path) -> list[str]:
    """Report missing targets that resolve inside the wiki-owned source tree."""

    root = source_root.resolve(strict=True)
    findings: list[str] = []
    for markdown_file in sorted(root.rglob("*.md")):
        text = markdown_file.read_text(encoding="utf-8")
        for line_number, line in enumerate(text.splitlines(), start=1):
            for match in MARKDOWN_LINK.finditer(line):
                target = local_target(match.group("target"))
                if target is None:
                    continue
                resolved = (markdown_file.parent / target).resolve(strict=False)
                if not resolved.is_relative_to(root):
                    continue
                if not resolved.exists():
                    relative_file = markdown_file.relative_to(root)
                    findings.append(f"{relative_file}:{line_number}: missing local target: {target}")
    return findings


def main() -> int:
    """Validate the requested wiki source tree and return a CI-friendly status."""

    arguments = parse_arguments()
    findings = validate_links(arguments.source_root)
    if findings:
        for finding in findings:
            print(f"ERROR: {finding}")
        return 1
    print("Wiki-owned Markdown links are valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
