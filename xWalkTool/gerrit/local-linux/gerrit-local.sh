#!/bin/sh
set -eu

dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
root="$(CDPATH='' cd -- "$dir/.." && pwd -P)"
export XWALK_GERRIT_SETUP_CONFIG="$dir/gerrit-local.conf"
exec "$root/shell-script/gerrit-setup.sh" "$@"
