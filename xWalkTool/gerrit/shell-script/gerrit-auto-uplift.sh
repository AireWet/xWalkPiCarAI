#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/gerrit/shell-script/xwalk-gerrit-common.sh
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
[[ "$repository" != "MyPiCarX" ]] || { echo "Cannot uplift MyPiCarX into itself" >&2; exit 2; }
[[ "$submitted_commit" =~ ^[0-9a-fA-F]{40}$ ]] || { echo "Submitted commit must be a full ID" >&2; exit 2; }
[[ -z "$topic" || "$topic" =~ ^[A-Za-z0-9._-]{1,80}$ ]] || { echo "Unsafe Gerrit topic" >&2; exit 2; }

plan()
{
    xwalk_log "uplift-submodule" "uplift" "$repository" "MyPiCarX/$repository" "recorded-commit" \
        "$submitted_commit" "Plan automatic component uplift" \
        "Submitted module change $source_change requires an integration review." "Planned" \
        "Full commit ID, repository allowlist and topic validated" "" "$source_change" "" "$submitted_commit"
    xwalk_log "integration-build" "CI" "MyPiCarX" "build-host/integration" "not-run" "planned" \
        "Plan complete integration build" "Every uplift must preserve the verified baseline." "Planned"
    xwalk_log "integration-test" "CI" "MyPiCarX" "ctest" "not-run" "planned" \
        "Plan complete integration tests" "A submodule uplift may affect cross-repository consumers." "Planned"
    xwalk_log "upload-uplift-review" "uplift" "MyPiCarX" "refs/for/main" "not-uploaded" "planned" \
        "Plan Gerrit uplift review" "Automatic uplifts remain reviewed unless explicit submission is enabled." \
        "Planned" "No direct main push is planned" "" "$source_change"
}

apply()
{
    local state lock work old_commit changed ref review_output
    state="${XWALK_UPLIFT_STATE_DIR:-$HOME/.local/state/xwalk-gerrit/uplift}"
    mkdir -p "$state"
    chmod 700 "$state"
    lock="$state/uplift.lock"
    exec 9>"$lock"
    flock -x 9
    work="$(mktemp -d -t xwalk-uplift-XXXXXXXX)"
    trap 'rm -rf -- "$work"' EXIT
    git clone --quiet --recurse-submodules \
        "ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/MyPiCarX" "$work/MyPiCarX"
    old_commit="$(git -C "$work/MyPiCarX/$repository" rev-parse HEAD)"
    git -C "$work/MyPiCarX/$repository" fetch --quiet origin main
    git -C "$work/MyPiCarX/$repository" merge-base --is-ancestor "$submitted_commit" origin/main || {
        xwalk_log "uplift-submodule" "uplift" "$repository" "MyPiCarX/$repository" "$old_commit" \
            "unchanged" "Rejected unreachable uplift commit" \
            "Only commits reachable from the module Gerrit main are eligible." "Failed" \
            "git merge-base --is-ancestor failed" "Submitted commit is not on origin/main" "$source_change"
        return 1
    }
    git -C "$work/MyPiCarX/$repository" checkout --quiet --detach "$submitted_commit"
    git -C "$work/MyPiCarX" add "$repository"
    changed="$(git -C "$work/MyPiCarX" diff --cached --name-only)"
    [[ "$changed" == "$repository" ]] || {
        echo "Uplift changed more than the selected submodule pointer" >&2
        return 1
    }
    git -C "$work/MyPiCarX" -c user.name=xWalk-CI -c user.email=ci.invalid commit -s \
        -m "Uplift $repository to $submitted_commit" \
        -m "Source-Change: $source_change" -m "Topic: ${topic:-none}"
    xwalk_log "uplift-submodule" "uplift" "$repository" "MyPiCarX/$repository" "$old_commit" \
        "$submitted_commit" "Updated exact submodule pointer" \
        "Submitted module change $source_change passed module verification." "Applied" \
        "Only the selected gitlink changed" "" "$source_change" "$old_commit" "$submitted_commit"
    if ! cmake --fresh -S "$work/MyPiCarX" -B "$work/MyPiCarX/build-host/integration" -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug -DXWALK_ENABLE_STRICT_WARNINGS=ON || \
        ! cmake --build "$work/MyPiCarX/build-host/integration" --parallel; then
        xwalk_log "integration-build" "CI" "MyPiCarX" "build-host/integration" "not-run" "failed" \
            "Integration build failed" "The last verified baseline must be preserved." "Failed" \
            "" "CMake configure or build failed" "$source_change"
        return 1
    fi
    xwalk_log "integration-build" "CI" "MyPiCarX" "build-host/integration" "not-run" "passed" \
        "Completed integration build" "The uplift must compile with all recorded components." "Verified" \
        "CMake configure and build succeeded" "" "$source_change"
    if ! ctest --test-dir "$work/MyPiCarX/build-host/integration" --output-on-failure --no-tests=error || \
        ! "$work/MyPiCarX/build-host/integration/xWalkController/xWalkApp/xwalk-picarx-control" \
            --deployment-config="$work/MyPiCarX/xWalkController/xWalkConfig/picar-x.conf" \
            --diagnose --no-hardware; then
        xwalk_log "integration-test" "CI" "MyPiCarX" "ctest and diagnostic" "not-run" "failed" \
            "Integration validation failed" "The last verified baseline must be preserved." "Failed" \
            "" "CTest or host-safe diagnostic failed" "$source_change"
        return 1
    fi
    xwalk_log "integration-test" "CI" "MyPiCarX" "ctest and diagnostic" "not-run" "passed" \
        "Completed host-safe integration validation" \
        "Cross-repository behavior must pass before uplift review upload." "Verified" \
        "CTest and --no-hardware diagnostic succeeded" "" "$source_change"
    ref="refs/for/main"
    [[ -z "$topic" ]] || ref="$ref%topic=$topic"
    if review_output="$(git -C "$work/MyPiCarX" push origin "HEAD:$ref" 2>&1)"; then
        xwalk_log "upload-uplift-review" "uplift" "MyPiCarX" "$ref" "not-uploaded" \
            "uploaded" "Uploaded automatic uplift for Gerrit review" \
            "Integration validation passed without direct main submission." "Applied" \
            "Gerrit accepted refs/for/main" "" "$source_change" "$old_commit" "$submitted_commit"
    else
        xwalk_log "upload-uplift-review" "uplift" "MyPiCarX" "$ref" "not-uploaded" \
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
            xwalk_log "submit-uplift" "uplift" "MyPiCarX" "$review_number" "open" "submitted" \
                "Automatically submitted uplift" \
                "Explicit automatic-submit policy allowed submission after verification." "Verified" \
                "Gerrit accepted submit" "" "$review_number"
        else
            xwalk_log "submit-uplift" "uplift" "MyPiCarX" "$review_number" "open" "open" \
                "Automatic uplift submission failed" "Gerrit requirements were not satisfied." "Failed" \
                "" "Sanitized Gerrit submit failure" "$review_number"
            return 1
        fi
    else
        xwalk_log "submit-uplift" "uplift" "MyPiCarX" "${review_number:-unknown}" "open" "open" \
            "Skipped automatic uplift submission" "Automatic submit is disabled by policy." "Skipped" \
            "Review remains open for normal approval"
    fi
}

if [[ "$XWALK_MODE" == "dry-run" ]]; then plan; else apply; fi
