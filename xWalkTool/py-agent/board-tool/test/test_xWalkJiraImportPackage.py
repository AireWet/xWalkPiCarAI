#!/usr/bin/env python3
"""Test the installable xWalk Jira package metadata and entry points."""

from __future__ import annotations

import tomllib
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

import xWalkJiraImport
from xWalkJiraImport.xWalkJiraImportConfig import load_config


PACKAGE_ROOT = Path(__file__).resolve().parents[1]


class XWalkJiraImportPackageTest(unittest.TestCase):
    """Verify distribution metadata and safe installed behavior."""

    def test_distribution_and_console_script_use_repository_names(self) -> None:
        """Package and command names follow the xWalk module convention."""
        metadata = tomllib.loads((PACKAGE_ROOT / "pyproject.toml").read_text(encoding="utf-8"))
        self.assertEqual(metadata["project"]["name"], "xWalkJiraImport")
        self.assertEqual(
            metadata["project"]["scripts"]["xWalkJiraImport"],
            "xWalkJiraImport.xWalkJiraImportApplication:main",
        )

    def test_package_version_matches_distribution_version(self) -> None:
        """The importable and packaged versions remain synchronized."""
        metadata = tomllib.loads((PACKAGE_ROOT / "pyproject.toml").read_text(encoding="utf-8"))
        self.assertEqual(xWalkJiraImport.__version__, metadata["project"]["version"])

    def test_installed_configuration_remains_dry_run_by_default(self) -> None:
        """Packaging cannot weaken the default mutation boundary."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            configuration = load_config(["--netrc-file", str(netrc_path)], {})
        self.assertTrue(configuration.dry_run)
        self.assertFalse(configuration.apply)


if __name__ == "__main__":
    unittest.main()
