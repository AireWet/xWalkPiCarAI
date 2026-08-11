# xWalk project-managed dependencies

This directory is the reviewed prefix for portable third-party libraries, native runtimes, models, and
architecture-independent configuration. Operating-system tools and hardware integration remain outside this
tree and are managed by the platform.

## Layout

```text
xWalkLibrary/
├── X_WALK_LICENSE.KEY
├── common/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── xHal_Rpi5Car*.h
│   ├── configuration/
│   └── models/
│       └── vosk/
├── x86_64/
│   ├── bin/
│   ├── include/
│   ├── lib/
│   └── share/
└── aarch64/
    ├── bin/
    ├── include/
    ├── lib/
    └── share/
```

`X_WALK_LICENSE.KEY` is the fixed versioned authenticated-encryption output for
deployment environment values. It is data, never a compiler or linker input.
The repository marker is not a usable licence until it is replaced through the
[licence-key tool](../DevloperNote/Doc/note/License%20Key%20Workflow.md).

## Trace ownership

Compiled sources below `xWalkLibrary` use `XWALK_LIB_TRACE_UIDn` with
`LIB.<numeric-id>` identifiers. The build-time trace scanner rejects HAL,
Controller, or Agent trace families in this tree.

The header-only `common` target intentionally remains independent of
`xWalkTrace`, because the trace runtime itself depends on Common types. A
compiled Library component may link `xWalkTrace` privately when it emits a
Library trace; Common headers must not include the trace header merely to emit
diagnostics.

The `xHal_Rpi5Car*.h` files under `common` are the workspace-wide public C++ declarations exported by the
`xWalkLibraryCommon` interface target. Consumers include them by basename and must not add this directory globally.
`xHal_Rpi5CarTypes.h` exports generic shared aliases through the HAL (`hal`), Agent (`agent`), and Controller
(`ctrl`) namespaces while retaining HAL-specific types only in `xwalk::hal`.

Place architecture-independent models and configuration under `common`. Install a reviewed native dependency
into the conventional `bin`, `include`, `lib`, and `share` directories of each supported architecture. Never
copy a native binary between architecture prefixes.

Suitable project-managed dependencies include Vosk, GoogleTest, TinyXML2, yaml-cpp, Protobuf, gRPC,
libsndfile, and other ordinary portable C or C++ libraries. A library is not considered bundled merely because
its directory exists: retain its upstream version, source URL, checksum, license, and target architecture.

The compiler, linker, CMake, Ninja, Linux kernel interfaces, ALSA integration, udev rules, camera tools,
Device Tree overlays, system services, and package-management utilities remain system-managed. Do not extract
ordinary Debian packages into this prefix as a substitute for a reviewed relocatable build; package scripts,
absolute paths, transitive dependencies, and metadata may assume system installation locations.

## CMake selection

`XWalkDependencies.cmake` maps `CMAKE_SYSTEM_PROCESSOR` to `x86_64` or `aarch64`. It prepends the selected
native prefix and `common` to `CMAKE_PREFIX_PATH`, so compatible project-managed packages are preferred and
ordinary system discovery remains the fallback. Unsupported processors fail configuration instead of loading
a mismatched native library.

Root and dependency-owning standalone module builds include this selector automatically. An external consumer
can select the prefix explicitly:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$PWD/xWalkLibrary/x86_64"
```

For an AArch64 cross-build or Raspberry Pi target:

```sh
cmake -S . -B build-rpi -DCMAKE_PREFIX_PATH="$PWD/xWalkLibrary/aarch64"
```

Override automatic target selection with `-DXWALK_LIBRARY_ARCHITECTURE=aarch64` or `x86_64`. Set
`-DXWALK_LIBRARY_PREFER_PROJECT_DEPENDENCIES=OFF` to use normal system search order. Build-tree executables
receive an RPATH for the selected `lib` and `lib64` directories by default; disable it with
`-DXWALK_LIBRARY_USE_BUILD_RPATH=OFF`. Installed system packages retain the deployment RPATH policy of their
own install target and do not depend on `LD_LIBRARY_PATH`.

## Vosk inventory

| Asset | Version | Target | Runtime path |
|---|---|---|---|
| Vosk shared library | 0.3.45 | Linux ARM64/AArch64 | `aarch64/lib/libvosk.so` |
| Vosk shared library | 0.3.45 | Linux x86-64 | `x86_64/lib/libvosk.so` |
| Vosk C API header | 0.3.45 | ARM64/AArch64 declarations | `aarch64/include/vosk_api.h` |
| Vosk C API header | 0.3.45 | x86-64 declarations | `x86_64/include/vosk_api.h` |
| Small US English model | 0.15 | Architecture-independent | `common/models/vosk/vosk-model-small-en-us-0.15` |
| CMake selector | Project | Target architecture mapping | `VoskModel.cmake` |

The runtime archive comes from the official
[`vosk-api` 0.3.45 release](https://github.com/alphacep/vosk-api/releases/tag/v0.3.45). The model comes from
the official [Vosk model catalog](https://alphacephei.com/vosk/models). Both are distributed under Apache
License 2.0; the retained license is [`common/models/vosk/LICENSE`](common/models/vosk/LICENSE).

## Reviewed archive checksums

```text
45e95d37755deb07568e79497d7feba8c03aee5a9e071df29961aa023fd94541  vosk-linux-aarch64-0.3.45.zip
bbdc8ed85c43979f6443142889770ea95cbfbc56cffb5c5dcd73afa875c5fbb2  vosk-linux-x86_64-0.3.45.zip
30f26242c4eb449f948e42cb302dd7a686cb29a3423a8367f99ff41780942498  vosk-model-small-en-us-0.15.zip
```

The retained native-library SHA-256 checksums are:

```text
0e9df29f060a93cf3df3263a4d3635e1b75688a5fd84e86ade1599372e3c9597  aarch64/lib/libvosk.so
85c4654de3acdeb99abab86eeb2a6e603927d37089597c0fcc33d8638dc2ccaf  x86_64/lib/libvosk.so
```

A deployment or cross-build can override `XWALK_VOSK_ARCHITECTURE`, `XWALK_VOSK_LIBRARY_PATH`, or
`XWALK_VOSK_MODEL_PATH`. No bundled runtime supports 32-bit Raspberry Pi OS.
