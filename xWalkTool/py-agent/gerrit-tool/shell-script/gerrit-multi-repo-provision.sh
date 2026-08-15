#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-multi-repo-provision.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config

fail_log()
{
    local operation="$1" category="$2" repository="$3" target="$4" explanation="$5"
    local status=1
    xwalk_log "$operation" "$category" "$repository" "$target" "unknown" "unchanged" \
        "Operation failed" "$explanation" "Failed" "" "Command exited with status $status"
    exit "$status"
}

planned()
{
    printf '[%s] %s: %s\n' "$XWALK_MODE" "$1" "$2"
}

connectivity()
{
    planned "connectivity" "$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        xwalk_log "check-connectivity" "validation" "xWalk-rpi5" "Gerrit SSH" "unchecked" \
            "planned" "Planned Gerrit connectivity check" \
            "Provisioning requires authenticated Gerrit SSH." "Planned"
        return
    fi
    local version
    if ! version="$(xwalk_ssh gerrit version 2>&1)"; then
        xwalk_log "check-connectivity" "validation" "xWalk-rpi5" "Gerrit SSH" "unreachable" \
            "unreachable" "Gerrit connectivity failed" \
            "Provisioning cannot continue without authenticated Gerrit SSH." "Failed" "" "$version"
        return 1
    fi
    xwalk_log "check-connectivity" "validation" "xWalk-rpi5" "Gerrit SSH" "unchecked" \
        "reachable" "Verified Gerrit connectivity" \
        "Provisioning requires authenticated Gerrit SSH." "Verified" "$version"
}

inspect_inheritance()
{
    local work config inherited="none"
    planned "inheritance" "All-Projects refs/meta/config"
    if [[ "$XWALK_MODE" == dry-run ]]; then
        xwalk_log "inspect-inherited-read" "ACL" "All-Projects" "refs/*:read" "unchecked" \
            "planned" "Plan inherited read inspection" \
            "Private repositories must override anonymous and registered-user read grants." "Planned"
        return
    fi
    work="$(mktemp -d -t xwalk-all-projects-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    git -C "$work" init --quiet
    git -C "$work" remote add origin \
        "ssh://$GERRIT_ADMIN_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/All-Projects"
    if ! git -C "$work" fetch --quiet origin refs/meta/config; then
        xwalk_log "inspect-inherited-read" "ACL" "All-Projects" "refs/*:read" "unknown" \
            "unknown" "Inherited read inspection failed" \
            "Provisioning must inspect All-Projects before applying private ACLs." "Failed" "" \
            "Sanitized refs/meta/config fetch failure"
        return 1
    fi
    config="$(git -C "$work" show FETCH_HEAD:project.config)"
    if grep -E -q 'read[[:space:]]*=.*group (Anonymous Users|Registered Users)' <<< "$config"; then
        inherited="anonymous-or-registered-read-present"
    fi
    xwalk_log "inspect-inherited-read" "ACL" "All-Projects" "refs/*:read" "unchecked" \
        "$inherited" "Inspected inherited Gerrit read access" \
        "Every child project uses exclusive read rules to enforce its own visibility matrix." "Verified" \
        "Fetched and parsed All-Projects refs/meta/config"
    rm -rf -- "$work"
    trap - EXIT
}

ensure_group()
{
    local group="$1" existing=""
    planned "group" "$group"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        xwalk_log "ensure-group" "Provisioning" "All-Projects" "$group" "unknown" "present" \
            "Ensure Gerrit group exists" "The access matrix references this dedicated group." "Planned"
        return
    fi
    existing="$(xwalk_ssh gerrit ls-groups 2>/dev/null | grep -F -x -- "$group" || true)"
    if [[ -n "$existing" ]]; then
        xwalk_log "discover-group" "Provisioning" "All-Projects" "$group" "present" "present" \
            "Discovered existing Gerrit group" "Idempotent provisioning preserves the group." "Skipped" \
            "Exact group-name match succeeded"
        return
    fi
    xwalk_ssh gerrit create-group "$group" || fail_log \
        "create-group" "Provisioning" "All-Projects" "$group" "The access matrix requires this group."
    xwalk_log "create-group" "Provisioning" "All-Projects" "$group" "absent" "present" \
        "Created Gerrit group" "The access matrix requires this dedicated group." "Applied"
    existing="$(xwalk_ssh gerrit ls-groups 2>/dev/null | grep -F -x -- "$group" || true)"
    [[ -n "$existing" ]] || fail_log "verify-group" "Provisioning" "All-Projects" "$group" \
        "The newly created group was not returned by Gerrit."
    xwalk_log "verify-group" "Provisioning" "All-Projects" "$group" "present" "present" \
        "Verified Gerrit group" "Provisioning must read back every created group." "Verified" \
        "Exact group-name lookup succeeded"
}

ensure_group_member()
{
    local group="$1" username="$2"
    planned "group-member" "$group: $username"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        xwalk_log "ensure-group-member" "Provisioning" "All-Users" "$group:$username" \
            "unknown" "present" "Ensure Gerrit group membership" \
            "The fixed access matrix requires this identity membership." "Planned"
        return
    fi
    xwalk_ssh gerrit set-members --add "$username" "$group" || fail_log \
        "ensure-group-member" "Provisioning" "All-Users" "$group:$username" \
        "The fixed access matrix requires this identity membership."
    xwalk_log "ensure-group-member" "Provisioning" "All-Users" "$group:$username" \
        "unknown" "present" "Ensured Gerrit group membership" \
        "The fixed access matrix requires this identity membership." "Applied"
}

ensure_project()
{
    local project="$1" existing="" parent="xWalk-Projects" current_parent="" current_head=""
    local description=""
    [[ "$project" == "xWalk-Projects" ]] && parent="All-Projects"
    [[ "$project" == "xWalk-rpi5" ]] && description="Integrated Raspberry Pi 5 xWalk product"
    [[ "$project" == "xWalkPiCarAI" ]] && description="Current integrated xWalkPiCarAI product"
    planned "repository" "$project"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        xwalk_log "ensure-repository" "Provisioning" "$project" "repository" "unknown" "present" \
            "Ensure Gerrit repository exists" "Each architecture component needs an independent review history." \
            "Planned"
        xwalk_log "set-parent" "Provisioning" "$project" "parent" "unknown" "$parent" \
            "Assign permission-only parent" "A common parent prevents unsafe All-Projects inheritance." "Planned"
        if [[ "$project" != "xWalk-Projects" ]]; then
            xwalk_log "set-default-branch" "Provisioning" "$project" "HEAD" "unknown" \
                "refs/heads/master" "Configure master as default" \
                "All module and integration workflows target master." "Planned"
        fi
        if [[ -n "$description" ]]; then
            xwalk_log "set-description" "Provisioning" "$project" "description" "unknown" \
                "$description" "Configure integration repository description" \
                "The private superproject must be identifiable as the Raspberry Pi 5 product." "Planned"
        fi
        return
    fi
    existing="$(xwalk_ssh gerrit ls-projects 2>/dev/null | grep -F -x -- "$project" || true)"
    if [[ -z "$existing" ]]; then
        local -a create_arguments=(
            gerrit create-project --parent "$parent" --owner "$GERRIT_OWNER_GROUP"
        )
        if [[ "$project" == "xWalk-Projects" ]]; then
            create_arguments+=(--permissions-only)
        else
            create_arguments+=(--branch master --empty-commit)
        fi
        [[ -z "$description" ]] || create_arguments+=(--description "$description")
        create_arguments+=("$project")
        xwalk_ssh "${create_arguments[@]}" || fail_log \
            "create-repository" "Provisioning" "$project" "repository" \
            "Independent Gerrit review requires this repository."
        xwalk_log "create-repository" "Provisioning" "$project" "repository" "absent" "present" \
            "Created Gerrit repository" "Independent Gerrit review requires this repository." "Applied"
        existing="$(xwalk_ssh gerrit ls-projects 2>/dev/null | grep -F -x -- "$project" || true)"
        [[ -n "$existing" ]] || fail_log "verify-repository" "Provisioning" "$project" "repository" \
            "The newly created repository was not returned by Gerrit."
        xwalk_log "verify-repository" "Provisioning" "$project" "repository" "present" "present" \
            "Verified Gerrit repository" "Provisioning must read back every created repository." \
            "Verified" "Exact project-name lookup succeeded"
    else
        xwalk_log "discover-repository" "Provisioning" "$project" "repository" "present" "present" \
            "Discovered Gerrit repository" "Existing repository data must be preserved." "Skipped" \
            "Exact project-name match succeeded"
    fi
    if [[ -n "$description" ]]; then
        xwalk_ssh gerrit set-project --description "$description" "$project" || fail_log \
            "set-description" "Provisioning" "$project" "description" \
            "The integration repository requires its fixed product description."
        xwalk_log "set-description" "Provisioning" "$project" "description" "compatible-or-unknown" \
            "$description" "Configured integration repository description" \
            "The private superproject is the Raspberry Pi 5 product integration repository." "Applied" \
            "Gerrit accepted the idempotent description update"
    fi
    current_parent="$(xwalk_ssh gerrit get-project-parent "$project" 2>/dev/null || true)"
    if [[ "$current_parent" == "$parent" ]]; then
        xwalk_log "set-parent" "Provisioning" "$project" "parent" "$parent" "$parent" \
            "Preserved permission-only parent" "The existing parent already isolates permissions." \
            "Skipped" "Exact parent lookup matched"
    else
        xwalk_ssh gerrit set-project-parent --parent "$parent" "$project" || fail_log \
            "set-parent" "Provisioning" "$project" "parent" "Parent assignment is required for ACL isolation."
        xwalk_log "set-parent" "Provisioning" "$project" "parent" "${current_parent:-unknown}" "$parent" \
            "Assigned permission-only parent" "The parent isolates project permissions." "Applied"
    fi
    if [[ "$project" == "xWalk-Projects" ]]; then
        return
    fi
    current_head="$(xwalk_ssh gerrit set-head "$project" 2>/dev/null || true)"
    if [[ "$current_head" == *"refs/heads/master"* ]]; then
        xwalk_log "set-default-branch" "Provisioning" "$project" "HEAD" "refs/heads/master" \
            "refs/heads/master" "Preserved master as default" \
            "The repository already follows the integration branch standard." "Skipped" \
            "Gerrit HEAD lookup matched"
    else
        xwalk_ssh gerrit set-head "$project" --new-head refs/heads/master || fail_log \
            "set-default-branch" "Provisioning" "$project" "HEAD" "The architecture standardizes on master."
        xwalk_log "set-default-branch" "Provisioning" "$project" "HEAD" "${current_head:-unknown}" \
            "refs/heads/master" "Configured master as default" \
            "The architecture standardizes module workflows on master." "Applied"
    fi
}

main()
{
    if [[ "$XWALK_MODE" == "apply" && "${XWALK_CONFIRM_PROVISION:-}" != "CREATE_XWALK_REPOSITORIES" ]]; then
        echo "Set XWALK_CONFIRM_PROVISION=CREATE_XWALK_REPOSITORIES for explicit apply" >&2
        return 2
    fi
    connectivity
    inspect_inheritance
    ensure_group "$GERRIT_OWNER_GROUP"
    ensure_group "$GERRIT_PARTNER_GROUP"
    ensure_group "$GERRIT_CI_GROUP"
    ensure_group_member "$GERRIT_OWNER_GROUP" "$GERRIT_ADMIN_USERNAME"
    ensure_group_member "$GERRIT_CI_GROUP" "$GERRIT_CI_USERNAME"
    ensure_project xWalk-Projects
    local project
    for project in "${xwalk_repositories[@]}"; do
        ensure_project "$project"
    done
    "$script_dir/gerrit-multi-repo-acl.sh" "$mode"
}

main
