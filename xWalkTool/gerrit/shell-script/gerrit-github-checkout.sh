#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

root="$(git rev-parse --show-toplevel)"
script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
logger="$script_dir/../py-src/xWalkGerritChangeLog.py"

log_checkout()
{
    python3 "$logger" --operation "github-submodule-checkout" --category "CI" \
        --repository "xWalk-rpi5" --target ".gitmodules" --previous-value "$1" \
        --new-value "$2" --change-summary "$3" --change-explanation \
        "GitHub Actions may initialize only exact recorded Gerrit component revisions." \
        --requested-by "CI" --executed-by "GitHub-Actions" --mode "apply" \
        --status "$4" --verification "${5:-}" --error-summary "${6:-}"
}

[[ -f "$root/.gitmodules" ]] || {
    log_checkout "absent" "absent" "Skipped Gerrit submodule checkout" "Skipped" \
        "No submodule metadata exists yet"
    echo "No submodules recorded; metadata-only checkout complete"
    exit 0
}
if [[ "${XWALK_GITHUB_METADATA_ONLY:-false}" == "true" ]]; then
    git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' >/dev/null
    git -C "$root" ls-files --stage | awk '$1 == "160000" {found=1} END {exit !found}'
    log_checkout "recorded" "not-initialized" "Validated superproject metadata" "Verified" \
        "Gitlinks and .gitmodules entries exist"
    echo "Validated superproject metadata without private Gerrit checkout"
    exit 0
fi
: "${GERRIT_SUBMODULE_SSH_KEY_FILE:?Provide the read-only Gerrit SSH key file through GitHub Secrets}"
: "${GERRIT_SUBMODULE_USERNAME:?Provide the read-only Gerrit account name through GitHub Secrets}"
: "${GERRIT_SERVER_HOST:?Provide the Gerrit server host through GitHub Secrets or variables}"
: "${GERRIT_SSH_PORT:=29418}"
[[ -f "$GERRIT_SUBMODULE_SSH_KEY_FILE" ]] || { echo "Missing Gerrit SSH key file" >&2; exit 2; }
key_mode="$(stat -c %a "$GERRIT_SUBMODULE_SSH_KEY_FILE")"
(( (8#$key_mode & 8#077) == 0 )) || {
    echo "Gerrit SSH key file must not allow group or other access" >&2
    exit 2
}
temporary="$(mktemp -d -t xwalk-github-submodules-XXXXXXXX)"
override_names=()
cleanup()
{
    local name
    git -C "$root" config --local --unset core.sshCommand >/dev/null 2>&1 || true
    for name in "${override_names[@]}"; do
        git -C "$root" config --local --unset "submodule.$name.url" >/dev/null 2>&1 || true
    done
    rm -rf -- "$temporary"
}
trap cleanup EXIT
known_hosts="$temporary/known_hosts"
if ! ssh-keyscan -T 10 -p "$GERRIT_SSH_PORT" "$GERRIT_SERVER_HOST" > "$known_hosts"; then
    log_checkout "recorded" "not-initialized" "Gerrit host-key discovery failed" "Failed" "" \
        "The configured Gerrit endpoint was unreachable"
    exit 1
fi
printf -v ssh_command '%q ' ssh -i "$GERRIT_SUBMODULE_SSH_KEY_FILE" -o BatchMode=yes \
    -o IdentitiesOnly=yes -o StrictHostKeyChecking=yes -o "UserKnownHostsFile=$known_hosts" \
    -p "$GERRIT_SSH_PORT"
git -C "$root" config --local core.sshCommand "$ssh_command"
git -C "$root" submodule sync --recursive
while read -r key path; do
    name="${key#submodule.}"
    name="${name%.path}"
    override_names+=("$name")
    git -C "$root" config --local "submodule.$name.url" \
        "ssh://$GERRIT_SUBMODULE_USERNAME@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$path"
done < <(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$')
if ! git -C "$root" submodule update --init --recursive; then
    log_checkout "recorded" "not-initialized" "Private Gerrit submodule checkout failed" "Failed" "" \
        "A required component revision was inaccessible"
    exit 1
fi
git -C "$root" submodule foreach --recursive 'git rev-parse --verify HEAD >/dev/null'
log_checkout "recorded" "exact-commits-initialized" "Checked out Gerrit submodules" "Verified" \
    "Every recursive submodule has a checked-out commit"
echo "Checked out exact Gerrit submodule commits"
