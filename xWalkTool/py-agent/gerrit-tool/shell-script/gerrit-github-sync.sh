#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"
mode="${1:---dry-run}"
[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-github-sync.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$mode"
xwalk_load_config

validate_remote()
{
    local value="${GITHUB_INTEGRATION_REMOTE:-$GITHUB_XWALK_RPI5_REMOTE}" path
    [[ -n "$value" ]] || { echo "Set GITHUB_INTEGRATION_REMOTE" >&2; return 2; }
    [[ "$value" == git@* || "$value" == ssh://* ]] || {
        echo "GitHub destination must use SSH credentials" >&2
        return 2
    }
    path="${value##*:}"
    path="${path##*/}"
    path="${path%.git}"
    [[ "$path" == "$GERRIT_INTEGRATION_PROJECT" ]] || {
        echo "GitHub destination repository must match $GERRIT_INTEGRATION_PROJECT" >&2
        return 2
    }
    [[ "${GITHUB_INTEGRATION_BRANCH:-$GITHUB_XWALK_RPI5_BRANCH}" == "$GERRIT_INTEGRATION_BRANCH" ]] || {
        echo "GitHub branch must match the integrated Gerrit branch" >&2
        return 2
    }
}

main()
{
    validate_remote
    local revision="${XWALK_INTEGRATION_VERIFIED_COMMIT:-}" work fetched
    local github_remote="${GITHUB_INTEGRATION_REMOTE:-$GITHUB_XWALK_RPI5_REMOTE}"
    local github_branch="${GITHUB_INTEGRATION_BRANCH:-$GITHUB_XWALK_RPI5_BRANCH}"
    if [[ "$XWALK_MODE" == "dry-run" ]]; then
        echo "[dry-run] git push github refs/heads/$GERRIT_INTEGRATION_BRANCH:refs/heads/$github_branch"
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$GERRIT_INTEGRATION_BRANCH" "unknown" \
            "verified-gerrit-master" "Plan the sole permitted GitHub synchronization" \
            "Only a submitted and integration-verified commit may reach GitHub." "Planned" \
            "Destination name and exact refspec validated"
        return
    fi
    [[ "$GITHUB_PUSH_ENABLED" == "true" ]] || {
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$GERRIT_INTEGRATION_BRANCH" "unchanged" "unchanged" \
            "Skipped GitHub synchronization" "GITHUB_PUSH_ENABLED is not true." "Skipped" \
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
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$GERRIT_INTEGRATION_PROJECT"
    if ! xwalk_retry git -C "$work" fetch --quiet origin \
        "refs/heads/$GERRIT_INTEGRATION_BRANCH:refs/remotes/origin/$GERRIT_INTEGRATION_BRANCH"; then
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$GERRIT_INTEGRATION_BRANCH" "unknown" "unchanged" \
            "Gerrit master fetch failed" "Synchronization requires the submitted Gerrit integration branch." \
            "Failed" "" "Sanitized Gerrit fetch failure"
        return 1
    fi
    git -C "$work" switch --quiet --create "$GERRIT_INTEGRATION_BRANCH" \
        "refs/remotes/origin/$GERRIT_INTEGRATION_BRANCH"
    fetched="$(git -C "$work" rev-parse "refs/heads/$GERRIT_INTEGRATION_BRANCH")"
    [[ "$fetched" == "$revision" ]] || {
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$GERRIT_INTEGRATION_BRANCH" "$fetched" "unchanged" \
            "Rejected unverified Gerrit master" \
            "The submitted Gerrit tip does not match the integration-verified commit." "Failed" \
            "Fetched exact Gerrit master" "Verified commit mismatch" "" "$fetched" "$revision"
        return 1
    }
    [[ "$(git -C "$work" remote get-url origin)" == */"$GERRIT_INTEGRATION_PROJECT" ]] || {
        echo "Current repository is not the configured Gerrit integration repository" >&2
        return 2
    }
    git -C "$work" remote add github "$github_remote"
    if git -C "$work" fetch --quiet github \
        "refs/heads/$github_branch:refs/remotes/github/$github_branch" 2>/dev/null; then
        git -C "$work" merge-base --is-ancestor "refs/remotes/github/$github_branch" \
            "refs/heads/$GERRIT_INTEGRATION_BRANCH" || {
            echo "GitHub is not an ancestor of Gerrit; refusing non-fast-forward push" >&2
            return 1
        }
    fi
    if xwalk_retry git -C "$work" push github \
        "refs/heads/$GERRIT_INTEGRATION_BRANCH:refs/heads/$github_branch" && \
        [[ "$(git -C "$work" ls-remote github "refs/heads/$github_branch" | awk '{print $1}')" == "$revision" ]]; then
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$github_branch" "previous" "$revision" \
            "Synchronized verified integration commit" \
            "Only submitted integrated Gerrit history is permitted on GitHub." "Verified" \
            "Exact non-force branch refspec and read-back succeeded" "" "" "" "" "$revision"
        xwalk_changelog "$GERRIT_INTEGRATION_PROJECT" "GitHub sync" "merged" "$revision" \
            "submitted" "$revision" "success" \
            "Pushed and read back the exact merged Gerrit commit" "$github_remote"
    else
        xwalk_log "github-sync" "GitHub" "$GERRIT_INTEGRATION_PROJECT" \
            "refs/heads/$github_branch" "previous" "unchanged" \
            "GitHub synchronization failed" "The exact protected push was rejected." "Failed" \
            "" "Sanitized git push failure"
        xwalk_changelog "$GERRIT_INTEGRATION_PROJECT" "GitHub sync" "merged" "$revision" \
            "submitted" "$revision" "failed" \
            "GitHub push or exact SHA read-back failed" "$github_remote"
        return 1
    fi
    rm -rf -- "$work"
    trap - EXIT
}

main
