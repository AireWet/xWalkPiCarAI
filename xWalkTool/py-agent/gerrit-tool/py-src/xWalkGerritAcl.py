#!/usr/bin/env python3
"""Generate and reconcile the fixed xWalk Gerrit access matrix."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import sys


PUBLIC = {"DevloperNote", "xWalkHal", "xWalkLibrary", "xWalkTrace"}
PARTNER_REVIEW = {"DevloperNote", "xWalkHal", "xWalkController", "xWalkLibrary", "xWalkTrace"}
PARTNER_READ = PARTNER_REVIEW | {"xWalkIW", "xWalkAgent"}
PRIVATE_PARTNER = {"xWalkAudioResources", "xWalk-rpi5"}
REPOSITORIES = (
    "xWalk-Projects",
    "DevloperNote", "xWalkAgent", "xWalkAudioResources", "xWalkController", "xWalkHal",
    "xWalkIW", "xWalkLibrary", "xWalkTrace", "xWalk-rpi5",
)


@dataclass(frozen=True)
class Rule:
    """Describe one independently logged Gerrit ACL operation."""

    ref: str
    permission: str
    group: str
    value: str
    explanation: str


def rules(repository: str, owner: str, partner: str, ci: str, direct_devnote: bool) -> list[Rule]:
    """Return the complete non-inherited permission set for one repository."""

    if repository not in REPOSITORIES:
        raise ValueError(f"Repository is not allowlisted: {repository}")
    result = [
        Rule(
            "refs/*", "exclusiveGroupPermissions", "inherited-read", "read",
            "Stop inherited read grants before applying the project visibility matrix.",
        ),
        Rule("refs/*", "read", owner, "ALLOW", "Owners require complete repository access."),
        Rule("refs/*", "read", ci, "ALLOW", "CI requires read access for deterministic validation."),
        Rule("refs/*", "owner", owner, "ALLOW", "Only owners may administer project ACLs."),
        Rule(
            "refs/meta/config", "push", owner, "ALLOW",
            "Owners require push access to maintain the project ACL configuration.",
        ),
        Rule("refs/heads/*", "push", owner, "ALLOW", "Owners maintain protected branches."),
        Rule("refs/heads/*", "push", owner, "FORCE", "Only owners may perform authorized recovery."),
        Rule("refs/heads/main", "submit", owner, "ALLOW", "Owners submit reviewed changes."),
        Rule("refs/heads/main", "label-Code-Review", owner, "-2..+2", "Owners perform final review."),
        Rule("refs/heads/main", "label-Verified", ci, "-1..+1", "CI records module verification."),
    ]
    if repository in PUBLIC:
        result.append(Rule(
            "refs/*", "read", "Anonymous Users", "ALLOW",
            "Public source is browsable and cloneable.",
        ))
    if repository in PARTNER_READ:
        result.append(Rule("refs/*", "read", partner, "ALLOW", "Partner access follows the approved matrix."))
    else:
        result.append(Rule("refs/*", "read", partner, "DENY", "Partner access is prohibited for this repository."))
    if repository in PARTNER_REVIEW:
        result.append(Rule(
            "refs/for/refs/heads/main", "push", partner, "ALLOW",
            "Partners upload review changes without direct branch push.",
        ))
    if repository == "DevloperNote":
        result.extend([
            Rule("refs/heads/main", "label-Code-Review", partner, "-2..+2", "Partners review documentation."),
            Rule("refs/heads/main", "submit", partner, "ALLOW", "Partners may submit reviewed documentation."),
        ])
        if direct_devnote:
            result.append(Rule(
                "refs/heads/main", "push", partner, "ALLOW",
                "Explicit optional setting permits direct documentation push.",
            ))
    if repository == "xWalk-rpi5":
        result.append(Rule(
            "refs/for/refs/heads/main", "push", ci, "ALLOW",
            "CI uploads verified automatic integration uplifts for review.",
        ))
    return result


def strip_access_sections(content: str) -> str:
    """Remove only existing access sections while retaining unrelated project settings."""

    sections = re.split(r"(?m)(?=^\[)", content)
    retained = [section for section in sections if not section.startswith('[access "')]
    return "".join(retained).rstrip() + "\n"


def config_text(existing: str, repository: str, owner: str, partner: str, ci: str,
                direct_devnote: bool) -> str:
    """Return project.config with canonical explicit access sections."""

    grouped: dict[str, list[Rule]] = {}
    for rule in rules(repository, owner, partner, ci, direct_devnote):
        grouped.setdefault(rule.ref, []).append(rule)
    output = strip_access_sections(existing)
    for ref, ref_rules in grouped.items():
        output += f'\n[access "{ref}"]\n'
        for rule in ref_rules:
            if rule.permission == "exclusiveGroupPermissions":
                output += f"\texclusiveGroupPermissions = {rule.value}\n"
                continue
            action = "deny " if rule.value == "DENY" else "+force " if rule.value == "FORCE" else ""
            range_value = f"{rule.value} " if ".." in rule.value else ""
            output += f"\t{rule.permission} = {action}{range_value}group {rule.group}\n"
    return output


def parser() -> argparse.ArgumentParser:
    """Build plan and rewrite commands."""

    argument_parser = argparse.ArgumentParser(description=__doc__)
    argument_parser.add_argument("--repository", required=True, choices=REPOSITORIES)
    argument_parser.add_argument("--owner-group", required=True)
    argument_parser.add_argument("--partner-group", required=True)
    argument_parser.add_argument("--ci-group", required=True)
    argument_parser.add_argument("--direct-devnote", action="store_true")
    argument_parser.add_argument("--config")
    argument_parser.add_argument("--list", action="store_true")
    return argument_parser


def main() -> int:
    """List tab-separated rules or rewrite one checked-out project configuration."""

    arguments = parser().parse_args()
    try:
        repository_rules = rules(
            arguments.repository, arguments.owner_group, arguments.partner_group,
            arguments.ci_group, arguments.direct_devnote,
        )
        if arguments.list:
            for rule in repository_rules:
                print("\t".join((rule.ref, rule.permission, rule.group, rule.value, rule.explanation)))
            return 0
        if not arguments.config:
            raise ValueError("--config is required unless --list is selected")
        path = Path(arguments.config)
        existing = path.read_text(encoding="utf-8") if path.exists() else ""
        path.write_text(
            config_text(existing, arguments.repository, arguments.owner_group,
                        arguments.partner_group, arguments.ci_group, arguments.direct_devnote),
            encoding="utf-8",
        )
    except (OSError, ValueError) as error:
        print(f"XWALK_GERRIT_ACL: FAILED - {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
