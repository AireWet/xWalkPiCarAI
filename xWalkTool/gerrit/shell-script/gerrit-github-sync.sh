#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/gerrit/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"
mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-github-sync.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config

validate_remote()
{
    local value="$GITHUB_MYPICARX_REMOTE" path
    [[ -n "$value" ]] || { echo "Set GITHUB_MYPICARX_REMOTE" >&2; return 2; }
    path="${value##*:}"
    path="${path##*/}"
    path="${path%.git}"
    [[ "$path" == "MyPiCarX" ]] || {
        echo "GitHub destination repository must be named exactly MyPiCarX" >&2
        return 2
    }
    [[ "$GITHUB_MYPICARX_BRANCH" == "main" ]] || {
        echo "Only GitHub MyPiCarX/main synchronization is supported" >&2
        return 2
    }
}

main()
{
    validate_remote
    local revision="${XWALK_INTEGRATION_VERIFIED_COMMIT:-}" work fetched
    local submitted_owner="${XWALK_SUBMITTED_OWNER_EMAIL:-}"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        echo "[dry-run] git push github refs/heads/main:refs/heads/main"
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "unknown" \
            "verified-gerrit-main" "Plan the sole permitted GitHub synchronization" \
            "Only a submitted and integration-verified MyPiCarX commit may reach GitHub." "Planned" \
            "Destination name and exact refspec validated"
        return
    fi
    [[ "$GITHUB_PUSH_ENABLED" == "true" ]] || {
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "unchanged" "unchanged" \
            "Skipped GitHub synchronization" "GITHUB_PUSH_ENABLED is not true." "Skipped" \
            "No remote write attempted"
        return
    }
    [[ -n "$GITHUB_DIRECT_PUSH_OWNER_EMAIL" && \
        "${submitted_owner,,}" == "${GITHUB_DIRECT_PUSH_OWNER_EMAIL,,}" ]] || {
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "unchanged" "unchanged" \
            "Skipped unauthorized direct synchronization" \
            "Only the configured repository owner may publish directly to GitHub main." "Skipped" \
            "No remote write attempted"
        return
    }
    [[ "$revision" =~ ^[0-9a-fA-F]{40}$ ]] || {
        echo "XWALK_INTEGRATION_VERIFIED_COMMIT must be a full commit ID" >&2
        return 2
    }
    work="$(mktemp -d -t xwalk-github-sync-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    git -C "$work" init --quiet
    git -C "$work" remote add gerrit \
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/MyPiCarX"
    if ! git -C "$work" fetch --quiet gerrit refs/heads/main:refs/heads/main; then
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "unknown" "unchanged" \
            "Gerrit main fetch failed" "Synchronization requires the submitted Gerrit integration branch." \
            "Failed" "" "Sanitized Gerrit fetch failure"
        return 1
    fi
    fetched="$(git -C "$work" rev-parse refs/heads/main)"
    [[ "$fetched" == "$revision" ]] || {
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "$fetched" "unchanged" \
            "Rejected unverified Gerrit main" \
            "The submitted Gerrit tip does not match the integration-verified commit." "Failed" \
            "Fetched exact Gerrit main" "Verified commit mismatch" "" "$fetched" "$revision"
        return 1
    }
    git -C "$work" remote add github "$GITHUB_MYPICARX_REMOTE"
    if git -C "$work" push github refs/heads/main:refs/heads/main; then
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "previous" "$revision" \
            "Synchronized verified integration commit" \
            "Only submitted MyPiCarX integration history is permitted on GitHub." "Verified" \
            "Exact non-force main-to-main refspec succeeded" "" "" "" "" "$revision"
    else
        xwalk_log "github-sync" "GitHub" "MyPiCarX" "refs/heads/main" "previous" "unchanged" \
            "GitHub synchronization failed" "The exact protected push was rejected." "Failed" \
            "" "Sanitized git push failure"
        return 1
    fi
    rm -rf -- "$work"
    trap - EXIT
}

main
