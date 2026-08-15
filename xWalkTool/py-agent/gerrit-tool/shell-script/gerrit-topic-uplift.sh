#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

usage()
{
    echo "Usage: gerrit-topic-uplift.sh --dry-run|--apply TOPIC REPOSITORY COMMIT CHANGE [...]" >&2
    exit 2
}
[[ "$#" -ge 5 && $(( ($# - 2) % 3 )) -eq 0 ]] || usage
xwalk_mode "$1"
topic="$2"
shift 2
xwalk_load_config
[[ "$topic" =~ ^[A-Za-z0-9._-]{1,80}$ ]] || { echo "Unsafe Gerrit topic" >&2; exit 2; }

repositories=()
commits=()
changes=()
declare -A selected=()
while [[ "$#" -gt 0 ]]; do
    repository="$1"
    commit="$2"
    change="$3"
    shift 3
    xwalk_repository "$repository"
    [[ "$repository" != xWalk-rpi5 ]] || { echo "Cannot uplift xWalk-rpi5 into itself" >&2; exit 2; }
    [[ "$commit" =~ ^[0-9a-fA-F]{40}$ ]] || { echo "Submitted commit must be a full ID" >&2; exit 2; }
    [[ -z "${selected[$repository]:-}" ]] || { echo "Duplicate topic repository: $repository" >&2; exit 2; }
    selected["$repository"]=1
    repositories+=("$repository")
    commits+=("$commit")
    changes+=("$change")
done

plan()
{
    local index component_path
    for index in "${!repositories[@]}"; do
        component_path="$(xwalk_component_path "${repositories[$index]}")"
        xwalk_log "uplift-topic-submodule" "uplift" "${repositories[$index]}" \
            "xWalk-rpi5/$component_path" "recorded-commit" "${commits[$index]}" \
            "Plan coordinated topic uplift" \
            "Related change ${changes[$index]} belongs to Gerrit topic $topic." "Planned" \
            "Repository allowlist and full commit validated" "" "${changes[$index]}" "" \
            "${commits[$index]}"
    done
    xwalk_log "integration-topic-validation" "CI" "xWalk-rpi5" "build-host/integration" \
        "not-run" "planned" "Plan one coordinated integration validation" \
        "All changes in Gerrit topic $topic must be tested and reviewed together." "Planned"
}

apply()
{
    local state work index repository component_path commit change old_commit changed message
    state="${XWALK_UPLIFT_STATE_DIR:-$HOME/.local/state/xwalk-gerrit/uplift}"
    mkdir -p "$state"
    chmod 700 "$state"
    exec 9>"$state/uplift.lock"
    flock -x 9
    work="$(mktemp -d -t xwalk-topic-uplift-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    git -c \
        "url.ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/.insteadOf=${GERRIT_BASE_URL%/}/" \
        clone --quiet --recurse-submodules \
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/xWalk-rpi5" "$work/xWalk-rpi5"
    message="Gerrit-Topic: $topic"
    for index in "${!repositories[@]}"; do
        repository="${repositories[$index]}"
        component_path="$(xwalk_component_path "$repository")"
        commit="${commits[$index]}"
        change="${changes[$index]}"
        old_commit="$(git -C "$work/xWalk-rpi5/$component_path" rev-parse HEAD)"
        git -C "$work/xWalk-rpi5/$component_path" fetch --quiet origin master
        git -C "$work/xWalk-rpi5/$component_path" merge-base --is-ancestor "$commit" origin/master || {
            echo "$repository commit is not reachable from origin/master" >&2
            return 1
        }
        git -C "$work/xWalk-rpi5/$component_path" checkout --quiet --detach "$commit"
        git -C "$work/xWalk-rpi5" add "$component_path"
        message+=$'\n\n'"Component: $repository"$'\n'"Previous-Commit: $old_commit"$'\n'\
"New-Commit: $commit"$'\n'"Gerrit-Change: $change"$'\n'"Submitted-Revision: $commit"
        xwalk_log "uplift-topic-submodule" "uplift" "$repository" "xWalk-rpi5/$component_path" \
            "$old_commit" "$commit" "Updated coordinated topic gitlink" \
            "Related change $change belongs to Gerrit topic $topic." "Applied" \
            "Submitted commit is reachable from component master" "" "$change" "$old_commit" "$commit"
    done
    changed="$(git -C "$work/xWalk-rpi5" diff --cached --name-only | sort)"
    [[ "$(wc -l <<< "$changed")" -eq "${#repositories[@]}" ]] || {
        echo "Coordinated uplift changed an unexpected path" >&2
        return 1
    }
    git -C "$work/xWalk-rpi5" -c user.name=xWalk-CI -c user.email=ci.invalid commit -s \
        -m "Uplift Gerrit topic $topic" -m "$message"
    cmake --fresh -S "$work/xWalk-rpi5" -B "$work/xWalk-rpi5/build-host/integration" -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug -DXWALK_ENABLE_STRICT_WARNINGS=ON
    cmake --build "$work/xWalk-rpi5/build-host/integration" --parallel
    ctest --test-dir "$work/xWalk-rpi5/build-host/integration" --output-on-failure --no-tests=error
    "$work/xWalk-rpi5/build-host/integration/xWalkController/xWalkApp/xwalk-picarx-control" \
        --deployment-config="$work/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf" \
        --diagnose --no-hardware
    xwalk_log "integration-topic-validation" "CI" "xWalk-rpi5" "build-host/integration" \
        "not-run" "passed" "Validated coordinated topic uplift" \
        "All changes in Gerrit topic $topic were tested together." "Verified" \
        "Configure, build, CTest and host-safe diagnostic passed"
    git -C "$work/xWalk-rpi5" push origin HEAD:refs/for/master%topic=component-uplift
    xwalk_log "upload-topic-uplift-review" "uplift" "xWalk-rpi5" \
        "refs/for/master%topic=component-uplift" "not-uploaded" "uploaded" \
        "Uploaded coordinated topic uplift" \
        "One integration review records every related component gitlink." "Applied" \
        "Gerrit accepted the exact review refspec"
}

if [[ "$XWALK_MODE" == dry-run ]]; then plan; else apply; fi
