#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/py-agent/gerrit-tool/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

usage()
{
    echo "Usage: gerrit-auto-uplift.sh --dry-run|--apply REPOSITORY COMMIT CHANGE [TOPIC]" >&2
    exit 2
}
[[ "$#" -ge 4 && "$#" -le 5 ]] || usage
xwalk_mode "$1"
repository="$2"
submitted_commit="$3"
source_change="$4"
topic="${5:-}"
xwalk_load_config
xwalk_repository "$repository"
[[ "$repository" != "xWalk-rpi5" ]] || { echo "Cannot uplift xWalk-rpi5 into itself" >&2; exit 2; }
component_path="$(xwalk_component_path "$repository")"
[[ "$submitted_commit" =~ ^[0-9a-fA-F]{40}$ ]] || { echo "Submitted commit must be a full ID" >&2; exit 2; }
[[ -z "$topic" || "$topic" =~ ^[A-Za-z0-9._-]{1,80}$ ]] || { echo "Unsafe Gerrit topic" >&2; exit 2; }

plan()
{
    xwalk_log "uplift-submodule" "uplift" "$repository" "xWalk-rpi5/$component_path" "recorded-commit" \
        "$submitted_commit" "Plan automatic component uplift" \
        "Submitted module change $source_change requires an integration review." "Planned" \
        "Full commit ID, repository allowlist and topic validated" "" "$source_change" "" "$submitted_commit"
    xwalk_log "integration-build" "CI" "xWalk-rpi5" "build-host/integration" "not-run" "planned" \
        "Plan complete integration build" "Every uplift must preserve the verified baseline." "Planned"
    xwalk_log "integration-test" "CI" "xWalk-rpi5" "ctest" "not-run" "planned" \
        "Plan complete integration tests" "A submodule uplift may affect cross-repository consumers." "Planned"
    xwalk_log "upload-uplift-review" "uplift" "xWalk-rpi5" "refs/for/main" "not-uploaded" "planned" \
        "Plan Gerrit uplift review" "Automatic uplifts remain reviewed unless explicit submission is enabled." \
        "Planned" "No direct main push is planned" "" "$source_change"
}

apply()
{
    local state lock work old_commit changed ref review_output baseline latest_main
    state="${XWALK_UPLIFT_STATE_DIR:-$HOME/.local/state/xwalk-gerrit/uplift}"
    mkdir -p "$state"
    chmod 700 "$state"
    lock="$state/uplift.lock"
    exec 9>"$lock"
    flock -x 9
    work="$(mktemp -d -t xwalk-uplift-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    git -c \
        "url.ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/.insteadOf=${GERRIT_BASE_URL%/}/" \
        clone --quiet --recurse-submodules \
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/xWalk-rpi5" "$work/xWalk-rpi5"
    old_commit="$(git -C "$work/xWalk-rpi5/$component_path" rev-parse HEAD)"
    git -C "$work/xWalk-rpi5/$component_path" fetch --quiet origin main
    git -C "$work/xWalk-rpi5/$component_path" merge-base --is-ancestor "$submitted_commit" origin/main || {
        xwalk_log "uplift-submodule" "uplift" "$repository" "xWalk-rpi5/$component_path" "$old_commit" \
            "unchanged" "Rejected unreachable uplift commit" \
            "Only commits reachable from the module Gerrit main are eligible." "Failed" \
            "git merge-base --is-ancestor failed" "Submitted commit is not on origin/main" "$source_change"
        return 1
    }
    git -C "$work/xWalk-rpi5/$component_path" checkout --quiet --detach "$submitted_commit"
    git -C "$work/xWalk-rpi5" add "$component_path"
    changed="$(git -C "$work/xWalk-rpi5" diff --cached --name-only)"
    [[ "$changed" == "$component_path" ]] || {
        echo "Uplift changed more than the selected submodule pointer" >&2
        return 1
    }
    git -C "$work/xWalk-rpi5" -c user.name=xWalk-CI -c user.email=ci.invalid commit -s \
        -m "Uplift $repository to $submitted_commit" \
        -m "Component: $repository" -m "Previous-Commit: $old_commit" \
        -m "New-Commit: $submitted_commit" -m "Gerrit-Change: $source_change" \
        -m "Submitted-Revision: $submitted_commit" -m "Gerrit-Topic: ${topic:-none}"
    xwalk_log "uplift-submodule" "uplift" "$repository" "xWalk-rpi5/$component_path" "$old_commit" \
        "$submitted_commit" "Updated exact submodule pointer" \
        "Submitted module change $source_change passed module verification." "Applied" \
        "Only the selected gitlink changed" "" "$source_change" "$old_commit" "$submitted_commit"
    if ! cmake --fresh -S "$work/xWalk-rpi5" -B "$work/xWalk-rpi5/build-host/integration" -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug -DXWALK_ENABLE_STRICT_WARNINGS=ON || \
        ! cmake --build "$work/xWalk-rpi5/build-host/integration" --parallel; then
        xwalk_log "integration-build" "CI" "xWalk-rpi5" "build-host/integration" "not-run" "failed" \
            "Integration build failed" "The last verified baseline must be preserved." "Failed" \
            "" "CMake configure or build failed" "$source_change"
        return 1
    fi
    xwalk_log "integration-build" "CI" "xWalk-rpi5" "build-host/integration" "not-run" "passed" \
        "Completed integration build" "The uplift must compile with all recorded components." "Verified" \
        "CMake configure and build succeeded" "" "$source_change"
    if ! ctest --test-dir "$work/xWalk-rpi5/build-host/integration" --output-on-failure --no-tests=error || \
        ! "$work/xWalk-rpi5/build-host/integration/xWalkController/xWalkApp/xwalk-picarx-control" \
            --deployment-config="$work/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf" \
            --diagnose --no-hardware; then
        xwalk_log "integration-test" "CI" "xWalk-rpi5" "ctest and diagnostic" "not-run" "failed" \
            "Integration validation failed" "The last verified baseline must be preserved." "Failed" \
            "" "CTest or host-safe diagnostic failed" "$source_change"
        return 1
    fi
    xwalk_log "integration-test" "CI" "xWalk-rpi5" "ctest and diagnostic" "not-run" "passed" \
        "Completed host-safe integration validation" \
        "Cross-repository behavior must pass before uplift review upload." "Verified" \
        "CTest and --no-hardware diagnostic succeeded" "" "$source_change"
    git -C "$work/xWalk-rpi5" fetch --quiet origin main
    baseline="$(git -C "$work/xWalk-rpi5" rev-parse HEAD^)"
    latest_main="$(git -C "$work/xWalk-rpi5" rev-parse origin/main)"
    if [[ "$baseline" != "$latest_main" ]]; then
        xwalk_log "retry-concurrent-uplift" "uplift" "xWalk-rpi5" "refs/heads/main" \
            "$baseline" "$latest_main" "Retry after concurrent integration update" \
            "A newer uplift changed main while validation was running." "Skipped" \
            "Fresh clone and complete revalidation required" "" "$source_change"
        rm -rf -- "$work"
        trap - EXIT
        return 75
    fi
    ref="refs/for/main%topic=component-uplift"
    if review_output="$(git -C "$work/xWalk-rpi5" push origin "HEAD:$ref" 2>&1)"; then
        xwalk_log "upload-uplift-review" "uplift" "xWalk-rpi5" "$ref" "not-uploaded" \
            "uploaded" "Uploaded automatic uplift for Gerrit review" \
            "Integration validation passed without direct main submission." "Applied" \
            "Gerrit accepted refs/for/main" "" "$source_change" "$old_commit" "$submitted_commit"
    else
        xwalk_log "upload-uplift-review" "uplift" "xWalk-rpi5" "$ref" "not-uploaded" \
            "unchanged" "Uplift review upload failed" "Gerrit rejected the review upload." "Failed" \
            "" "Sanitized Gerrit push failure" "$source_change" "$old_commit" "$submitted_commit"
        return 1
    fi
    local review_number
    review_number="$(grep -oE '/\+/[0-9]+' <<< "$review_output" | tail -1 | grep -oE '[0-9]+' || true)"
    if [[ "$GERRIT_UPLIFT_AUTO_SUBMIT" == "true" ]]; then
        [[ -n "$review_number" ]] || { echo "Cannot identify uplift review for submission" >&2; return 1; }
        if ssh -o BatchMode=yes -p "$GERRIT_SSH_PORT" \
            "$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST" gerrit review --submit "$review_number"; then
            xwalk_log "submit-uplift" "uplift" "xWalk-rpi5" "$review_number" "open" "submitted" \
                "Automatically submitted uplift" \
                "Explicit automatic-submit policy allowed submission after verification." "Verified" \
                "Gerrit accepted submit" "" "$review_number"
        else
            xwalk_log "submit-uplift" "uplift" "xWalk-rpi5" "$review_number" "open" "open" \
                "Automatic uplift submission failed" "Gerrit requirements were not satisfied." "Failed" \
                "" "Sanitized Gerrit submit failure" "$review_number"
            return 1
        fi
    else
        xwalk_log "submit-uplift" "uplift" "xWalk-rpi5" "${review_number:-unknown}" "open" "open" \
            "Skipped automatic uplift submission" "Automatic submit is disabled by policy." "Skipped" \
            "Review remains open for normal approval"
    fi
}

if [[ "$XWALK_MODE" == "dry-run" ]]; then
    plan
else
    status=1
    for attempt in 1 2 3; do
        if apply; then
            exit 0
        else
            status=$?
        fi
        [[ "$status" -eq 75 ]] || exit "$status"
        printf 'Concurrent uplift detected; retrying with a fresh baseline (%d/3)\n' "$attempt" >&2
    done
    xwalk_log "retry-concurrent-uplift" "uplift" "xWalk-rpi5" "refs/heads/main" \
        "changed-repeatedly" "unchanged" "Concurrent uplift retries exhausted" \
        "Three newer integration baselines prevented a safe review upload." "Failed" \
        "Last verified baseline remains unchanged" "Concurrent uplift contention" "$source_change"
    exit 1
fi
