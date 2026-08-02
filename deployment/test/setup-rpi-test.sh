#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
fixture_root="$(mktemp -d)"
trap 'rm -rf "$fixture_root"' EXIT

mkdir -p "$fixture_root/etc" "$fixture_root/proc/device-tree" \
    "$fixture_root/boot/firmware" "$fixture_root/var/lib/xwalk"
printf 'ID=ubuntu\nVERSION_ID="24.04"\n' > "$fixture_root/etc/os-release"
printf 'Raspberry Pi 5 Model B Rev 1.0\000' > "$fixture_root/proc/device-tree/model"
printf '# retained fixture setting\ndtparam=audio=on\n' > "$fixture_root/boot/firmware/config.txt"
cp "$repository_root/xWalkCLI/xWalkController/config/picar-x.conf" \
    "$fixture_root/var/lib/xwalk/picar-x.conf"

before_checksum="$(sha256sum "$fixture_root/boot/firmware/config.txt")"
XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --dry-run \
    > "$fixture_root/dry-run.log"
after_checksum="$(sha256sum "$fixture_root/boot/firmware/config.txt")"
test "$before_checksum" = "$after_checksum"
grep -q 'mode: dry-run' "$fixture_root/dry-run.log"
grep -q 'Robot HAT overlays: no overlay will be installed or changed' "$fixture_root/dry-run.log"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/setup-rpi.sh" \
    --profile robot_hat_v5 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --dry-run \
    > "$fixture_root/v5.log" 2>&1; then
    echo "Unverified Robot HAT v5 was not refused." >&2
    exit 1
fi
grep -q 'requires the supported Robot HAT v5 Device Tree UUID' "$fixture_root/v5.log"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --apply \
    > "$fixture_root/apply.log" 2>&1; then
    echo "Fixture apply mode was not refused." >&2
    exit 1
fi
grep -q 'Apply mode is disabled' "$fixture_root/apply.log"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --check \
    > "$fixture_root/check.log" 2>&1; then
    echo "Incomplete fixture unexpectedly passed required validation." >&2
    exit 1
fi
grep -q 'Validation failed' "$fixture_root/check.log"

echo "Provisioning host tests passed."
