#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/py-agent/gerrit-tool/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"
mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-github-sync.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config

validate_remote()
{
    local value="$GITHUB_XWALK_RPI5_REMOTE" path
    [[ -n "$value" ]] || { echo "Set GITHUB_XWALK_RPI5_REMOTE" >&2; return 2; }
    path="${value##*:}"
    path="${path##*/}"
    path="${path%.git}"
    [[ "$path" == "xWalk-rpi5" ]] || {
        echo "GitHub destination repository must be named exactly xWalk-rpi5" >&2
        return 2
    }
    [[ "$GITHUB_XWALK_RPI5_BRANCH" == "main" ]] || {
        echo "Only GitHub xWalk-rpi5/main synchronization is supported" >&2
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
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "unknown" \
            "verified-gerrit-main" "Plan the sole permitted GitHub synchronization" \
            "Only a submitted and integration-verified xWalk-rpi5 commit may reach GitHub." "Planned" \
            "Destination name and exact refspec validated"
        return
    fi
    [[ "$GITHUB_PUSH_ENABLED" == "true" ]] || {
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "unchanged" "unchanged" \
            "Skipped GitHub synchronization" "GITHUB_PUSH_ENABLED is not true." "Skipped" \
            "No remote write attempted"
        return
    }
    [[ -n "$GITHUB_DIRECT_PUSH_OWNER_EMAIL" && \
        "${submitted_owner,,}" == "${GITHUB_DIRECT_PUSH_OWNER_EMAIL,,}" ]] || {
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "unchanged" "unchanged" \
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
    git -C "$work" remote add origin \
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/xWalk-rpi5"
    if ! git -C "$work" fetch --quiet origin refs/heads/main:refs/remotes/origin/main; then
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "unknown" "unchanged" \
            "Gerrit main fetch failed" "Synchronization requires the submitted Gerrit integration branch." \
            "Failed" "" "Sanitized Gerrit fetch failure"
        return 1
    fi
    git -C "$work" switch --quiet --create main refs/remotes/origin/main
    git -C "$work" reset --keep refs/remotes/origin/main
    fetched="$(git -C "$work" rev-parse refs/heads/main)"
    [[ "$fetched" == "$revision" ]] || {
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "$fetched" "unchanged" \
            "Rejected unverified Gerrit main" \
            "The submitted Gerrit tip does not match the integration-verified commit." "Failed" \
            "Fetched exact Gerrit main" "Verified commit mismatch" "" "$fetched" "$revision"
        return 1
    }
    [[ "$(git -C "$work" remote get-url origin)" == */xWalk-rpi5 ]] || {
        echo "Current repository is not the Gerrit xWalk-rpi5 integration repository" >&2
        return 2
    }
    git -C "$work" remote add github "$GITHUB_XWALK_RPI5_REMOTE"
    if git -C "$work" fetch --quiet github refs/heads/main:refs/remotes/github/main 2>/dev/null; then
        git -C "$work" merge-base --is-ancestor refs/remotes/github/main refs/heads/main || {
            echo "GitHub main is not an ancestor of Gerrit main; refusing non-fast-forward push" >&2
            return 1
        }
    fi
    if git -C "$work" push github refs/heads/main:refs/heads/main; then
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "previous" "$revision" \
            "Synchronized verified integration commit" \
            "Only submitted xWalk-rpi5 integration history is permitted on GitHub." "Verified" \
            "Exact non-force main-to-main refspec succeeded" "" "" "" "" "$revision"
    else
        xwalk_log "github-sync" "GitHub" "xWalk-rpi5" "refs/heads/main" "previous" "unchanged" \
            "GitHub synchronization failed" "The exact protected push was rejected." "Failed" \
            "" "Sanitized git push failure"
        return 1
    fi
    rm -rf -- "$work"
    trap - EXIT
}

main
