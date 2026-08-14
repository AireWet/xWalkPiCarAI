#!/usr/bin/env bash
set -Eeuo pipefail

root="$(git rev-parse --show-toplevel)"
components=(
    xWalkAgent xWalkAudioResources xWalkController xWalkHal xWalkIW xWalkLibrary
    DevloperNote xWalkTrace
)

[[ -f "$root/.gitmodules" ]] || {
    echo "Missing .gitmodules in xWalk-rpi5" >&2
    exit 1
}

declare -A expected=()
for component in "${components[@]}"; do
    expected_path="$component"
    [[ "$component" != DevloperNote ]] || expected_path="devloper-note"
    expected["$expected_path"]=1
    path="$(git -C "$root" config -f .gitmodules --get "submodule.$component.path")"
    url="$(git -C "$root" config -f .gitmodules --get "submodule.$component.url")"
    branch="$(git -C "$root" config -f .gitmodules --get "submodule.$component.branch")"
    [[ "$path" == "$expected_path" ]] || { echo "Invalid path for $component" >&2; exit 1; }
    [[ "$branch" == main ]] || { echo "Invalid branch for $component" >&2; exit 1; }
    [[ "$url" != *github.com* && "$url" == */"$component" ]] || {
        echo "Submodule $component must point only to its Gerrit repository" >&2
        exit 1
    }
    mode="$(git -C "$root" ls-files --stage -- "$path" | awk '{print $1}')"
    [[ "$mode" == 160000 ]] || { echo "Missing gitlink for $component" >&2; exit 1; }
done

while IFS= read -r path; do
    [[ -n "${expected[$path]:-}" ]] || { echo "Unexpected submodule: $path" >&2; exit 1; }
done < <(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | awk '{print $2}')

[[ "$(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | wc -l)" -eq 8 ]]
echo "Validated eight exact Gerrit component gitlinks"
