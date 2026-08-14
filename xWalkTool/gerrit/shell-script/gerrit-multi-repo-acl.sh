#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/gerrit/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-multi-repo-acl.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config
acl_tool="$xwalk_root/py-src/xWalkGerritAcl.py"

acl_arguments()
{
    local repository="$1" direct=()
    [[ "$GERRIT_PARTNER_DEVNOTE_DIRECT_PUSH" == "true" ]] && direct=(--direct-devnote)
    printf '%s\0' --repository "$repository" --owner-group "$GERRIT_OWNER_GROUP" \
        --partner-group "$GERRIT_PARTNER_GROUP" --ci-group "$GERRIT_CI_GROUP" "${direct[@]}"
}

log_rules()
{
    local repository="$1" status="$2" verification="$3" error="${4:-}"
    local -a arguments=()
    local ref permission group value explanation
    mapfile -d '' -t arguments < <(acl_arguments "$repository")
    while IFS=$'\t' read -r ref permission group value explanation; do
        xwalk_log "acl-$permission" "ACL" "$repository" "$ref:$group" "reconciled-state" "$value" \
            "Reconciled one Gerrit permission" "$explanation" "$status" "$verification" "$error"
    done < <(python3 "$acl_tool" "${arguments[@]}" --list)
}

group_uuid()
{
    local name="$1" listing="$2" uuid
    uuid="$(awk -v name="$name" 'substr($0, length($0) - length(name) + 1) == name {print $1; exit}' \
        <<< "$listing")"
    [[ -n "$uuid" ]] || { echo "Cannot resolve Gerrit group UUID: $name" >&2; return 1; }
    printf '%s\n' "$uuid"
}

apply_acl()
{
    local repository="$1" work listing before after verify
    local -a arguments=()
    work="$(mktemp -d -t xwalk-gerrit-acl-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    listing="$(xwalk_ssh gerrit ls-groups --verbose)"
    git -C "$work" init --quiet
    git -C "$work" remote add origin \
        "ssh://$GERRIT_ADMIN_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$repository"
    git -C "$work" fetch --quiet origin refs/meta/config
    git -C "$work" checkout --quiet --detach FETCH_HEAD
    before="$(git -C "$work" rev-parse HEAD)"
    mapfile -d '' -t arguments < <(acl_arguments "$repository")
    python3 "$acl_tool" "${arguments[@]}" --config "$work/project.config"
    local name uuid
    for name in "$GERRIT_OWNER_GROUP" "$GERRIT_PARTNER_GROUP" "$GERRIT_CI_GROUP" \
        "Anonymous Users" "Registered Users"; do
        uuid="$(group_uuid "$name" "$listing")"
        if ! grep -F -q -- "$uuid" "$work/groups" 2>/dev/null; then
            printf '%s\t%s\n' "$uuid" "$name" >> "$work/groups"
        fi
    done
    git -C "$work" add project.config groups
    if git -C "$work" diff --cached --quiet; then
        log_rules "$repository" "Skipped" "Existing refs/meta/config matches the fixed matrix"
        rm -rf -- "$work"
        trap - EXIT
        return
    fi
    git -C "$work" -c user.name=xWalk-Automation -c user.email=automation.invalid \
        commit --quiet -m "Configure xWalk repository access"
    after="$(git -C "$work" rev-parse HEAD)"
    if ! git -C "$work" push --quiet origin HEAD:refs/meta/config; then
        log_rules "$repository" "Failed" "" "refs/meta/config push failed"
        return 1
    fi
    log_rules "$repository" "Applied" "refs/meta/config push succeeded"
    git -C "$work" fetch --quiet origin refs/meta/config
    verify="$(git -C "$work" show FETCH_HEAD:project.config | sha256sum | awk '{print $1}')"
    [[ "$verify" == "$(sha256sum "$work/project.config" | awk '{print $1}')" ]] || {
        log_rules "$repository" "Failed" "" "Read-back configuration differs from the applied ACL"
        return 1
    }
    log_rules "$repository" "Verified" "Fetched refs/meta/config matches the applied configuration"
    printf 'ACL verified for %s: %s -> %s\n' "$repository" "$before" "$after"
    rm -rf -- "$work"
    trap - EXIT
}

main()
{
    local repository
    for repository in xWalk-Projects "${xwalk_repositories[@]}"; do
        if [[ "$XWALK_MODE" == "dry-run" ]]; then
            printf '[dry-run] reconcile ACL: %s\n' "$repository"
            log_rules "$repository" "Planned" "Rule generated from the fixed access matrix"
        else
            [[ "${XWALK_CONFIRM_ACL:-}" == "APPLY_XWALK_ACL" ]] || {
                echo "Set XWALK_CONFIRM_ACL=APPLY_XWALK_ACL for explicit ACL apply" >&2
                return 2
            }
            apply_acl "$repository"
        fi
    done
}

main
