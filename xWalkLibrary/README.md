# xWalk runtime models

This directory contains reviewed third-party runtime assets used by the xWalk
HAL examples. Application code must receive these paths through configuration;
it must not assume the process working directory.

## Vosk inventory

| Asset | Version | Target | Runtime path |
|---|---|---|---|
| Vosk shared library | 0.3.45 | Linux ARM64/AArch64 | `vosk/lib/aarch64/libvosk.so` |
| Vosk shared library | 0.3.45 | Linux x86-64 | `vosk/lib/x86_64/libvosk.so` |
| Vosk C API header | 0.3.45 | Platform-independent declarations | `vosk/include/vosk_api.h` |
| Small US English model | 0.15 | Vosk offline recognition | `vosk/models/vosk-model-small-en-us-0.15` |
| CMake selector | Project | Target architecture mapping | `VoskModel.cmake` |

The runtime archive comes from the official
[`vosk-api` 0.3.45 release](https://github.com/alphacep/vosk-api/releases/tag/v0.3.45).
The model comes from the official
[Vosk model catalog](https://alphacephei.com/vosk/models). Both are distributed
under the Apache License 2.0; the retained license is
[`vosk/LICENSE`](vosk/LICENSE).

## Reviewed archive checksums

```text
45e95d37755deb07568e79497d7feba8c03aee5a9e071df29961aa023fd94541  vosk-linux-aarch64-0.3.45.zip
bbdc8ed85c43979f6443142889770ea95cbfbc56cffb5c5dcd73afa875c5fbb2  vosk-linux-x86_64-0.3.45.zip
30f26242c4eb449f948e42cb302dd7a686cb29a3423a8367f99ff41780942498  vosk-model-small-en-us-0.15.zip
```

The retained ARM64 and x86-64 `libvosk.so` SHA-256 checksums are:

```text
0e9df29f060a93cf3df3263a4d3635e1b75688a5fd84e86ade1599372e3c9597  lib/aarch64/libvosk.so
85c4654de3acdeb99abab86eeb2a6e603927d37089597c0fcc33d8638dc2ccaf  lib/x86_64/libvosk.so
```

CMake selects `x86_64` for an x86-64 target and `aarch64` for an ARM64 target.
The shared speech model is architecture-independent. A deployment or cross
build can override `XWALK_VOSK_ARCHITECTURE`, `XWALK_VOSK_LIBRARY_PATH`, or
`XWALK_VOSK_MODEL_PATH`. No bundled runtime supports 32-bit Raspberry Pi OS.
