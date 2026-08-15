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
PRIVATE_PARTNER = {
    "xWalkAudioResources", "xWalkTool", "xWalk-rpi5-sim", "xWalk-rpi5", "xWalkPiCarAI",
}
REPOSITORIES = (
    "xWalk-Projects",
    "DevloperNote", "xWalkAgent", "xWalkAudioResources", "xWalkController", "xWalkHal",
    "xWalkIW", "xWalkLibrary", "xWalkTool", "xWalkTrace", "xWalk-rpi5-sim", "xWalk-rpi5",
    "xWalkPiCarAI",
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
    branch = "master"
    branch_ref = f"refs/heads/{branch}"
    review_ref = f"refs/for/{branch_ref}"
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
        Rule(branch_ref, "submit", owner, "ALLOW", "Owners submit reviewed changes."),
        Rule(branch_ref, "label-Code-Review", owner, "-2..+2", "Owners perform final review."),
        Rule(branch_ref, "label-Verified", ci, "-1..+1", "CI records module verification."),
    ]
    if repository == "xWalkPiCarAI":
        result.append(Rule(
            review_ref, "push", owner, "ALLOW",
            "Owners upload integrated changes through review without direct branch push.",
        ))
    else:
        result.extend([
            Rule("refs/heads/*", "push", owner, "ALLOW", "Owners maintain protected branches."),
            Rule("refs/heads/*", "push", owner, "FORCE", "Only owners may perform authorized recovery."),
            Rule(
                "refs/heads/*", "delete", owner, "ALLOW",
                "Only owners may delete a verified obsolete branch reference.",
            ),
        ])
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
            review_ref, "push", partner, "ALLOW",
            "Partners upload review changes without direct branch push.",
        ))
    if repository == "DevloperNote":
        result.extend([
            Rule(branch_ref, "label-Code-Review", partner, "-2..+2", "Partners review documentation."),
            Rule(branch_ref, "submit", partner, "ALLOW", "Partners may submit reviewed documentation."),
        ])
        if direct_devnote:
            result.append(Rule(
                "refs/heads/master", "push", partner, "ALLOW",
                "Explicit optional setting permits direct documentation push.",
            ))
    if repository in {"xWalk-rpi5", "xWalkPiCarAI"}:
        result.extend([
            Rule(
                review_ref, "push", ci, "ALLOW",
                "CI uploads automatic integration uplifts for review.",
            ),
            Rule(
                branch_ref, "submit", ci, "ALLOW",
                "The CI service may submit only after Gerrit requirements are satisfied.",
            ),
        ])
    return result


def strip_managed_sections(content: str) -> str:
    """Remove managed ACL and submit sections while retaining unrelated settings."""

    sections = re.split(r"(?m)(?=^\[)", content)
    managed_sections = {
        '[label "Verified"]',
        '[submit-requirement "Code-Review"]',
        '[submit-requirement "Verified"]',
        '[submit-requirement "No-Unresolved-Comments"]',
    }
    retained = [
        section for section in sections
        if not section.startswith('[access "')
        and (not section.splitlines() or section.splitlines()[0] not in managed_sections)
    ]
    return "".join(retained).rstrip() + "\n"


def config_text(existing: str, repository: str, owner: str, partner: str, ci: str,
                direct_devnote: bool) -> str:
    """Return project.config with canonical explicit access sections."""

    grouped: dict[str, list[Rule]] = {}
    for rule in rules(repository, owner, partner, ci, direct_devnote):
        grouped.setdefault(rule.ref, []).append(rule)
    output = strip_managed_sections(existing)
    for ref, ref_rules in grouped.items():
        output += f'\n[access "{ref}"]\n'
        for rule in ref_rules:
            if rule.permission == "exclusiveGroupPermissions":
                output += f"\texclusiveGroupPermissions = {rule.value}\n"
                continue
            action = "deny " if rule.value == "DENY" else "+force " if rule.value == "FORCE" else ""
            range_value = f"{rule.value} " if ".." in rule.value else ""
            output += f"\t{rule.permission} = {action}{range_value}group {rule.group}\n"
    if repository != "xWalk-Projects":
        output += (
            '\n[label "Verified"]\n'
            '\tfunction = NoBlock\n'
            '\tdefaultValue = 0\n'
            '\tvalue = -1 Fails\n'
            '\tvalue = 0 No score\n'
            '\tvalue = +1 Verified\n'
            '\tcopyCondition = changekind:NO_CHANGE OR changekind:TRIVIAL_REBASE\n'
            '\n[submit-requirement "Code-Review"]\n'
            '\tdescription = Require an authorized reviewer on the current patch set\n'
            '\tsubmittableIf = label:Code-Review=MAX\n'
            '\tcanOverrideInChildProjects = false\n'
            '\n[submit-requirement "Verified"]\n'
            '\tdescription = Require complete integrated CI on the current patch set\n'
            '\tsubmittableIf = label:Verified=MAX\n'
            '\tcanOverrideInChildProjects = false\n'
            '\n[submit-requirement "No-Unresolved-Comments"]\n'
            '\tdescription = Require every blocking review comment to be resolved\n'
            '\tsubmittableIf = -has:unresolved\n'
            '\tcanOverrideInChildProjects = false\n'
        )
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
