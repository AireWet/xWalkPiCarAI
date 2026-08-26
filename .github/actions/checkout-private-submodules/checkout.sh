#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

root="$(git rev-parse --show-toplevel)"
key_root="${RUNNER_TEMP:?}/xwalk-submodule-read-keys"
bundle_file="${AIREWET_SUBMODULE_READ_KEYS_FILE:-}"

cleanup()
{
    rm -rf -- "$key_root"
}
trap cleanup EXIT

if [[ -z "$bundle_file" ]]; then
    [[ -n "${AIREWET_SUBMODULE_READ_KEYS:-}" ]] || {
        echo "Missing AIREWET_SUBMODULE_READ_KEYS" >&2
        exit 2
    }
elif [[ ! -r "$bundle_file" ]]; then
    echo "Cannot read the configured submodule key bundle" >&2
    exit 2
fi

mkdir -p "$key_root"
chmod 700 "$key_root"
printf '%s\n' \
    'github.com ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl' \
    > "$key_root/known_hosts"

read_bundle_key()
{
    local component="$1"
    if [[ -n "$bundle_file" ]]; then
        jq --exit-status --raw-output --arg component "$component" '.[$component]' "$bundle_file"
    else
        jq --exit-status --raw-output --arg component "$component" \
            '.[$component]' <<< "$AIREWET_SUBMODULE_READ_KEYS"
    fi
}

while IFS=$'\t' read -r component repository alias key_name; do
    key_path="$key_root/$key_name"
    read_bundle_key "$component" > "$key_path"
    chmod 600 "$key_path"
    printf '%s\n' \
        "Host $alias" \
        '    HostName github.com' \
        '    HostKeyAlias github.com' \
        '    User git' \
        "    IdentityFile $key_path" \
        '    IdentitiesOnly yes' \
        '    BatchMode yes' \
        '    StrictHostKeyChecking yes' \
        "    UserKnownHostsFile $key_root/known_hosts" \
        >> "$key_root/config"
    git -C "$root" config --local "submodule.$component.url" \
        "git@$alias:AireWet/$repository.git"
    review_refs="$(GIT_SSH_COMMAND="ssh -F $key_root/config" git ls-remote \
        "git@$alias:AireWet/$repository.git" \
        'refs/changes/*' 'refs/drafts/*' 'refs/cache-automerge/*' 'refs/meta/*' \
        'refs/users/*' 'refs/edit/*' 'refs/starred-changes/*')"
    [[ -z "$review_refs" ]] || {
        echo "Gerrit review refs are exposed by AireWet/$repository" >&2
        exit 1
    }
done <<'MAPPINGS'
DevloperNote	xWalkDeveloperNote	github-xwalk-developer-note	devloper-note
xWalkAgent	xWalkAgent	github-xwalk-agent	xwalk-agent
xWalkAudioResources	xWalkAudioResources	github-xwalk-audio-resources	xwalk-audio-resources
xWalkController	xWalkController	github-xwalk-controller	xwalk-controller
xWalkHal	xWalkHal	github-xwalk-hal	xwalk-hal
xWalkLibrary	xWalkLibrary	github-xwalk-library	xwalk-library
xWalk-rpi5-trace	xWalk-rpi5-trace	github-xwalk-trace	xwalk-trace
xWalk-rpi5-iw	xWalk-rpi5-iw	github-xwalk-rpi5-iw	xwalk-rpi5-iw
xWalk-rpi5-tool	xWalk-rpi5-tool	github-xwalk-rpi5-tool	xwalk-rpi5-tool
MAPPINGS

GIT_SSH_COMMAND="ssh -F $key_root/config" git -C "$root" submodule update --init --recursive
git -C "$root" submodule foreach --recursive 'git rev-parse --verify HEAD >/dev/null'
echo "Checked out exact submitted commits and verified that Gerrit review refs are excluded"
