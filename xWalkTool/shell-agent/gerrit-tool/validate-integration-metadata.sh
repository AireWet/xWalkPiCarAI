#!/usr/bin/env bash
set -Eeuo pipefail

root="$(git rev-parse --show-toplevel)"
"$root/xWalkTool/shell-agent/gerrit-tool/validate-publication-policy.sh"
components=(
    xWalkAgent xWalkAudioResources xWalkController xWalkHal xWalkIW xWalkLibrary
    DevloperNote xWalkTrace
)

[[ -f "$root/.gitmodules" ]] || {
    echo "Missing .gitmodules in the integrated repository" >&2
    exit 1
}

validate_integrated_sources()
{
    local component path
    for component in "${components[@]}"; do
        path="$component"
        [[ "$component" != DevloperNote ]] || path="devloper-note"
        [[ -d "$root/xWalk-rpi5/$path" && ! -L "$root/xWalk-rpi5/$path" ]] || {
            echo "Missing integrated source directory for $component" >&2
            exit 1
        }
        git -C "$root" ls-files --error-unmatch "xWalk-rpi5/$path" >/dev/null || {
            echo "Integrated source is not tracked for $component" >&2
            exit 1
        }
    done
    [[ "$({ git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' 2>/dev/null || true; } | wc -l)" -eq 0 ]]
    echo "Validated eight integrated module source trees"
}

validate_hierarchical_migration()
{
    local component config_name expected_path expected_url expected_branch path url branch
    local nested="$root/xWalk-rpi5/.gitmodules"
    local -A expected_top=(
        [xWalkTool]=1
        [xWalk-rpi5]=1
        [devloper-note]=DevloperNote
    )
    [[ -f "$nested" ]] || {
        echo "Missing nested xWalk-rpi5/.gitmodules" >&2
        exit 1
    }
    for component in "${!expected_top[@]}"; do
        path="$(git -C "$root" config -f .gitmodules --get "submodule.$component.path")"
        url="$(git -C "$root" config -f .gitmodules --get "submodule.$component.url")"
        branch="$(git -C "$root" config -f .gitmodules --get "submodule.$component.branch")"
        [[ "$path" == "$component" ]] || { echo "Invalid path for $component" >&2; exit 1; }
        expected_url="${expected_top[$component]}"
        [[ "$expected_url" != 1 ]] || expected_url="$component"
        [[ "$url" == ../"$expected_url" ]] || { echo "Invalid relative URL for $component" >&2; exit 1; }
        expected_branch=master
        [[ "$branch" == "$expected_branch" ]] || { echo "Invalid branch for $component" >&2; exit 1; }
    done
    while IFS= read -r path; do
        [[ -n "${expected_top[$path]:-}" ]] || {
            echo "Unexpected top-level submodule: $path" >&2
            exit 1
        }
    done < <(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | awk '{print $2}')

    for component in "${components[@]}"; do
        [[ "$component" != DevloperNote ]] || continue
        config_name="$component"
        expected_path="$component"
        path="$(git config -f "$nested" --get "submodule.$config_name.path")"
        url="$(git config -f "$nested" --get "submodule.$config_name.url")"
        branch="$(git config -f "$nested" --get "submodule.$config_name.branch")"
        [[ "$path" == "$expected_path" ]] || {
            echo "Invalid nested path for $component" >&2
            exit 1
        }
        [[ "$url" == ../"$component" ]] || {
            echo "Invalid nested relative URL for $component" >&2
            exit 1
        }
        [[ "$branch" == master ]] || {
            echo "Invalid nested branch for $component" >&2
            exit 1
        }
        [[ -d "$root/xWalk-rpi5/$expected_path" && ! -L "$root/xWalk-rpi5/$expected_path" ]] || {
            echo "Missing integrated source directory for $component" >&2
            exit 1
        }
        git -C "$root" ls-files --error-unmatch "xWalk-rpi5/$expected_path" >/dev/null || {
            echo "Integrated source is not tracked for $component" >&2
            exit 1
        }
    done
    [[ "$(git config -f "$nested" --get-regexp '^submodule\..*\.path$' | wc -l)" -eq 7 ]] || {
        echo "Expected seven nested xWalk-rpi5 component mappings" >&2
        exit 1
    }
    echo "Validated root developer notes and hierarchical migration metadata"
}

validate_gitlink_superproject()
{
    local component expected_path expected_url path url branch mode
    local -A expected=()
    for component in "${components[@]}"; do
        expected_path="$component"
        [[ "$component" != DevloperNote ]] || expected_path="devloper-note"
        expected_url="$component"
        expected["$expected_path"]=1
        path="$(git -C "$root" config -f .gitmodules --get "submodule.$component.path")"
        url="$(git -C "$root" config -f .gitmodules --get "submodule.$component.url")"
        branch="$(git -C "$root" config -f .gitmodules --get "submodule.$component.branch")"
        [[ "$path" == "$expected_path" ]] || { echo "Invalid path for $component" >&2; exit 1; }
        [[ "$branch" == master ]] || { echo "Invalid branch for $component" >&2; exit 1; }
        [[ "$url" != *github.com* && "$url" == */"$expected_url" ]] || {
            echo "Submodule $component must point only to its Gerrit repository" >&2
            exit 1
        }
        mode="$(git -C "$root" ls-files --stage -- "$path" | awk '{print $1}')"
        [[ "$mode" == 160000 ]] || { echo "Missing gitlink for $component" >&2; exit 1; }
    done
    while IFS= read -r path; do
        [[ -n "${expected[$path]:-}" ]] || { echo "Unexpected submodule: $path" >&2; exit 1; }
    done < <(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | awk '{print $2}')
    echo "Validated eight exact Gerrit component gitlinks"
}

submodule_count="$({ git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' 2>/dev/null || true; } | wc -l)"
case "$submodule_count" in
    0) validate_integrated_sources ;;
    3) validate_hierarchical_migration ;;
    8) validate_gitlink_superproject ;;
    *) echo "Expected zero integrated, three hierarchical, or eight direct submodule mappings" >&2; exit 1 ;;
esac
