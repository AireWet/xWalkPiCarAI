#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../../.." && pwd)"
test_directory="$(mktemp -d)"
cleanup() {
    rm -rf -- "$test_directory"
}
trap cleanup EXIT HUP INT TERM

build_directory="$test_directory/cmake"
cmake -S "$repository_root/xWalk-rpi5" -B "$build_directory" -G Ninja \
    -DBUILD_TESTING=OFF \
    -DXWALK_BUILD_RPI=ON \
    -DXWALK_ENABLE_PACKAGING=OFF >/dev/null

cmake -LA -N "$build_directory" > "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_PROFILE:STRING=robot_hat_v4' "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_RUNTIME_USER:STRING=xwalk' "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_GPIO_DEVICE:FILEPATH=/dev/gpiochip4' "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_I2C_DEVICE:FILEPATH=/dev/i2c-1' "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_SPI_DEVICE:FILEPATH=/dev/spidev0.0' "$test_directory/cache.log"
grep -Fxq 'XWALK_RPI_CAMERA:STRING=csi' "$test_directory/cache.log"

cmake --build "$build_directory" --target help | grep -q '^rpi-provision:'
ninja -C "$build_directory" -t commands rpi-provision > "$test_directory/provision-commands.log"
grep -Fq 'setup-rpi-local.sh' "$test_directory/provision-commands.log"
grep -Fq 'setup-rpi.sh' "$test_directory/provision-commands.log"
grep -Fq -- '--profile robot_hat_v4' "$test_directory/provision-commands.log"
grep -Fq -- '--runtime-user xwalk' "$test_directory/provision-commands.log"
grep -Fq -- '--gpio-device /dev/gpiochip4' "$test_directory/provision-commands.log"
grep -Fq -- '--i2c-device /dev/i2c-1' "$test_directory/provision-commands.log"
grep -Fq -- '--spi-device /dev/spidev0.0' "$test_directory/provision-commands.log"
grep -Fq -- '--camera csi' "$test_directory/provision-commands.log"
grep -Fq -- '--template-config' "$test_directory/provision-commands.log"
grep -Fq -- '--template-fragments' "$test_directory/provision-commands.log"
grep -Fq -- '--with-vosk' "$test_directory/provision-commands.log"
grep -Fq -- '--validate-ollama' "$test_directory/provision-commands.log"
grep -Fq 'xwalk-picarx-control' "$test_directory/provision-commands.log"

override_build_directory="$test_directory/override-cmake"
cmake -S "$repository_root/xWalk-rpi5" -B "$override_build_directory" -G Ninja \
    -DBUILD_TESTING=OFF \
    -DXWALK_BUILD_RPI=ON \
    -DXWALK_ENABLE_PACKAGING=OFF \
    -DXWALK_RPI_PROFILE=robot_hat_v5 \
    -DXWALK_RPI_RUNTIME_USER=operator \
    -DXWALK_RPI_GPIO_DEVICE=/dev/gpiochip7 \
    -DXWALK_RPI_I2C_DEVICE=/dev/i2c-3 \
    -DXWALK_RPI_SPI_DEVICE=/dev/spidev2.1 \
    -DXWALK_RPI_CAMERA=usb >/dev/null
cmake -LA -N "$override_build_directory" > "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_PROFILE:STRING=robot_hat_v5' "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_RUNTIME_USER:STRING=operator' "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_GPIO_DEVICE:FILEPATH=/dev/gpiochip7' "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_I2C_DEVICE:FILEPATH=/dev/i2c-3' "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_SPI_DEVICE:FILEPATH=/dev/spidev2.1' "$test_directory/override-cache.log"
grep -Fxq 'XWALK_RPI_CAMERA:STRING=usb' "$test_directory/override-cache.log"

ninja -C "$build_directory" -n > "$test_directory/ordinary-build.log"
if grep -Eq 'setup-rpi(-local)?\.sh' "$test_directory/ordinary-build.log"; then
    echo "The ordinary build unexpectedly includes Raspberry Pi provisioning." >&2
    exit 1
fi

python3 -m json.tool "$repository_root/xWalk-rpi5/CMakePresets.json" >/dev/null
grep -Fq '"name": "rpi-provision"' "$repository_root/xWalk-rpi5/CMakePresets.json"

echo "RPi CMake provisioning target test passed"
