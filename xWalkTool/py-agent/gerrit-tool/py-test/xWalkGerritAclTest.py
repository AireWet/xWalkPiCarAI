#!/usr/bin/env python3
"""Test the fixed multi-repository Gerrit access matrix."""

from __future__ import annotations

import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).parents[1] / "py-src"))

from xWalkGerritAcl import config_text, rules


class XWalkGerritAclTest(unittest.TestCase):
    """Verify public, partner, owner, and CI permissions independently."""

    def permissions(self, repository: str) -> set[tuple[str, str, str, str]]:
        """Return comparable rules for one repository."""

        return {
            (rule.ref, rule.permission, rule.group, rule.value)
            for rule in rules(repository, "owners", "partners", "ci", False)
        }

    def test_public_repository_allows_anonymous_read(self) -> None:
        """Expose only an explicitly public component for browse and clone."""

        permissions = self.permissions("xWalkHal")
        self.assertIn(("refs/*", "read", "Anonymous Users", "ALLOW"), permissions)
        self.assertIn(("refs/for/refs/heads/master", "push", "partners", "ALLOW"), permissions)

    def test_private_repository_blocks_inherited_read(self) -> None:
        """Stop inherited read without denying explicitly allowed identities."""

        permissions = self.permissions("xWalkAudioResources")
        self.assertIn(
            ("refs/*", "exclusiveGroupPermissions", "inherited-read", "read"), permissions
        )
        self.assertNotIn(("refs/*", "read", "Anonymous Users", "ALLOW"), permissions)
        self.assertNotIn(("refs/*", "read", "Registered Users", "ALLOW"), permissions)
        self.assertIn(("refs/*", "read", "partners", "DENY"), permissions)

    def test_simulation_repository_uses_private_component_access(self) -> None:
        """Keep the simulation component private with owner and CI access."""

        permissions = self.permissions("xWalk-rpi5-sim")
        self.assertIn(("refs/*", "read", "owners", "ALLOW"), permissions)
        self.assertIn(("refs/*", "read", "ci", "ALLOW"), permissions)
        self.assertIn(("refs/*", "read", "partners", "DENY"), permissions)
        self.assertNotIn(("refs/*", "read", "Anonymous Users", "ALLOW"), permissions)

    def test_read_only_partner_has_no_review_push(self) -> None:
        """Keep xWalkIW cloneable by partners without accepting uploads."""

        permissions = self.permissions("xWalkIW")
        self.assertIn(("refs/*", "read", "partners", "ALLOW"), permissions)
        self.assertNotIn(("refs/for/refs/heads/master", "push", "partners", "ALLOW"), permissions)

    def test_ci_can_upload_only_integration_reviews(self) -> None:
        """Grant uplift upload to CI only in the private superproject."""

        integration = self.permissions("xWalk-rpi5")
        component = self.permissions("xWalkAgent")
        permission = ("refs/for/refs/heads/master", "push", "ci", "ALLOW")
        self.assertIn(permission, integration)
        self.assertNotIn(permission, component)

    def test_all_code_repositories_use_master_and_current_submit_requirements(self) -> None:
        """Protect every code repository with the standard branch and submit policy."""

        permissions = self.permissions("xWalkPiCarAI")
        self.assertIn(("refs/for/refs/heads/master", "push", "ci", "ALLOW"), permissions)
        self.assertIn(("refs/heads/master", "label-Verified", "ci", "-1..+1"), permissions)
        self.assertNotIn(("refs/heads/master", "label-Code-Review", "ci", "-2..+2"), permissions)
        self.assertNotIn(("refs/heads/*", "push", "owners", "ALLOW"), permissions)
        configured = config_text("", "xWalkPiCarAI", "owners", "partners", "ci", False)
        self.assertIn('[label "Verified"]', configured)
        self.assertIn('submit-requirement "Code-Review"', configured)
        self.assertIn("label:Verified=MAX", configured)
        self.assertIn("-has:unresolved", configured)
        component = config_text("", "xWalkHal", "owners", "partners", "ci", False)
        self.assertIn('[label "Verified"]', component)
        self.assertIn('submit-requirement "Code-Review"', component)
        self.assertIn("label:Verified=MAX", component)
        self.assertIn("-has:unresolved", component)

    def test_only_owners_can_push_project_access_configuration(self) -> None:
        """Permit the owner group to maintain reviewed repository ACLs."""

        permissions = self.permissions("xWalkHal")
        self.assertIn(("refs/meta/config", "push", "owners", "ALLOW"), permissions)
        self.assertNotIn(("refs/meta/config", "push", "partners", "ALLOW"), permissions)
        self.assertNotIn(("refs/meta/config", "push", "ci", "ALLOW"), permissions)

    def test_only_owners_can_delete_obsolete_branch_references(self) -> None:
        """Permit reviewed branch cleanup without granting deletion to partners or CI."""

        permissions = self.permissions("xWalkHal")
        self.assertIn(("refs/heads/*", "delete", "owners", "ALLOW"), permissions)
        self.assertNotIn(("refs/heads/*", "delete", "partners", "ALLOW"), permissions)
        self.assertNotIn(("refs/heads/*", "delete", "ci", "ALLOW"), permissions)

    def test_xwalk_tool_is_a_private_reviewed_component(self) -> None:
        """Review administration and quality tooling without exposing it to partners."""

        permissions = self.permissions("xWalkTool")
        self.assertIn(("refs/*", "read", "owners", "ALLOW"), permissions)
        self.assertIn(("refs/*", "read", "ci", "ALLOW"), permissions)
        self.assertIn(("refs/*", "read", "partners", "DENY"), permissions)
        self.assertIn(
            ("refs/heads/master", "label-Verified", "ci", "-1..+1"), permissions,
        )

    def test_direct_documentation_push_defaults_off(self) -> None:
        """Require an explicit option before partners can bypass review."""

        default = self.permissions("DevloperNote")
        enabled = {
            (rule.ref, rule.permission, rule.group, rule.value)
            for rule in rules("DevloperNote", "owners", "partners", "ci", True)
        }
        direct = ("refs/heads/master", "push", "partners", "ALLOW")
        self.assertNotIn(direct, default)
        self.assertIn(direct, enabled)

    def test_rewrite_preserves_non_access_configuration(self) -> None:
        """Replace stale access sections without losing project settings."""

        existing = "[project]\n\tdescription = retained\n[access \"refs/*\"]\n\tread = group old\n"
        rewritten = config_text(existing, "xWalkAudioResources", "owners", "partners", "ci", False)
        self.assertIn("description = retained", rewritten)
        self.assertNotIn("group old", rewritten)
        self.assertIn("exclusiveGroupPermissions = read", rewritten)


if __name__ == "__main__":
    unittest.main()
