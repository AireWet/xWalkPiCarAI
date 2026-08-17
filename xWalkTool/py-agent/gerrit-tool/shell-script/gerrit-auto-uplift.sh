#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

usage()
{
    echo "Usage: gerrit-auto-uplift.sh --dry-run|--apply MODULE COMMIT CHANGE PATCHSET" >&2
    exit 2
}

[[ "$#" -eq 6 ]] || usage
xwalk_mode "$1"
module="$2"
source_commit="$3"
source_change="$4"
source_patchset="$5"
source_topic="$6"
xwalk_load_config
xwalk_repository "$module"
component_path="$(xwalk_component_path "$module")"
[[ "$source_commit" =~ ^[0-9a-fA-F]{40}$ ]] || { echo "Source commit must be a full ID" >&2; exit 2; }
[[ "$source_change" =~ ^[1-9][0-9]*$ ]] || { echo "Source change must be numeric" >&2; exit 2; }
[[ "$source_patchset" =~ ^[1-9][0-9]*$ ]] || { echo "Source patchset must be numeric" >&2; exit 2; }
[[ -z "$source_topic" || "$source_topic" =~ ^[A-Za-z0-9._-]{1,80}$ ]] || {
    echo "Unsafe source Gerrit topic" >&2
    exit 2
}

integration_remote="ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$GERRIT_INTEGRATION_PROJECT"
module_remote="ssh://$GERRIT_CI_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$module"
topic="uplift-${module}-${source_change}"
change_id="I$(printf '%s\n' "$GERRIT_INTEGRATION_PROJECT:$GERRIT_INTEGRATION_BRANCH:$module:$source_change" | sha1sum | awk '{print $1}')"
source_reference="${source_change},${source_patchset}"
integration_web_url="${GERRIT_WEB_URL:-${GERRIT_HTTP_BASE_URL:-https://$GERRIT_SERVER_HOST}}"
integration_link="${integration_web_url%/}/q/change:${change_id}"

plan()
{
    printf 'Source module: %s\n' "$module"
    printf 'Source Gerrit change: %s\n' "$source_reference"
    printf 'Integrated repository: %s/%s\n' "$GERRIT_INTEGRATION_PROJECT" "$GERRIT_INTEGRATION_BRANCH"
    printf 'Integrated topic: %s\n' "$topic"
    printf 'Integrated Change-Id: %s\n' "$change_id"
    xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
        "$change_id" "not-created" "skipped" \
        "Dry-run planned an integrated-source uplift review" "$integration_link"
}

apply()
{
    local state marker work integration source target target_relative changed baseline latest commit
    local message
    local push_output review_number
    local push_succeeded=false push_attempt
    state="${XWALK_UPLIFT_STATE_DIR:-$HOME/.local/state/xwalk-gerrit/uplift}"
    mkdir -p "$state/events"
    chmod 700 "$state" "$state/events"
    marker="$state/events/${module}-${source_change}-${source_patchset}-${source_commit}.done"
    exec 9>"$state/uplift.lock"
    flock -x 9

    if [[ -f "$marker" ]]; then
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "already-uploaded" "skipped" \
            "Duplicate merged-change event was already processed" "$integration_link"
        echo "Uplift already processed for $module change $source_reference"
        echo "Uplift status: ALREADY_PROCESSED"
        return 0
    fi

    work="$(mktemp -d -t xwalk-uplift-XXXXXXXX)"
    trap 'rm -rf -- "$work"' RETURN
    integration="$work/integration"
    source="$work/source"

    if ! xwalk_retry git clone --quiet --branch "$GERRIT_INTEGRATION_BRANCH" \
        "$integration_remote" "$integration"; then
        xwalk_changelog "$module" "fetch" "$source_reference" "$source_commit" \
            "$change_id" "unavailable" "failed" \
            "Could not fetch the integrated Gerrit branch after retries" "$integration_link"
        return 1
    fi
    if ! xwalk_retry git clone --quiet --no-checkout "$module_remote" "$source" || \
        ! xwalk_retry git -C "$source" fetch --quiet origin master; then
        xwalk_changelog "$module" "fetch" "$source_reference" "$source_commit" \
            "$change_id" "unavailable" "failed" \
            "Could not fetch the source module after retries" "$integration_link"
        return 1
    fi
    if ! git -C "$source" merge-base --is-ancestor "$source_commit" origin/master; then
        xwalk_changelog "$module" "fetch" "$source_reference" "$source_commit" \
            "$change_id" "unresolved" "failed" \
            "Source commit is not reachable from the module master branch" "$integration_link"
        return 1
    fi
    xwalk_changelog "$module" "fetch" "$source_reference" "$source_commit" \
        "$change_id" "$source_commit" "success" \
        "Fetched and resolved the exact merged source commit" "$integration_link"

    if [[ "$module" == xWalkTool ]]; then
        target_relative=xWalkTool
    else
        target_relative="$GERRIT_INTEGRATION_SOURCE_ROOT/$component_path"
    fi
    target="$integration/$target_relative"
    [[ -d "$target" && ! -L "$target" ]] || {
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "missing-target" "failed" \
            "Integrated module directory is missing or symbolic" "$integration_link"
        return 1
    }
    git -C "$integration" rm --quiet -r -- "$target_relative"
    mkdir -p "$target"
    git -C "$source" archive "$source_commit" | tar -x -C "$target"
    git -C "$integration" add -- "$target_relative"
    changed="$(git -C "$integration" diff --cached --name-only)"
    if [[ -z "$changed" ]]; then
        : > "$marker"
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "already-integrated" "skipped" \
            "The exact source content is already present in the integrated branch" "$integration_link"
        echo "Uplift status: ALREADY_INTEGRATED"
        return 0
    fi
    if grep -Ev "^${target_relative}(/|$)" <<< "$changed" | grep -q .; then
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "invalid-diff" "failed" \
            "Uplift modified content outside the selected module" "$integration_link"
        return 1
    fi

    message="$work/uplift-message.txt"
    git -C "$source" show -s --format=%B "$source_commit" > "$message"
    git interpret-trailers --in-place --if-exists replace --if-missing add \
        --trailer "Source-Repository: $module" \
        --trailer "Source-Commit: $source_commit" \
        --trailer "Source-Change: $source_change" \
        --trailer "Source-Patchset: $source_patchset" \
        --trailer "Source-Topic: ${source_topic:-none}" \
        --trailer "Change-Id: $change_id" \
        "$message"
    git -C "$integration" -c user.name=xWalk-CI -c user.email="$GERRIT_CI_EMAIL" \
        commit -s -F "$message"
    commit="$(git -C "$integration" rev-parse HEAD)"

    git -C "$integration" diff --check HEAD^
    if [[ -x "$integration/xWalkTool/shell-agent/gerrit-tool/validate-integration-metadata.sh" ]]; then
        git -C "$integration" submodule update --init --recursive
        (
            cd "$integration"
            xWalkTool/shell-agent/gerrit-tool/validate-integration-metadata.sh
        )
    fi

    xwalk_retry git -C "$integration" fetch --quiet origin "$GERRIT_INTEGRATION_BRANCH"
    baseline="$(git -C "$integration" rev-parse HEAD^)"
    latest="$(git -C "$integration" rev-parse "origin/$GERRIT_INTEGRATION_BRANCH")"
    if [[ "$baseline" != "$latest" ]]; then
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "$commit" "retrying" \
            "Integrated branch changed while the uplift was prepared" "$integration_link"
        return 75
    fi

    for ((push_attempt = 1; push_attempt <= GERRIT_UPLIFT_RETRY_ATTEMPTS; ++push_attempt)); do
        if push_output="$(git -C "$integration" push origin \
            "HEAD:refs/for/$GERRIT_INTEGRATION_BRANCH%topic=$topic" 2>&1)"; then
            push_succeeded=true
            break
        fi
        grep -qi 'no new changes' <<< "$push_output" && break
        ((push_attempt < GERRIT_UPLIFT_RETRY_ATTEMPTS)) || break
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "$commit" "retrying" \
            "Gerrit review upload failed temporarily" "$integration_link"
        sleep "$GERRIT_UPLIFT_RETRY_DELAY_SECONDS"
    done
    if [[ "$push_succeeded" == true ]]; then
        review_number="$(grep -oE '/\+/[0-9]+' <<< "$push_output" | tail -1 | grep -oE '[0-9]+' || true)"
        if [[ -n "$review_number" ]]; then
            integration_link="${integration_web_url%/}/c/${GERRIT_INTEGRATION_PROJECT}/+/${review_number}"
        fi
        : > "$marker"
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "${review_number:-$change_id}" "$commit" "success" \
            "Uploaded an active integrated Gerrit change; complete CI will start from patchset-created" \
            "$integration_link"
        printf 'Integrated Gerrit change: %s\n' "${review_number:-$change_id}"
        printf 'Integrated Gerrit URL: %s\n' "$integration_link"
        printf 'Uplift status: REVIEW_CREATED\n'
        printf 'CI status: triggered by active patch-set upload\n'
        return 0
    fi
    if grep -qi 'no new changes' <<< "$push_output"; then
        : > "$marker"
        xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
            "$change_id" "$commit" "skipped" \
            "Gerrit already contains the same uplift patch set" "$integration_link"
        echo "Uplift status: REVIEW_EXISTS"
        echo "Integrated Gerrit URL: $integration_link"
        return 0
    fi
    push_reason="$(grep -Ei 'invalid (author|committer)|not registered|prohibited by Gerrit|permission denied|not permitted|missing Change-Id|change .* closed' \
        <<< "$push_output" | tail -1 | sed -E 's/^[[:space:]]*(remote:[[:space:]]*)?//' || true)"
    [[ -n "$push_reason" ]] || push_reason="Gerrit rejected the uplift review upload"
    xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
        "$change_id" "$commit" "failed" \
        "$push_reason" "$integration_link"
    echo "Uplift status: FAILED"
    echo "Uplift reason: $push_reason"
    echo "Integrated Gerrit URL: $integration_link"
    return 1
}

if [[ "$XWALK_MODE" == dry-run ]]; then
    plan
    exit 0
fi

status=1
for ((attempt = 1; attempt <= GERRIT_UPLIFT_RETRY_ATTEMPTS; ++attempt)); do
    if apply; then
        exit 0
    else
        status=$?
    fi
    [[ "$status" -eq 75 ]] || exit "$status"
    ((attempt < GERRIT_UPLIFT_RETRY_ATTEMPTS)) || break
done
xwalk_changelog "$module" "uplift" "$source_reference" "$source_commit" \
    "$change_id" "not-uploaded" "failed" \
    "Concurrent integrated-branch updates exhausted the retry limit" "$integration_link"
exit 1
