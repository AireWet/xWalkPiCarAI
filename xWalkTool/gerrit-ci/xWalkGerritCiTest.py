#!/usr/bin/env python3
"""Test owner-aware GitHub destination selection for Gerrit submissions."""

from __future__ import annotations

import unittest

from xWalkGerritCi import XWalkGerritCi


class XWalkGerritCiTest(unittest.TestCase):
    """Verify that only a directly applicable Joxy change targets master."""

    def test_joxy_change_targets_master(self) -> None:
        """Allow Joxy when GitHub is exactly at the submitted change parent."""

        branch = XWalkGerritCi.select_github_branch(
            "joxjoh24@student.hh.se",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "master")

    def test_joxy_email_comparison_is_case_insensitive(self) -> None:
        """Treat email casing as insignificant for the configured owner."""

        branch = XWalkGerritCi.select_github_branch(
            "JOXJOH24@STUDENT.HH.SE",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "master")

    def test_other_owner_targets_review_branch(self) -> None:
        """Route every non-Joxy owner through a GitHub pull request."""

        branch = XWalkGerritCi.select_github_branch(
            "partner@example.com",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "gerrit-submitted")

    def test_missing_owner_targets_review_branch(self) -> None:
        """Fail safely when Gerrit does not expose an owner email."""

        branch = XWalkGerritCi.select_github_branch(
            "",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "gerrit-submitted")

    def test_stacked_change_targets_review_branch(self) -> None:
        """Prevent a later Joxy change from bypassing an earlier pending PR."""

        branch = XWalkGerritCi.select_github_branch(
            "joxjoh24@student.hh.se",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "gerrit-parent",
            "github-tip",
        )
        self.assertEqual(branch, "gerrit-submitted")


if __name__ == "__main__":
    unittest.main()
