# xWalkSpi

`xWalkSpi` provides bounded, hardware-independent full-duplex SPI transactions
and an optional Linux `spidev` owner. The core stores one non-owning callback
context and never opens a platform device.

One transfer accepts 1 through 256 bytes. The Linux backend configures standard
SPI mode 0 through 3, a positive clock frequency, and 1 through 32 bits per
word. Linux owns chip-select assertion for the complete ioctl transaction.

## Layout

| Path | Responsibility |
| --- | --- |
| `core/include/xHal_Rpi5CarSpiTypes.h` | Configuration and callback types |
| `core/include/xHal_Rpi5CarSpi.h` | Bounded callback-driven SPI interface |
| `core/src/` | Validation, forwarding, and lifecycle |
| `hardware/include/xHal_Rpi5CarSpiLinux.h` | Linux descriptor ownership contract |
| `hardware/src/` | `spidev` configuration and full-duplex ioctl transaction |
| `test/src/` | Device-free host coverage |
| `hardware/test/src/` | Opt-in one-byte physical transfer smoke test |

## Host verification

```bash
cmake -S xWalkHal/xWalkSpi -B xWalkHal/xWalkSpi/build-host -DXWALK_SPI_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/xWalkSpi/build-host --parallel
ctest --test-dir xWalkHal/xWalkSpi/build-host --output-on-failure
```

## Raspberry Pi compilation

```bash
cmake -S xWalkHal/xWalkSpi -B xWalkHal/xWalkSpi/build-rpi -DXWALK_SPI_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkHal/xWalkSpi/build-rpi --parallel
ctest --test-dir xWalkHal/xWalkSpi/build-rpi -N -L hardware
```

The hardware test transmits one zero byte. Do not execute it until the selected
`/dev/spidev*` node, chip select, peripheral protocol, wiring, voltage, and power
state have been reviewed and approved.
