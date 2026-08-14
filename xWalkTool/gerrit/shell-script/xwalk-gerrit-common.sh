#!/usr/bin/env bash
set -Eeuo pipefail

xwalk_dir="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
xwalk_root="$(CDPATH='' cd -- "$xwalk_dir/.." && pwd -P)"
xwalk_config="${XWALK_GERRIT_MULTI_REPO_CONFIG:-$xwalk_root/config/multi-repo.conf}"
xwalk_logger="$xwalk_root/py-src/xWalkGerritChangeLog.py"

# shellcheck disable=SC2034 # These arrays are consumed by scripts that source this helper.
xwalk_repositories=(
    DevloperNote xWalkAgent xWalkAudioResources xWalkController xWalkHal
    xWalkIW xWalkLibrary xWalkTool xWalkTrace MyPiCarX
)
# shellcheck disable=SC2034 # Consumed by scripts that source this helper.
xwalk_components=(
    DevloperNote xWalkAgent xWalkAudioResources xWalkController xWalkHal
    xWalkIW xWalkLibrary xWalkTool xWalkTrace
)

xwalk_load_config()
{
    [[ -r "$xwalk_config" ]] || { echo "Missing configuration: $xwalk_config" >&2; return 2; }
    # shellcheck source=/dev/null
    source "$xwalk_config"
    : "${GERRIT_SERVER_HOST:?Set GERRIT_SERVER_HOST}"
    : "${GERRIT_SSH_PORT:=29418}"
    : "${GERRIT_ADMIN_USERNAME:?Set GERRIT_ADMIN_USERNAME}"
    : "${GERRIT_CI_USERNAME:=xwalk-ci}"
    : "${GERRIT_OWNER_GROUP:=xWalk-Owners}"
    : "${GERRIT_PARTNER_GROUP:=xWalk-Partners}"
    : "${GERRIT_CI_GROUP:=xWalk-CI}"
    : "${GERRIT_PARTNER_DEVNOTE_DIRECT_PUSH:=false}"
    : "${GERRIT_UPLIFT_AUTO_SUBMIT:=false}"
    : "${GITHUB_MYPICARX_REMOTE:=}"
    : "${GITHUB_MYPICARX_BRANCH:=main}"
    : "${GITHUB_PUSH_ENABLED:=false}"
    case "$GERRIT_SERVER_HOST:$GERRIT_ADMIN_USERNAME" in
        *FROM_ASSESSMENT*|*GERRIT_ADMIN_USERNAME*)
            echo "Resolve Gerrit host and administrator placeholders in $xwalk_config" >&2
            return 2
            ;;
    esac
    [[ "$GERRIT_SERVER_HOST" =~ ^[A-Za-z0-9.-]+$ ]] || {
        echo "GERRIT_SERVER_HOST contains unsupported characters" >&2
        return 2
    }
    [[ "$GERRIT_ADMIN_USERNAME" =~ ^[A-Za-z0-9._-]+$ ]] || {
        echo "GERRIT_ADMIN_USERNAME contains unsupported characters" >&2
        return 2
    }
    [[ "$GERRIT_CI_USERNAME" =~ ^[A-Za-z0-9._-]+$ ]] || {
        echo "GERRIT_CI_USERNAME contains unsupported characters" >&2
        return 2
    }
    local group setting
    for group in "$GERRIT_OWNER_GROUP" "$GERRIT_PARTNER_GROUP" "$GERRIT_CI_GROUP"; do
        [[ "$group" =~ ^[A-Za-z0-9._[:space:]-]+$ ]] || {
            echo "Gerrit group contains unsupported characters: $group" >&2
            return 2
        }
    done
    for setting in "$GERRIT_PARTNER_DEVNOTE_DIRECT_PUSH" "$GERRIT_UPLIFT_AUTO_SUBMIT" \
        "$GITHUB_PUSH_ENABLED"; do
        [[ "$setting" == true || "$setting" == false ]] || {
            echo "Boolean configuration values must be true or false" >&2
            return 2
        }
    done
    if [[ ! "$GERRIT_SSH_PORT" =~ ^[0-9]+$ ]] ||
        ((GERRIT_SSH_PORT <= 1024 || GERRIT_SSH_PORT >= 65536)); then
        echo "GERRIT_SSH_PORT must be an unprivileged valid port" >&2
        return 2
    fi
}

xwalk_mode()
{
    case "${1:---dry-run}" in
        --dry-run) XWALK_MODE="dry-run"; XWALK_STATUS="Planned" ;;
        --apply) XWALK_MODE="apply"; XWALK_STATUS="Applied" ;;
        *) echo "Expected --dry-run or --apply" >&2; return 2 ;;
    esac
    export XWALK_MODE XWALK_STATUS
}

xwalk_repository()
{
    local candidate="$1" repository
    for repository in "${xwalk_repositories[@]}"; do
        [[ "$candidate" == "$repository" ]] && return 0
    done
    echo "Repository is not in the fixed allowlist: $candidate" >&2
    return 2
}

xwalk_new_output_path()
{
    local candidate="$1" source_root="$2" resolved parent
    [[ -n "$candidate" && "$candidate" == /* ]] || {
        echo "Output path must be absolute" >&2
        return 2
    }
    resolved="$(realpath -m -- "$candidate")"
    [[ "$resolved" == "$candidate" ]] || {
        echo "Output path must be normalized and must not traverse symbolic paths: $candidate" >&2
        return 2
    }
    case "$resolved" in
        /|/home|/tmp|/var|/usr|/etc|"$HOME")
            echo "Refusing unsafe broad output path: $resolved" >&2
            return 2
            ;;
    esac
    case "$resolved/" in
        "$source_root"/*)
            echo "Output path must remain outside the source repository" >&2
            return 2
            ;;
    esac
    [[ ! -e "$resolved" ]] || { echo "Output path already exists: $resolved" >&2; return 2; }
    parent="$(dirname -- "$resolved")"
    [[ -d "$parent" && -w "$parent" ]] || {
        echo "Output parent must exist and be writable: $parent" >&2
        return 2
    }
    printf '%s\n' "$resolved"
}

xwalk_log()
{
    local operation="$1" category="$2" repository="$3" target="$4"
    local previous="$5" new="$6" summary="$7" explanation="$8"
    local status="$9" verification="${10:-}" error="${11:-}" related="${12:-}"
    local before="${13:-}" after="${14:-}"
    python3 "$xwalk_logger" \
        --operation="$operation" --category="$category" --repository="$repository" \
        --target="$target" --previous-value="$previous" --new-value="$new" \
        --change-summary="$summary" --change-explanation="$explanation" \
        --requested-by="${XWALK_REQUESTED_BY:-user}" \
        --executed-by="${XWALK_EXECUTED_BY:-automation}" --mode="$XWALK_MODE" \
        --status="$status" --verification="$verification" --error-summary="$error" \
        --related-change="$related" --commit-before="$before" --commit-after="$after"
}

xwalk_ssh()
{
    local argument command=""
    for argument in "$@"; do
        printf -v argument '%q' "$argument"
        command+="${command:+ }$argument"
    done
    ssh -o BatchMode=yes -o IdentitiesOnly=yes -p "$GERRIT_SSH_PORT" \
        "$GERRIT_ADMIN_USERNAME@$GERRIT_SERVER_HOST" "$command"
}
