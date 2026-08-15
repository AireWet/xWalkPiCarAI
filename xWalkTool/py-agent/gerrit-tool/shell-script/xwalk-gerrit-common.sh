#!/usr/bin/env bash
set -Eeuo pipefail

xwalk_dir="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
xwalk_root="$(CDPATH='' cd -- "$xwalk_dir/.." && pwd -P)"
xwalk_config="${XWALK_GERRIT_MULTI_REPO_CONFIG:-$xwalk_root/config/multi-repo.conf}"

# shellcheck disable=SC2034 # These arrays are consumed by scripts that source this helper.
xwalk_repositories=(
    DevloperNote xWalkAgent xWalkAudioResources xWalkController xWalkHal
    xWalkIW xWalkLibrary xWalkTrace xWalk-rpi5-sim xWalk-rpi5
    xWalkPiCarAI
)
# shellcheck disable=SC2034 # Consumed by scripts that source this helper.
xwalk_components=(
    DevloperNote xWalkAgent xWalkAudioResources xWalkController xWalkHal
    xWalkIW xWalkLibrary xWalkTrace xWalk-rpi5-sim
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
    : "${GERRIT_BASE_URL:=ssh://$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT}"
    : "${GERRIT_OWNER_GROUP:=xWalk-Owners}"
    : "${GERRIT_PARTNER_GROUP:=xWalk-Partners}"
    : "${GERRIT_CI_GROUP:=xWalk-CI}"
    : "${GERRIT_INTEGRATION_PROJECT:=xWalkPiCarAI}"
    : "${GERRIT_INTEGRATION_BRANCH:=master}"
    : "${GERRIT_INTEGRATION_SOURCE_ROOT:=xWalk-rpi5}"
    : "${GERRIT_PARTNER_DEVNOTE_DIRECT_PUSH:=false}"
    : "${GERRIT_UPLIFT_AUTO_SUBMIT:=false}"
    : "${GERRIT_UPLIFT_AUTO_REVIEW:=false}"
    : "${GERRIT_UPLIFT_RETRY_ATTEMPTS:=3}"
    : "${GERRIT_UPLIFT_RETRY_DELAY_SECONDS:=5}"
    : "${GITHUB_XWALK_RPI5_REMOTE:=}"
    : "${GITHUB_XWALK_RPI5_BRANCH:=main}"
    : "${GITHUB_INTEGRATION_REMOTE:=$GITHUB_XWALK_RPI5_REMOTE}"
    : "${GITHUB_INTEGRATION_BRANCH:=$GERRIT_INTEGRATION_BRANCH}"
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
    [[ "$GERRIT_BASE_URL" =~ ^(ssh|https?)://[^[:space:]]+$ ]] || {
        echo "GERRIT_BASE_URL must be an SSH or HTTP(S) Gerrit base URL" >&2
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
        "$GERRIT_UPLIFT_AUTO_REVIEW" "$GITHUB_PUSH_ENABLED"; do
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
    [[ "$GERRIT_INTEGRATION_PROJECT" =~ ^[A-Za-z0-9._-]+$ ]] || {
        echo "GERRIT_INTEGRATION_PROJECT contains unsupported characters" >&2
        return 2
    }
    [[ "$GERRIT_INTEGRATION_BRANCH" =~ ^[A-Za-z0-9._/-]+$ ]] || {
        echo "GERRIT_INTEGRATION_BRANCH contains unsupported characters" >&2
        return 2
    }
    [[ "$GERRIT_INTEGRATION_SOURCE_ROOT" =~ ^[A-Za-z0-9._/-]+$ ]] || {
        echo "GERRIT_INTEGRATION_SOURCE_ROOT contains unsupported characters" >&2
        return 2
    }
    [[ "$GERRIT_UPLIFT_RETRY_ATTEMPTS" =~ ^[1-9][0-9]*$ ]] || {
        echo "GERRIT_UPLIFT_RETRY_ATTEMPTS must be positive" >&2
        return 2
    }
    [[ "$GERRIT_UPLIFT_RETRY_DELAY_SECONDS" =~ ^[0-9]+$ ]] || {
        echo "GERRIT_UPLIFT_RETRY_DELAY_SECONDS must be non-negative" >&2
        return 2
    }
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

xwalk_component_path()
{
    case "$1" in
        DevloperNote) printf '%s\n' "devloper-note" ;;
        xWalkAgent|xWalkAudioResources|xWalkController|xWalkHal|xWalkIW|xWalkLibrary|xWalkTrace|xWalk-rpi5-sim)
            printf '%s\n' "$1"
            ;;
        *)
            echo "Component is not in the fixed allowlist: $1" >&2
            return 2
            ;;
    esac
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
    local operation="$1" repository="$3" status="$9"
    printf '[%s] %s: %s\n' "$status" "$repository" "$operation"
}

xwalk_changelog()
{
    local module="$1" operation="$2" source_change="$3" source_commit="$4"
    local integrated_change="$5" integrated_commit="$6" result="$7"
    local explanation="$8" link="$9" changelog
    changelog="${XWALK_UPLIFT_CHANGELOG:-${XWALK_UPLIFT_STATE_DIR:-$HOME/.local/state/xwalk-gerrit/uplift}/changelog.jsonl}"
    mkdir -p "$(dirname -- "$changelog")"
    python3 - "$changelog" "$module" "$operation" "$source_change" "$source_commit" \
        "$integrated_change" "$integrated_commit" "$result" "$explanation" "$link" <<'PY'
import datetime
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
entry = {
    "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat().replace("+00:00", "Z"),
    "module": sys.argv[2],
    "operation": sys.argv[3],
    "source_change": sys.argv[4],
    "source_commit": sys.argv[5],
    "integrated_change": sys.argv[6],
    "integrated_commit": sys.argv[7],
    "result": sys.argv[8],
    "explanation": sys.argv[9],
    "link": sys.argv[10],
}
with path.open("a", encoding="utf-8") as stream:
    stream.write(json.dumps(entry, sort_keys=True) + "\n")
PY
    chmod 600 "$changelog"
}

xwalk_retry()
{
    local attempt status=1
    for ((attempt = 1; attempt <= GERRIT_UPLIFT_RETRY_ATTEMPTS; ++attempt)); do
        if "$@"; then
            return 0
        else
            status=$?
        fi
        ((attempt < GERRIT_UPLIFT_RETRY_ATTEMPTS)) || break
        printf 'Temporary operation failed; retrying (%d/%d)\n' \
            "$attempt" "$GERRIT_UPLIFT_RETRY_ATTEMPTS" >&2
        sleep "$GERRIT_UPLIFT_RETRY_DELAY_SECONDS"
    done
    return "$status"
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
