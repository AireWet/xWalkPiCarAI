#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/gerrit/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-submodule-migrate.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config
source_root="$(git rev-parse --show-toplevel)"
output="${XWALK_INTEGRATION_OUTPUT_DIR:-}"

plan_component()
{
    local component="$1" verification=""
    [[ "$XWALK_MODE" != "dry-run" ]] || verification="Fixed allowlist entry validated"
    printf '[%s] convert %s to Gerrit submodule\n' "$XWALK_MODE" "$component"
    xwalk_log "add-submodule" "submodule" "MyPiCarX" "$component" "tracked-directory" \
        "exact-gerrit-gitlink" "Convert component directory to a Gerrit submodule" \
        "The private integration repository must record an exact verified component commit." \
        "$XWALK_STATUS" "$verification"
    xwalk_log "set-submodule-url" "submodule" "MyPiCarX" ".gitmodules:$component:url" \
        "none" "../$component" "Configure relative Gerrit submodule URL" \
        "Relative URLs preserve the authenticated Gerrit host without a developer username." \
        "$XWALK_STATUS" "$verification"
    xwalk_log "set-submodule-branch" "submodule" "MyPiCarX" ".gitmodules:$component:branch" \
        "none" "main" "Record the maintenance branch" \
        "Uplift discovery uses main while builds remain pinned to the gitlink." "$XWALK_STATUS" \
        "$verification"
}

apply_component()
{
    local component="$1" url commit
    url="ssh://$GERRIT_ADMIN_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$component"
    git -C "$output" rm -r --quiet -- "$component"
    git -C "$output" submodule add --force -b main "$url" "$component"
    git -C "$output" config -f .gitmodules "submodule.$component.url" "../$component"
    commit="$(git -C "$output/$component" rev-parse HEAD)"
    git -C "$output" add .gitmodules "$component"
    xwalk_log "add-submodule" "submodule" "MyPiCarX" "$component" "tracked-directory" "$commit" \
        "Added exact Gerrit component gitlink" \
        "The integration repository records a reproducible verified component revision." "Applied" \
        "gitlink and .gitmodules entry staged" "" "" "" "" "$commit"
    xwalk_log "set-submodule-url" "submodule" "MyPiCarX" ".gitmodules:$component:url" \
        "generated-absolute-url" "../$component" "Configured relative Gerrit submodule URL" \
        "The URL inherits the authenticated Gerrit endpoint and contains no developer username." \
        "Applied" "git config -f .gitmodules read-back succeeded"
    xwalk_log "set-submodule-branch" "submodule" "MyPiCarX" ".gitmodules:$component:branch" \
        "unset" "main" "Configured submodule maintenance branch" \
        "Automated uplift checks target the component main branch." "Applied" \
        "git config -f .gitmodules read-back succeeded"
}

main()
{
    local component
    for component in "${xwalk_components[@]}"; do
        [[ -d "$source_root/$component" ]] || { echo "Missing allowlisted component: $component" >&2; return 2; }
        [[ "$XWALK_MODE" == "dry-run" ]] && plan_component "$component"
    done
    [[ "$XWALK_MODE" == "apply" ]] || return
    [[ -z "$(git -C "$source_root" status --porcelain)" ]] || {
        echo "Submodule migration requires a clean source repository" >&2
        return 2
    }
    [[ "${XWALK_CONFIRM_SUBMODULES:-}" == "CREATE_INTEGRATION_CLONE" ]] || {
        echo "Set XWALK_CONFIRM_SUBMODULES=CREATE_INTEGRATION_CLONE for explicit apply" >&2
        return 2
    }
    output="$(xwalk_new_output_path "$output" "$source_root")"
    echo "Selected integration output: $output"
    git clone --quiet --no-local "$source_root" "$output"
    trap 'rm -rf -- "$output"' ERR INT TERM
    for component in "${xwalk_components[@]}"; do apply_component "$component"; done
    git -C "$output" submodule sync --recursive
    git -C "$output" submodule update --init --recursive
    # shellcheck disable=SC2016 # Git expands $name within each submodule process.
    git -C "$output" submodule foreach --quiet \
        'if git remote -v | grep -qi github; then echo "Forbidden GitHub remote in $name" >&2; exit 1; fi'
    git -C "$output" -c user.name=xWalk-Automation -c user.email=automation.invalid \
        commit -s -m "Convert xWalk components to Gerrit submodules"
    xwalk_log "validate-submodules" "validation" "MyPiCarX" ".gitmodules" "absent" "valid" \
        "Validated exact Gerrit submodules" \
        "Reproducible integration requires exact commits and no component GitHub remotes." "Verified" \
        "Recursive initialization and remote policy checks succeeded"
    trap - ERR INT TERM
    printf 'Prepared integration clone: %s\n' "$output"
}

main
