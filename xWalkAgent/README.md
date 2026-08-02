# xWalk Agent

`xWalkAgent` contains application-level coordinators that compose caller-owned xWalk HAL objects. The
aggregate target exports `xWalk::Agent` and contains the `xWalkPicarx`, `xWalkLineTracking`,
`xWalkSelfDrive`, SPI transfer, voice, camera capture, and `xWalkBoot` submodules. `xWalkLineTracking` ports
bounded grayscale line following, `xWalkSelfDrive` ports the named preset-action and background queue behavior,
and `xWalkVoiceActiveCar` contains sensor-aware, wake-word, and spoken-demo coordinators.
`xWalkCameraCapture` adapts the camera HAL to voice image input. `xWalkBoot` owns the process
hardware-composition lifetime, offline voice-provider graph, and device-free host stub.
`xWalkSpiTransfer` coordinates bounded full-duplex requests without owning the Linux device.

The command-line application is a separate sibling aggregate under `xWalkCLI`. The standalone Agent tree does
not contain or compose `xWalkCLI` or `xWalkController`.

Host verification is deterministic and uses in-memory callbacks:

```bash
cmake -S xWalkAgent -B xWalkAgent/build-host -DXWALK_AGENT_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAgent/build-host --parallel
ctest --test-dir xWalkAgent/build-host --output-on-failure
```

The RPi build compiles Linux backends and hardware-labelled tests without running them:

```bash
cmake -S xWalkAgent -B xWalkAgent/build-rpi -DXWALK_AGENT_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAgent/build-rpi --parallel
ctest --test-dir xWalkAgent/build-rpi -N -L hardware
```

Never execute the RPi test until the correct Raspberry Pi and Robot HAT are connected, wheels are lifted, the
camera and steering mechanisms have clear travel, and powered motion has been approved.
