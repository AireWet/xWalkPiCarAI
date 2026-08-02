#!/usr/bin/env bash

set -eu

usage() {
    echo "Usage: $0 --profile robot_hat_v4|robot_hat_v5 --runtime-user USER"
    echo "  --gpio-device /dev/gpiochipN [--i2c-device /dev/i2c-N]"
    echo "  [--spi-device /dev/spidevN.N] [--config FILE] [--camera csi|usb]"
    echo "  [--with-vosk] [--with-ollama] [--check|--validate|--dry-run|--apply]"
}

profile=""
runtime_user=""
gpio_device=""
i2c_device="/dev/i2c-1"
spi_device="/dev/spidev0.0"
config_file="/var/lib/xwalk/picar-x.conf"
camera="csi"
with_vosk="false"
with_ollama="false"
mode="dry-run"

while [ "$#" -gt 0 ]; do
    case "$1" in
        --profile) profile="${2-}"; shift 2 ;;
        --runtime-user) runtime_user="${2-}"; shift 2 ;;
        --gpio-device) gpio_device="${2-}"; shift 2 ;;
        --i2c-device) i2c_device="${2-}"; shift 2 ;;
        --spi-device) spi_device="${2-}"; shift 2 ;;
        --config) config_file="${2-}"; shift 2 ;;
        --camera) camera="${2-}"; shift 2 ;;
        --with-vosk) with_vosk="true"; shift ;;
        --with-ollama) with_ollama="true"; shift ;;
        --check|--validate) mode="check"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --apply) mode="apply"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage; exit 2 ;;
    esac
done

if [ "$profile" != "robot_hat_v4" ] && [ "$profile" != "robot_hat_v5" ]; then
    echo "An explicit robot_hat_v4 or robot_hat_v5 profile is required." >&2
    exit 2
fi
if [ -z "$runtime_user" ] || ! id "$runtime_user" >/dev/null 2>&1; then
    echo "--runtime-user must name an existing operating-system user." >&2
    exit 2
fi
if [ -z "$gpio_device" ]; then
    echo "Select the GPIO controller explicitly with --gpio-device." >&2
    exit 2
fi
if [ "$camera" != "csi" ] && [ "$camera" != "usb" ]; then
    echo "--camera must be csi or usb." >&2
    exit 2
fi

test_root="${XWALK_SETUP_TEST_ROOT-}"
if [ -n "$test_root" ] && [ "$mode" = "apply" ]; then
    echo "Apply mode is disabled when XWALK_SETUP_TEST_ROOT is set." >&2
    exit 2
fi
root_path() {
    printf '%s%s\n' "$test_root" "$1"
}

os_release="$(root_path /etc/os-release)"
if [ ! -r "$os_release" ]; then
    echo "Unable to identify the target operating system." >&2
    exit 2
fi
os_id="$(sed -n 's/^ID=//p' "$os_release" | tr -d '"' | head -n 1)"
case "$os_id" in
    debian|raspbian|ubuntu) ;;
    *) echo "Unsupported operating system: $os_id" >&2; exit 2 ;;
esac

model=""
model_path="$(root_path /proc/device-tree/model)"
if [ -r "$model_path" ]; then
    model="$(tr -d '\000' < "$model_path")"
fi
case "$model" in
    *"Raspberry Pi"*) ;;
    *) echo "This setup script must run on a Raspberry Pi." >&2; exit 2 ;;
esac

v5_uuid="9daeea78-0000-076e-0032-582369ac3e02"
v5_detected="false"
for uuid_file in "$(root_path /proc/device-tree)"/*hat*/uuid; do
    if [ -r "$uuid_file" ] && [ "$(tr -d '\000' < "$uuid_file")" = "$v5_uuid" ]; then
        v5_detected="true"
    fi
done
if [ "$profile" = "robot_hat_v4" ] && [ "$v5_detected" = "true" ]; then
    echo "The v4 profile conflicts with the detected Robot HAT v5 UUID." >&2
    exit 2
fi
if [ "$profile" = "robot_hat_v5" ] && [ "$v5_detected" != "true" ]; then
    echo "The v5 profile requires the supported Robot HAT v5 Device Tree UUID." >&2
    exit 2
fi

printf '%s\n' "$gpio_device" | grep -Eq '^/dev/gpiochip[0-9]+$' || {
    echo "Invalid GPIO device path: $gpio_device" >&2
    exit 2
}
printf '%s\n' "$i2c_device" | grep -Eq '^/dev/i2c-[0-9]+$' || {
    echo "Invalid I2C device path: $i2c_device" >&2
    exit 2
}
printf '%s\n' "$spi_device" | grep -Eq '^/dev/spidev[0-9]+\.[0-9]+$' || {
    echo "Invalid SPI device path: $spi_device" >&2
    exit 2
}

boot_config=""
if [ -f "$(root_path /boot/firmware/config.txt)" ]; then
    boot_config="$(root_path /boot/firmware/config.txt)"
elif [ -f "$(root_path /boot/config.txt)" ]; then
    boot_config="$(root_path /boot/config.txt)"
else
    echo "No Raspberry Pi config.txt was found." >&2
    exit 2
fi

for setting in i2c_arm spi; do
    if grep -Eq "^[[:space:]]*dtparam=${setting}=off([[:space:]]|$)" "$boot_config"; then
        echo "Resolve the conflicting dtparam=${setting}=off in $boot_config first." >&2
        exit 2
    fi
done

required_packages="build-essential cmake ninja-build pkg-config linux-libc-dev"
required_packages="$required_packages libasound2-dev alsa-utils libcurl4-openssl-dev"
required_packages="$required_packages libsndfile1-dev i2c-tools libi2c-dev gpiod"
required_packages="$required_packages espeak-ng curl ca-certificates"
if [ "$camera" = "csi" ]; then
    required_packages="$required_packages rpicam-apps"
else
    required_packages="$required_packages ffmpeg"
fi
missing_packages=""
for package in $required_packages; do
    if ! dpkg-query -W -f='${Status}' "$package" 2>/dev/null | grep -q 'ok installed'; then
        missing_packages="$missing_packages $package"
    fi
done

effective_config="$(root_path "$config_file")"
config_ready="true"
if [ ! -f "$effective_config" ]; then
    config_ready="false"
fi

configuration_value() {
    awk -F= -v key="$1" '
        $1 ~ "^[[:space:]]*" key "[[:space:]]*$" {
            value=$2
            sub(/^[[:space:]]*/, "", value)
            sub(/[[:space:]]*$/, "", value)
            print value
            exit
        }
    ' "$effective_config"
}

script_directory="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
rule_template="$script_directory/../deployment/udev/99-xwalk-picarx.rules.in"
if [ ! -f "$rule_template" ]; then
    rule_template="$script_directory/../../share/xwalk/deployment/udev/99-xwalk-picarx.rules.in"
fi
if [ ! -f "$rule_template" ]; then
    echo "The xWalk udev rule template was not found." >&2
    exit 2
fi

echo "xWalk Raspberry Pi provisioning plan"
echo "  mode: $mode"
echo "  operating system: $os_id"
echo "  board: $model"
echo "  Robot HAT profile: $profile"
echo "  runtime user: $runtime_user"
echo "  writable configuration: $config_file"
if [ "$config_ready" != "true" ]; then
    echo "  configuration action: install the default file before apply mode"
fi
echo "  exact devices: $i2c_device, $gpio_device, $spi_device"
if [ -n "$missing_packages" ]; then
    echo "  install packages:$missing_packages"
else
    echo "  packages: already installed"
fi
for setting in i2c_arm spi; do
    if grep -Eq "^[[:space:]]*dtparam=${setting}=on([[:space:]]|$)" "$boot_config"; then
        echo "  dtparam=$setting: already enabled"
    else
        echo "  append dtparam=$setting=on to $boot_config"
    fi
done
echo "  create xwalk, i2c, gpio, and spi groups when absent"
echo "  add $runtime_user to xwalk, i2c, gpio, spi, audio, video, and render when available"
echo "  install one udev rule for only the three selected device nodes"
echo "  Robot HAT overlays: no overlay will be installed or changed"
if [ "$with_vosk" = "true" ]; then
    if [ "$config_ready" = "true" ]; then
        vosk_library="$(configuration_value voice_vosk_library)"
        vosk_model="$(configuration_value voice_vosk_model)"
        if ldconfig -p 2>/dev/null | grep -Fq "$vosk_library" && [ -r "$vosk_model" ]; then
            echo "  Vosk: configured library and model are locally available"
        else
            echo "  Vosk: configured library or model is missing; install it from an approved source"
        fi
    else
        echo "  Vosk: validation deferred until the configuration is installed"
    fi
fi
if [ "$with_ollama" = "true" ]; then
    if [ "$config_ready" = "true" ]; then
        ollama_manifest="$(configuration_value voice_ollama_model_manifest)"
        if command -v ollama >/dev/null 2>&1 && [ -n "$ollama_manifest" ] && \
            [ -r "$ollama_manifest" ]; then
            echo "  Ollama: executable and configured model manifest are locally available"
        else
            echo "  Ollama: executable or model manifest is missing; install from an approved source"
        fi
    else
        echo "  Ollama: validation deferred until the configuration is installed"
    fi
fi

if [ "$mode" = "check" ]; then
    required_failures=0
    if [ -n "$missing_packages" ]; then
        echo "[FAIL] required packages:$missing_packages"
        required_failures=$((required_failures + 1))
    fi
    for setting in i2c_arm spi; do
        if ! grep -Eq "^[[:space:]]*dtparam=${setting}=on([[:space:]]|$)" "$boot_config"; then
            echo "[FAIL] required interface: dtparam=$setting is not enabled"
            required_failures=$((required_failures + 1))
        fi
    done
    for device in "$gpio_device" "$i2c_device" "$spi_device"; do
        if [ ! -e "$(root_path "$device")" ]; then
            echo "[FAIL] required device: $device is unavailable"
            required_failures=$((required_failures + 1))
        fi
    done
    if [ "$config_ready" != "true" ]; then
        echo "[FAIL] required configuration: $config_file is unavailable"
        required_failures=$((required_failures + 1))
    fi
    if [ "$required_failures" -ne 0 ]; then
        echo "Validation failed with $required_failures required issue(s)."
        exit 1
    fi
    echo "Validation passed. Optional components are reported separately above."
    exit 0
fi

if [ "$mode" = "dry-run" ]; then
    echo "Dry run complete. Re-run with --apply to perform the listed privileged changes."
    exit 0
fi

if [ "$config_ready" != "true" ]; then
    config_template="/etc/xwalk/picar-x.conf"
    if [ ! -r "$config_template" ]; then
        echo "Apply mode requires the installed template at $config_template." >&2
        exit 2
    fi
fi

if [ "$(id -u)" -eq 0 ]; then
    root_command=""
elif command -v sudo >/dev/null 2>&1; then
    root_command="sudo"
else
    echo "Apply mode requires root privileges or sudo." >&2
    exit 2
fi

run_root() {
    if [ -n "$root_command" ]; then
        "$root_command" "$@"
    else
        "$@"
    fi
}

if [ -n "$missing_packages" ]; then
    run_root apt-get update
    # Word splitting is intentional: this list contains validated package names only.
    run_root apt-get install -y $missing_packages
fi

for setting in i2c_arm spi; do
    if ! grep -Eq "^[[:space:]]*dtparam=${setting}=on([[:space:]]|$)" "$boot_config"; then
        printf '\ndtparam=%s=on\n' "$setting" | run_root tee -a "$boot_config" >/dev/null
    fi
done

for group in xwalk i2c gpio spi; do
    if ! getent group "$group" >/dev/null 2>&1; then
        run_root groupadd --system "$group"
    fi
done
runtime_groups="xwalk,i2c,gpio,spi"
for group in audio video render; do
    if getent group "$group" >/dev/null 2>&1; then
        runtime_groups="$runtime_groups,$group"
    fi
done
run_root usermod -a -G "$runtime_groups" "$runtime_user"

config_directory="$(dirname -- "$config_file")"
run_root install -d -m 0770 -o root -g xwalk "$config_directory"
if [ ! -f "$config_file" ]; then
    run_root install -m 0660 -o root -g xwalk /etc/xwalk/picar-x.conf "$config_file"
fi
if [ -f "$config_file" ]; then
    run_root chown root:xwalk "$config_file"
    run_root chmod 0660 "$config_file"
else
    echo "Install the default configuration at $config_file before running the application." >&2
    exit 2
fi

temporary_rule="$(mktemp)"
trap 'rm -f "$temporary_rule"' EXIT
sed -e "s/@I2C_KERNEL@/$(basename "$i2c_device")/g" \
    -e "s/@GPIO_KERNEL@/$(basename "$gpio_device")/g" \
    -e "s/@SPI_KERNEL@/$(basename "$spi_device")/g" \
    "$rule_template" > "$temporary_rule"
run_root install -m 0644 -o root -g root "$temporary_rule" /etc/udev/rules.d/99-xwalk-picarx.rules
run_root udevadm control --reload-rules
run_root udevadm trigger --subsystem-match=i2c-dev
run_root udevadm trigger --subsystem-match=gpio
run_root udevadm trigger --subsystem-match=spidev

provision_script="$script_directory/provision-hardware.sh"
if [ -x "$provision_script" ] && [ -e "$gpio_device" ]; then
    run_root "$provision_script" --profile "$profile" --config "$config_file" \
        --gpio-device "$gpio_device" --i2c-device "$i2c_device" --spi-device "$spi_device"
else
    echo "GPIO identity provisioning is deferred until $gpio_device is available."
fi

echo "Provisioning complete. Reboot if an interface was enabled, then log out and back in."
echo "Run xwalk-picarx-control doctor before actuator calibration."
