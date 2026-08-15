#!/usr/bin/env bash
set -Eeuo pipefail

[[ "$#" -eq 2 ]] || {
    echo "Usage: overlay-gerrit-component.sh SOURCE_CHECKOUT TARGET_DIRECTORY" >&2
    exit 2
}
source_checkout="$1"
target_directory="$2"
root="$(git rev-parse --show-toplevel)"
source_checkout="$(realpath -- "$source_checkout")"
target_directory="$(realpath -- "$target_directory")"
[[ -d "$source_checkout/.git" || -f "$source_checkout/.git" ]] || {
    echo "Component checkout is not a Git work tree" >&2
    exit 2
}
case "$target_directory/" in
    "$root/xWalk-rpi5/"*) ;;
    *) echo "Component target must remain below xWalk-rpi5" >&2; exit 2 ;;
esac
[[ -d "$target_directory" && ! -L "$target_directory" ]] || {
    echo "Component target is missing or symbolic" >&2
    exit 2
}
relative_target="${target_directory#"$root/"}"
git -C "$root" rm --quiet -r -- "$relative_target"
mkdir -p "$target_directory"
git -C "$source_checkout" archive HEAD | tar -x -C "$target_directory"
git -C "$root" add -- "$relative_target"
echo "Overlaid exact Gerrit component patch set at $relative_target"
