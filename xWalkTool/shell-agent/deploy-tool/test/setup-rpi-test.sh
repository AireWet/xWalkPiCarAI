#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../../.." && pwd)"
fixture_root="$(mktemp -d)"
trap 'rm -rf "$fixture_root"' EXIT

mkdir -p "$fixture_root/etc" "$fixture_root/proc/device-tree" \
    "$fixture_root/boot/firmware" "$fixture_root/var/lib" "$fixture_root/bin"
printf 'ID=ubuntu\nVERSION_ID="24.04"\n' > "$fixture_root/etc/os-release"
printf 'Raspberry Pi 5 Model B Rev 1.0\000' > "$fixture_root/proc/device-tree/model"
printf '# retained fixture setting\ndtparam=audio=on\n' > "$fixture_root/boot/firmware/config.txt"
template_config="$repository_root/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf"
template_fragments="$repository_root/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.d"
vosk_library="$repository_root/xWalk-rpi5/xWalkLibrary/x86_64/lib/libvosk.so"
vosk_model="$repository_root/xWalk-rpi5/xWalkLibrary/common/models/vosk/vosk-model-small-en-us-0.15"
template_checksum="$(find "$repository_root/xWalk-rpi5/xWalkController/xWalkConfig" -type f \
    -exec sha256sum {} + | sort | sha256sum)"
template_mode="$(stat -c '%a:%U:%G' "$template_config")"

before_checksum="$(sha256sum "$fixture_root/boot/firmware/config.txt")"
XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 \
    --template-config "$template_config" --template-fragments "$template_fragments" \
    --with-vosk --vosk-library-source "$vosk_library" --vosk-model-source "$vosk_model" \
    --dry-run \
    > "$fixture_root/dry-run.log"
after_checksum="$(sha256sum "$fixture_root/boot/firmware/config.txt")"
test "$before_checksum" = "$after_checksum"
grep -q 'mode: dry-run' "$fixture_root/dry-run.log"
grep -q 'Robot HAT overlays: no overlay will be installed or changed' "$fixture_root/dry-run.log"
grep -q 'configuration action: initialize the writable copy' "$fixture_root/dry-run.log"
grep -q 'SELECTED GPIO DEVICE: /dev/gpiochip0' "$fixture_root/dry-run.log"
grep -q 'Vosk action: install repository assets' "$fixture_root/dry-run.log"
test "$template_checksum" = "$(find "$repository_root/xWalk-rpi5/xWalkController/xWalkConfig" -type f \
    -exec sha256sum {} + | sort | sha256sum)"
test "$template_mode" = "$(stat -c '%a:%U:%G' "$template_config")"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 \
    --config "$template_config" --template-config "$template_config" \
    --template-fragments "$template_fragments" --dry-run \
    > "$fixture_root/source-destination.log" 2>&1; then
    echo "Tracked source configuration was accepted as a writable destination." >&2
    exit 1
fi
grep -q 'Writable deployment paths must differ' "$fixture_root/source-destination.log"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v5 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --dry-run \
    > "$fixture_root/v5.log" 2>&1; then
    echo "Unverified Robot HAT v5 was not refused." >&2
    exit 1
fi
grep -q 'requires the supported Robot HAT v5 Device Tree UUID' "$fixture_root/v5.log"

mkdir -p "$fixture_root/proc/device-tree/robot-hat"
printf '9daeea78-0000-076e-0032-582369ac3e02\000' \
    > "$fixture_root/proc/device-tree/robot-hat/uuid"
XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v5 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 \
    --template-config "$template_config" --template-fragments "$template_fragments" --dry-run \
    > "$fixture_root/v5-valid.log"
grep -q 'Robot HAT profile: robot_hat_v5' "$fixture_root/v5-valid.log"
rm "$fixture_root/proc/device-tree/robot-hat/uuid"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --apply \
    > "$fixture_root/apply.log" 2>&1; then
    echo "Fixture apply mode was not refused." >&2
    exit 1
fi
grep -q 'Apply mode is disabled' "$fixture_root/apply.log"

if XWALK_SETUP_TEST_ROOT="$fixture_root" "$repository_root/xWalkTool/shell-agent/deploy-tool/setup-rpi.sh" \
    --profile robot_hat_v4 --runtime-user "$(id -un)" --gpio-device /dev/gpiochip0 --check \
    --template-config "$template_config" --template-fragments "$template_fragments" \
    > "$fixture_root/check.log" 2>&1; then
    echo "Incomplete fixture unexpectedly passed required validation." >&2
    exit 1
fi
grep -q 'Validation failed' "$fixture_root/check.log"
if grep -q '\[FAIL\] required configuration' "$fixture_root/check.log"; then
    echo "Repository template fallback was not accepted during validation." >&2
    exit 1
fi

cp "$template_config" "$fixture_root/runtime.conf"
printf '#!/usr/bin/env bash\nprintf "gpiochip4 [pinctrl-rp1] (54 lines)\\n"\n' \
    > "$fixture_root/bin/gpiodetect"
chmod 0755 "$fixture_root/bin/gpiodetect"
PATH="$fixture_root/bin:$PATH" XWALK_PROVISION_TEST_ROOT="$fixture_root" \
    "$repository_root/xWalkTool/shell-agent/deploy-tool/provision-hardware.sh" \
    --profile robot_hat_v4 --config "$fixture_root/runtime.conf" \
    --gpio-device /dev/gpiochip4 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0 \
    > "$fixture_root/provision.log"
grep -q '^hardware_board = robot_hat_v4$' "$fixture_root/runtime.conf"
grep -q '^hardware_gpio_device = /dev/gpiochip4$' "$fixture_root/runtime.conf"
grep -q '^hardware_gpio_chip_name = gpiochip4$' "$fixture_root/runtime.conf"
grep -q '^hardware_gpio_chip_label = pinctrl-rp1$' "$fixture_root/runtime.conf"
grep -q '^hardware_i2c_device = /dev/i2c-1$' "$fixture_root/runtime.conf"
grep -q '^hardware_spi_device = /dev/spidev0.0$' "$fixture_root/runtime.conf"

echo "Provisioning host tests passed."
