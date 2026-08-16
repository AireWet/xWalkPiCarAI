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
    [[ "$(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | wc -l)" -eq 1 ]]
    [[ "$(git -C "$root" config -f .gitmodules --get submodule.xWalk-rpi5-py3.path)" == xWalk-rpi5-py3 ]]
    [[ "$(git -C "$root" config -f .gitmodules --get submodule.xWalk-rpi5-py3.branch)" == master ]]
    local simulation_url
    simulation_url="$(git -C "$root" config -f .gitmodules --get submodule.xWalk-rpi5-py3.url)"
    [[ "$simulation_url" != *github.com* && "$simulation_url" == */xWalk-rpi5-sim ]] || {
        echo "Simulation submodule must point only to its Gerrit repository" >&2
        exit 1
    }
    [[ "$(git -C "$root" ls-files --stage -- xWalk-rpi5-py3 | awk '{print $1}')" == 160000 ]] || {
        echo "Missing xWalk-rpi5-py3 gitlink" >&2
        exit 1
    }
    echo "Validated eight integrated module source trees and the Gerrit simulation gitlink"
}

validate_hierarchical_migration()
{
    local component config_name expected_path expected_url expected_branch mode path url branch
    local simulation_config simulation_path
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
    if git -C "$root" config -f .gitmodules --get submodule.xWalk-rpi5-py3.path >/dev/null; then
        simulation_config=xWalk-rpi5-py3
    elif git -C "$root" config -f .gitmodules --get submodule.xWalk-rpi5-sim.path >/dev/null; then
        simulation_config=xWalk-rpi5-sim
    else
        echo "Missing hierarchical Gerrit simulation mapping" >&2
        exit 1
    fi
    simulation_path="$simulation_config"
    expected_top["$simulation_config"]=xWalk-rpi5-sim
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
    mode="$(git -C "$root" ls-files --stage -- "$simulation_path" | awk -v path="$simulation_path" \
        '$4 == path {print $1}')"
    [[ "$mode" == 160000 ]] || { echo "Missing $simulation_path gitlink" >&2; exit 1; }
    echo "Validated root developer notes, hierarchical migration metadata, and the Gerrit simulation gitlink"
}

validate_gitlink_superproject()
{
    local component expected_path expected_url path url branch mode
    local -A expected=()
    for component in "${components[@]}" xWalk-rpi5-py3; do
        expected_path="$component"
        [[ "$component" != DevloperNote ]] || expected_path="devloper-note"
        expected_url="$component"
        [[ "$component" != xWalk-rpi5-py3 ]] || expected_url=xWalk-rpi5-sim
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
    echo "Validated nine exact Gerrit component gitlinks"
}

submodule_count="$(git -C "$root" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | wc -l)"
case "$submodule_count" in
    1) validate_integrated_sources ;;
    4) validate_hierarchical_migration ;;
    9) validate_gitlink_superproject ;;
    *) echo "Expected one legacy, four hierarchical, or nine direct submodule mappings" >&2; exit 1 ;;
esac
