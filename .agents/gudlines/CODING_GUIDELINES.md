# xWalk HAL Coding Guidelines

## Purpose and authority

This guide records the conventions already established in `xWalkCommon`,
`xWalkI2c`, `xWalkPwm`, and `xWalkServo`. Apply it to new code and to code changed during
maintenance. Preserve the surrounding file's local style when it differs, but
do not copy accidental inconsistencies such as trailing whitespace or uneven
indentation.

These guidelines describe the current project rather than a generic C++ style.
If a deliberate architectural or project-wide convention changes, update this
file in the same change. Do not rewrite the guide merely to justify a one-off
exception.

## Language and compiler expectations

- Write C++17. Declare `cxx_std_17` on every public library target that needs to
  propagate this requirement.
- Keep code warning-clean with GCC and Clang under `-Wall`, `-Wextra`,
  `-Wpedantic`, `-Wconversion`, and `-Wsign-conversion`.
- Use the fixed project types from `xHal_Rpi5CarTypes.h`, such as `boolean`,
  `uint8`, `uint16`, `uint32`, `int32`, `float64`, and `size`, instead of adding
  unrelated spellings throughout the modules.
- Declare every reusable standard-library data type behind a documented alias
  in `xWalkCommon/include/xHal_Rpi5CarTypes.h`. Production modules and tests use
  the project alias instead of spelling the standard type directly. This
  includes containers, exceptions, streams, filesystem paths and metadata,
  synchronization types, and error codes.
- Put module-specific aggregate aliases and their related data structures in a
  documented type header under that submodule's `include` directory. For
  example, LineTracker fixed arrays, its non-owning ADC-pointer array, and
  `XWalkLineCalibration` belong in `xHal_Rpi5CarLineTrackerTypes.h`.
- Direct `std::` qualification remains appropriate for standard functions,
  algorithms, namespace constants, and the underlying type on the right side
  of an alias declared in `xWalkCommon`. Doxygen `@throws` fields use the real
  standard exception name presented to API callers, except where a public
  project exception alias such as `filesystemerror` is part of the interface.
- Make narrowing and signed/unsigned conversions explicit with `static_cast`.
- Break multi-stage hardware calculations into named `const` intermediate
  values. Convert constants and integer operands explicitly to the calculation
  type before arithmetic, and keep one conceptual multiplication or division
  per assignment. Name derived divisors and ratios so units and formula intent
  remain reviewable.
- Add an unsigned suffix to unsigned integer constants where conversion matters,
  for example `8U`, `0xFFU`, and `72'000'000.0`.
- Use `nullptr`, `noexcept`, `const`, and value initialization (`{}`) whenever
  their guarantees apply.
- Keep assertions enabled in every host verification configuration, including
  Release. New tests evaluate state-changing operations before asserting their
  results so static analysis and reviewers can see both the action and check.

## Project structure

Keep each C++ module independently configurable with its own `CMakeLists.txt`
and README. The workspace root `CMakeLists.txt` composes every HAL module and
maps the `XWALK_HAL_BUILD_HOST` and `XWALK_HAL_BUILD_RPI` flags to module
verification options. The `xWalkHal` directory intentionally has no aggregate
`CMakeLists.txt`. Both aggregate flags default to `OFF` and must not be enabled
in the same build directory. The default aggregate build contains production
libraries only. A host build must register every submodule host and unit test so plain
`ctest` runs the complete host suite. An RPI build must register every submodule
hardware test so plain `ctest` runs the complete hardware suite after deployment
and safety approval. The `Doc` directory has no build system. Use the existing
layout:

```text
.vscode/                     workspace configuration for HAL and CLI
.project                     Eclipse CDT workspace and host-build configuration
.cproject                    Eclipse CDT C++17 indexing and include-path configuration
.settings/                   Eclipse CDT project preferences
DevloperNote/Doc/note/       C++ Markdown documentation mirroring upstream pages
DevloperNote/index.md        C++ architecture and module documentation index
Doc/image/                   hardware and project images referenced by documentation
xWalkTool/                   workspace maintenance, verification, provisioning, and overlay resources
xWalkAgent/                  application coordinators composed from caller-owned HAL objects
xWalkAgent/xWalkPicarx/      complete PiCar-X movement, calibration, and sensing coordinator
xWalkAgent/xWalkLineTracking/ bounded grayscale line-following and recovery coordination
xWalkAgent/xWalkLocalVoiceChatbot/ foreground local voice-assistant loop and response filtering
xWalkAgent/xWalkCameraCapture/ camera-to-voice image callback adaptation
xWalkAgent/xWalkSpiTransfer/ bounded full-duplex SPI transaction coordination
xWalkAgent/xWalkVoiceActiveCar/ sensor-aware, wake-word, and spoken-demo vehicle coordination
xWalkAgent/xWalkVoiceActiveCarGpt/ English GPT voice-active-car profile
xWalkAgent/xWalkSelfDrive/   preset gestures, sound actions, and background action-flow coordination
xWalkAgent/xWalkBoot/        host-stub and Raspberry Pi process hardware composition
xWalkCLI/                    standalone command-line aggregate that imports xWalkAgent
xWalkCLI/xWalkController/    PiCar-X command parsing and Raspberry Pi application entry point
xWalkCommon/                 workspace-wide types, constants, and reusable facilities
xWalkMusics/                 packaged background-music resources and provenance
xWalkSounds/                 packaged sound-effect resources and provenance
xWalkIW/                     Protobuf and gRPC interface definitions for xWalkI2c
xWalkLibrary/                reviewed project-managed runtime models and native AI assets
CMakeLists.txt               workspace and HAL aggregate build
xWalkHal/xWalkBoardControl/  board control, discovery, and firmware information
xWalkHal/xWalkBuzzer/        active GPIO and passive PWM buzzer control
xWalkHal/xWalkCamera/        backend-neutral capture plus Linux CSI and USB providers
xWalkHal/xWalkConfig/        section-aware and flat key-value configuration persistence
xWalkHal/xWalkGPT/           speech coordination plus Linux Vosk and Espeak providers
xWalkHal/xWalkLed/           GPIO and three-channel PWM LED control
xWalkHal/xWalkLanguageModel/ provider-neutral conversation and prompting control
xWalkHal/xWalkMusic/         music theory, PCM tone, and injected audio control
xWalkHal/xWalkRobot/         coordinated multi-servo robot control
xWalkHal/xWalkSpeaker/       bounded asynchronous audio-file playback control
xWalkHal/xWalkTrace/         filtered callback-based embedded diagnostics
xWalkHal/xWalkUserButton/    active-low button events and press timing
xWalkHal/xWalkUltrasonic/    two-pin ultrasonic distance measurement
xWalkHal/xWalkUtils/         injected platform utilities and bounded lazy caching
xWalkHal/xWalkVoiceAssistant/ synchronous speech, model, and response coordination
xWalkHal/xWalkLineTracker/   grayscale sensing and line-position estimation
xWalkHal/xWalkAdc/           Robot HAT analog-to-digital converter module
xWalkHal/xWalkAudio/         shared ALSA PCM and mixer ownership
xWalkHal/xWalkGpio/          Robot HAT digital GPIO module and Linux backend
xWalkHal/xWalkSpi/           bounded callback-driven SPI and Linux spidev backend
xWalkHal/xWalkMotor/         single and paired Robot HAT motor control
xWalkHal/xWalk<Module>/include/ public module headers
xWalkHal/xWalk<Module>/src/  hardware-independent implementation
xWalkHal/xWalk<Module>/test/ host-side tests and test helpers
xWalkHal/xWalk<Module>/hardware/ optional platform backend and hardware tests
xWalkHal/xWalkTest/xGoogleTest/ centralized HAL host/hardware runner and XML selection
xWalkHal/xWalkTest/xExample/ centralized ported examples with core and hardware layers
xWalkHal/xWalkTest/xSequenceTest/ bounded opt-in sequence and integration tests
```

For a module with `core` and `hardware` layers, keep the hardware-independent
interface under `core/` and Linux-specific ownership and system calls under
`hardware/`.

Split implementation files by responsibility rather than allowing one source
file to grow indefinitely. Follow the established suffixes:

- `xHal_Rpi5Car<Component>.cpp` for normal behavior.
- `xHal_Rpi5Car<Component>Lifecycle.cpp` for constructors and destructors.
- A descriptive suffix such as `Timer`, `Output`, or `Validation` for a cohesive
  behavior group.
- The same responsibility-based split for test files.

Whenever files are added, update the module tree and responsibility table in
its README.

Keep AddressSanitizer/UndefinedBehaviorSanitizer and ThreadSanitizer in separate host build directories and
never combine them in one executable. Use the root presets for repeat-under-load host verification so a
failure stops the bounded repetition immediately.

Keep HAL unit-test implementations in each owning module's existing `test/`
tree. The central `xGoogleTest` target may compile those sources and adapt
legacy entry points, but it must contain only common runner code, use one
central `main()`, list sources explicitly, and remain disabled with
`BUILD_TESTING=OFF`. Physical hardware executables stay separate, use a
separate disabled-by-default XML profile, and require explicit runtime
selection.

Sequence tests that observe or manipulate physical hardware must use a bounded
runtime, carry the CTest `hardware;sequence` labels, and remain disabled in the
hardware XML profile until explicitly selected. Keep reusable, host-testable
logic under `core/include` and `core/src`; keep platform adapters and physical
implementations under `hardware/include` and `hardware/src`. Keep the single
sequence-runner process entry point at `xWalkHal/xWalkTest/xSequenceTest/main.cpp`.
Keep CLI usage, argument validation, and selector dispatch in the dedicated
hardware-layer `XWalkSequenceTestRunner`; `main.cpp` only delegates to it.

Port upstream example programs under `xWalkHal/xWalkTest/xExample`, keeping reusable
behavior in `core` and Raspberry Pi composition in `hardware`. Keep its only
process entry point at `xWalkHal/xWalkTest/xExample/main.cpp`; individual example
sources must not define `main()`. Add only real supplied examples. Select them
through the `xExample` executable using their formal selector names. Keep
default board, AI, and bounded selector arguments in the module YAML file and
accept an explicit YAML-path override; validated positional arguments remain a
compatibility override. Do not add xExample XML manifests, CTest registrations,
or xGoogleTest inventory entries. Keep CLI usage, argument validation,
environment lookup, and selector dispatch in the hardware-layer
`XWalkExampleRunner`; `main.cpp` only delegates to it. Place example contracts
and adapters in `namespace xwalk::hal::example`; reserve
`namespace xwalk::hal::test` for test implementations.

Keep reviewed offline AI runtime assets under the root-level `xWalkLibrary`. Record the
upstream version, architecture, source URL, checksum, license, and runtime path
in that directory. Architecture-specific native libraries must use an explicit
architecture subdirectory and must never be loaded by a mismatched target.
Generate absolute deployment paths into build-local configuration so execution
does not depend on the current working directory. Do not modify third-party
model or binary contents locally.

Name handwritten project YAML configuration files
`xHal_Rpi5Car<Component>Config.yml`, where `<Component>` matches the owning
module or runner, such as `Example`, `GoogleTest`, or `SequenceTest`. Keep
generated copies under the same filename so runtime diagnostics and deployment
instructions identify one stable configuration artifact.

Keep generated Protobuf and gRPC sources under the owning module's `auto-gen`
tree. Regenerate them from reviewed schemas, never edit them by hand, and
exclude them from handwritten-source coverage and static-analysis gates while
retaining normal compiler warnings and compilation checks.

## Files and naming

- Name project headers and sources `xHal_Rpi5Car<Component>.h` and
  `xHal_Rpi5Car<Component>.cpp`.
- Keep at most one class definition in each physical header or source file.
  Give each additional class its own consistently named file and include that
  file explicitly wherever the class is used. Supporting enums and structures
  may remain with the single class that owns their contract.
- Use include guards derived from the file name:

  ```cpp
  #ifndef XHAL_RPI5CAR_COMPONENT_H
  #define XHAL_RPI5CAR_COMPONENT_H

  // Declarations.

  #endif  // XHAL_RPI5CAR_COMPONENT_H
  ```

- Put production declarations in `namespace xwalk::hal`. Put tests in
  `namespace xwalk::hal::test`. End a namespace with a named comment.
- Put every non-member production function in `xWalkCommon` under
  `namespace xwalk::hal::common`. Call it with the `common::` qualifier from a
  module, except the file facilities described below, which live directly in
  `xwalk::hal`. Do not add anonymous or module-local free production helpers.
  Class-specific behavior remains a method on its owning class. Test scenarios,
  test-only callbacks, and the required global `main()` stay with their tests.
- Put reusable filesystem operations in
  `xWalkCommon/include/xHal_Rpi5CarFileFunctions.h`. Production modules and
  tests call these functions directly through `xwalk::hal`, matching project
  types such as `uint32`; do not place file facilities in a nested `common`
  namespace. Keep filesystem data types behind the aliases declared in
  `xHal_Rpi5CarTypes.h`.
- Keep file stream modes and filesystem operation options behind common typed
  constants. Use `FILE_OPEN_WRITE_TRUNCATE` and `FILE_PERMISSION_REPLACE`
  instead of spelling `std::ios` or
  `std::filesystem::perm_options` outside `xWalkCommon`. Use
  `readFileLine()` instead of calling `std::getline` on a file stream
  from a module.
- Use `FILE_OPEN_READ_BINARY` and `readFileContents()` when binary property data
  can contain terminal null bytes. Use `listFilesystemEntryNames()` for direct
  child enumeration rather than exposing filesystem iterators to a module.
- Name classes in PascalCase with the `XWalk` prefix, for example `XWalkI2c` and
  `XWalkPwmTimerState`.
- Name functions and local variables in lower camel case: `setPulseWidth`,
  `timerIndex`, and `requestedAddress`.
- Name stored scalar members with a descriptive `Value` suffix when it
  distinguishes state from a similarly named accessor or parameter:
  `addressValue`, `channelValue`, and `frequencyHzValue`.
- Keep project type aliases lowercase, following `bytevector`,
  `optionaluint8`, `filesystempath`, and `pwmtimerstatepointer`.
- Prefix public preprocessor constants and function-like macros with
  `XHAL_RPI5CAR_` or the narrower established `XHAL_` prefix. Use uppercase
  snake case.
- Use explicit units in names when a value could otherwise be ambiguous, such
  as `frequencyHz`, `pwmClockHz`, and `pulseWidthPercentValue`.

## Formatting

- Indent with four spaces; do not use tabs.
- Limit every line in C++ sources, headers, CMake files, and repository
  documentation to 115 characters, except complete shell commands inside fenced Markdown code blocks. Wrap any
  other line that would exceed this limit.
  Count leading indentation and all other characters when measuring the line.
  Do not wrap a C++ statement, declaration, condition, call, or expression when
  the complete construct fits within 115 characters. When wrapping is required,
  keep as much of the construct as practical on each line without exceeding the
  limit.
  Keep each fenced shell-command example on one physical line without a continuation backslash. A complete
  command in such a block may exceed 115 characters when required to preserve its one-line form.
- Use Allman braces for namespaces, classes, functions, and control statements:

  ```cpp
  if (value > maximum)
  {
      XHAL_THROW_OUT_OF_RANGE("value is outside its range");
  }
  ```

- Put one space after commas and around binary operators. Do not leave trailing
  whitespace.
- Keep related declarations together and separate logical groups with one blank
  line. Avoid repeated blank lines.
- In protocol-mirrored enums and payload structures, place a complete
  multi-line Doxygen block before every enumerator and data member. Use the same
  `/**`, `@brief`, optional non-empty `@details`, and `*/` form used for function
  contracts. Do not use trailing `/**< ... */` comments or single-line
  `/** @brief ... */` comments for these declarations.
- Wrap long parameter lists and expressions onto continuation lines. Align for
  readability without depending on tabs.
- Use aligned assignments only for a short, closely related block when it makes
  comparisons clearer.
- Keep small, unambiguous read-only accessors inline in the header. Put behavior,
  validation, locking, I/O, and non-trivial calculations in a source file.
- Prefer direct, compact comments that explain a constraint, compatibility
  choice, hardware behavior, or ownership decision. Do not narrate obvious code.
- Give every member-function definition in a `.cpp` file a complete
  Doxygen-compatible contract, even when its header declaration is already
  documented. Keep both contracts consistent as required by
  `DOCUMENTATION_GUIDELINES.md`.

## Headers and dependencies

- Include a source file's matching public header first.
- Add a blank line before additional project headers when they form a separate
  dependency group.
- Include project headers by their public basename, for example
  `#include "xHal_Rpi5CarPwm.h"`; let CMake provide include directories.
- Keep standard-library headers centralized in
  `xHal_Rpi5CarStandardHeaders.h`. Module code includes its project header and
  normally does not add direct standard-library includes.
- Directly include a standard header in a translation unit when that header is
  required to complete an implementation-only type and relying on a transitive
  include produces an incomplete-type diagnostic. Keep the matching project
  header first, then add a separate standard-header group. For example, a file
  that instantiates `std::ifstream` or `std::ofstream` includes `<fstream>`;
  `<iosfwd>` and the stream declarations exposed by `<filesystem>` are not
  complete definitions.
- Include `<filesystem>` directly in every public header that exposes or stores
  a `std::filesystem` type and in every translation unit that calls filesystem
  operations. Do not rely on another standard header to expose filesystem
  declarations transitively.
- Keep Linux-only headers centralized in `xHal_Rpi5CarLinuxHeaders.h` and out of
  hardware-independent source.
- Expose reusable standard operations through the existing common facilities.
  Use the `XHAL_*` math and exception macros instead of introducing direct
  `std::` calls throughout module code.
- Avoid unnecessary transitive coupling. Link each CMake target publicly only
  to dependencies used by its public interface; keep implementation-only and
  test dependencies private.

### VS Code IntelliSense configuration

- Treat the configured CMake build as the authoritative C++ result. IntelliSense
  diagnostics are editor assistance and do not replace compilation with the
  project's warning flags.
- When adding or renaming a module, add its public and test include directories
  to `../.vscode/c_cpp_properties.json`. Update both `includePath` and `browse.path`
  in the same change.
- Keep `compilerPath`, `cppStandard`, and `intelliSenseMode` consistent with the
  compiler and C++ language version selected by CMake. This project currently
  uses `/usr/bin/c++`, C++17, and `linux-gcc-x64`.
- Ensure IntelliSense can reach `xWalkCommon/include`. Project aliases such as
  `uint8`, `uint16`, `uint32`, and `float64` are declared there and are expected
  to be available through the module include chain.
- An unknown project type followed by an incorrect function signature or an
  `expected a ';'` message is usually a cascading parser diagnostic. First check
  whether the matching class header and its dependency headers are resolved.
- An `incomplete type is not allowed` diagnostic for a standard-library object
  usually means only its forward declaration is visible. Include the defining
  standard header directly in the translation unit, such as `<fstream>` for
  `std::ifstream` and `std::ofstream`, or `<filesystem>` for
  `std::filesystem::path`, `file_status`, and filesystem operations. Then
  confirm the same file still compiles with the configured CMake target.
- Do not modify valid C++ declarations merely to silence an IntelliSense-only
  diagnostic when the configured compiler accepts the translation unit. Correct
  the editor include configuration and confirm the result with a clean build.
- Validate `../.vscode/c_cpp_properties.json` after editing it. Do not leave stale,
  duplicated, or nonexistent module paths in the configuration.
- If corrected paths do not clear stale diagnostics, run
  `C/C++: Reset IntelliSense Database` from the VS Code Command Palette and then
  reload the VS Code window.
- A new module is not complete until its sources compile and its public types are
  resolved without IntelliSense errors when the `MyPiCarX` parent directory is
  opened as the VS Code workspace.

## Class design and ownership

- Prefer small concrete, non-virtual objects unless runtime polymorphism is a
  real requirement.
- Continue the I2C dependency-injection pattern: a context pointer plus C-style
  callbacks separates hardware-independent behavior from the backend.
- I2C consumers that acquire device data use the shared read callback. A raw read
  returns bytes in bus order and must reject a zero-byte request at the interface boundary.
- Continue the GPIO dependency-injection pattern: one `XWalkGpio` stores a
  non-owning backend context and validated callback set. A Linux GPIO backend is
  dedicated to one GPIO object because it owns that line's descriptor and event
  worker. Create the backend before the GPIO object in the composition root.
- Continue the SPI dependency-injection pattern: one `XWalkSpi` stores a
  non-owning backend context and required full-duplex callback. Keep device,
  mode, clock, bits-per-word, and chip-select selection in deployment and the
  Linux owner. Bound one transfer to 256 bytes and require equal request and
  response lengths.
- Compose the CLI SPI command through its dedicated SPI-only boot mode. It must
  not detect or reset the Robot HAT, claim GPIO, construct actuators, or start
  audio, camera, speech, or model services.
- Compose the CLI Doctor command through a dedicated passive boot mode. It may
  open configured device descriptors, inspect metadata, read firmware, and
  sample battery ADC A4. It must not request GPIO lines, reset the MCU,
  construct actuators, transfer SPI data, enable audio, capture media, or
  contact a model endpoint.
- GPIO interrupt application contexts are non-owning. They must outlive the
  registration, and handlers invoked by a backend worker must not throw or block
  indefinitely. Cancel the registration before destroying the handler context.
- Motor drivers receive caller-created PWM and GPIO objects by reference and
  store non-owning pointers. Select PWM-and-direction or dual-PWM mode through
  typed constructor dependencies rather than an unchecked numeric mode value.
- Keep ordinary `stop()` failure-reporting, but implement `stopSafely()` as a
  non-throwing best-effort boundary. A single motor independently attempts every
  speed PWM output, paired motors independently attempt both motors, and both
  destructors invoke their safe-stop operation without releasing dependencies.
  Route these attempts through explicit Boolean I2C and PWM status operations;
  do not intercept exceptions to implement fail-safe cleanup.
- Bound final PiCar-X motor PWM magnitude through the deployment key
  `picarx_max_motor_output_percent`, using 20 percent by default for first-run
  safety. Apply the bound after compatibility scaling and calibration. Do not
  permit a limit above 20 percent until the calibration workflow persists
  `picarx_calibration_verified = true` after raised-wheel motor-direction,
  steering-center, and motor-balance checks.
- Persist signed motor balance through `picarx_motor_speed_calibration` in the
  range -100 through 100 percentage points. Positive values reduce the left
  side and negative values reduce the right side. Persist confirmed stationary
  grayscale and cliff samples through `line_reference` and `cliff_reference`.
- Latch PiCar-X emergency stop before attempting safe shutdown. Suppress later
  motor and servo commands until the application explicitly starts a fresh
  operation, and arm one `XWalkPicarxSafetyGuard` for every non-SPI CLI command.
- Keep persistent motor role and reversal configuration outside the motor
  objects. Pass configuration values into `XWalkMotors`, and let
  `XWalkConfigStore` own filesystem access when persistence is required.
- Create `XWalkConfigStore` in `main()` or the application composition root.
  Load strings there, convert them to validated typed configuration, and pass
  those values into hardware consumers. Hardware drivers must not open
  configuration files.
- Keep configuration paths as owned standard-library values. A configuration
  store may contain its own mutex by value because a mutex is synchronization
  state, not an injected project-class dependency.
- Validate configuration names as non-empty, single-line keys without `=` or
  leading or trailing whitespace. Validate persisted values as single-line
  text. Preserve comments and unrelated entries when updating a file.
- Commit configuration updates through a replacement file in the same
  directory. Serialize operations performed through one store instance, and
  require external synchronization when separate instances or processes access
  the same file.
- Keep file permission and ownership policy outside `XWalkConfigStore`. Do not
  construct `sudo chmod` or `sudo chown` shell commands from application input.
- Use `XWalkConfig` for section-aware files and retain `XWalkConfigStore` for
  the flat key-value format. Provide
  both classes through the combined `xWalkConfig` module, but keep their headers,
  source files, tests, file grammars, and persistence contracts separate.
- Create `XWalkConfig` in `main()` or the application composition root. Its
  section and option containers are value-like state and remain stored by value;
  do not inject filesystem configuration directly into hardware drivers.
- Represent options before the first header through the empty default-section
  name. Validate named sections as non-empty trimmed text without brackets or
  line terminators, and validate option names as non-empty trimmed text without
  `=`, brackets, or line terminators.
- Keep `XWalkConfig` mutations explicit: `set()` and a missing-option `get()`
  update the memory image, `write()` persists it, and `read()` discards unsaved
  changes by reloading the file. Preserve comments, blank lines, unrelated
  options, and unrelated text during persistence.
- Create missing section-aware files with optional `# ` description comments.
  Reject malformed section headers conservatively, and use an empty string as
  the empty typed C++ default.
- Commit both configuration formats through a same-directory replacement file
  and preserve existing permission bits. Do not expose mode or owner
  constructor arguments and never execute `chmod`, `chown`, or `sudo` through a
  shell. Deployment tooling owns file permission and ownership policy.
- Keep `XWalkBoardControl`, `XWalkDevice`, and `XWalkFirmwareInfo` in the
  combined `xWalkBoardControl` module. Retain separate class headers, source
  files, tests, and responsibilities rather than combining them into one class.
- Keep `XWalkDevice` limited to Robot HAT device discovery. It owns a configurable
  device-tree root and value-like discovery result; it must not create GPIO, LED,
  speaker, motor, ADC, or board-control objects.
- Scan only direct children whose names contain `hat`, require a regular UUID
  property, and recognize Robot HAT v5 through its documented UUID. Publish
  detected metadata only after every selected property is read and validated.
- Preserve Robot HAT v4 board values as an explicit deployment profile: speaker
  enable GPIO 20 and motor mode 1. A recognized v5 board uses GPIO 12 and motor
  mode 2. Keep the separate `detected` state so defaults do not imply discovery,
  and never infer v4 merely because the supported v5 UUID is absent.
- Parse product ID and version as bounded unsigned 32-bit hexadecimal values.
  Accept one through eight digits with an optional `0x` prefix, and reject
  malformed or oversized properties without partially replacing prior state.
- Preserve the property-trimming distinction conservatively. UUID,
  product ID, and version may remove one terminal null byte; product and vendor
  data remain complete. Never remove a non-null final byte.
- Keep `XWalkBoardControl` limited to GPIO output, MCU reset, battery voltage,
  and speaker power. Do not add terminal formatting, process
  execution, network lookup, user lookup, error redirection, or lazy caching.
- Create the MCU-reset GPIO, board-selected speaker-enable GPIO, I2C interface,
  and A4 ADC in `main()`. Pass them to `XWalkBoardControl` by reference and
  store only non-owning pointers. Every dependency must outlive the controller.
- Validate hardware roles during construction. MCU reset uses GPIO 5, battery
  sensing uses ADC channel A4, and speaker enable uses GPIO 20 for Robot HAT v4
  or GPIO 12 for Robot HAT v5. Obtain the selected speaker pin from
  `XWalkDevice::information()` in the application composition root.
- Preserve the reset sequence as a logical low level for 10 milliseconds
  followed by a logical high level for 10 milliseconds. Document that a GPIO
  failure after assertion may leave the MCU held in reset.
- Calculate battery potential by naming the ADC voltage, divider ratio, and
  resulting battery voltage separately. Apply the hardware divider ratio of
  three without combining conversion and multiplication in one expression.
- Use a required, context-based callback for speaker priming. Enable the GPIO
  before requesting 500 milliseconds
  of priming output. A failed callback leaves speaker power enabled.
- Do not invoke `pinctrl`, `raspi-gpio`, `play`, `sudo`, or another shell command
  from board-control production code. Use the injected GPIO and audio contracts.
- Keep non-hardware platform behavior in the separate `xWalkUtils` module. Do not
  move terminal, command, executable, network,
  username, standard-error, or lazy-cache behavior into `xWalkBoardControl`.
- Inject terminal output, volume control, command execution, executable lookup,
  IPv4 lookup, username lookup, monotonic time, and standard-error redirection.
  The hardware-independent module must not call `sudo`, `amixer`, `which`, a
  shell, environment lookup, network command, or descriptor API directly.
- Treat command text as security-sensitive. Forward it without modification so
  compatibility is explicit, but require the application backend to validate
  all untrusted input before invoking a process or shell.
- Preserve volume clamping to zero through one hundred percent. Preserve
  ordered IPv4 lookup with an empty string for not found, and consolidate the
  three executable checks behind one typed backend operation.
- Implement `ignore_stderr` as the scope-bound `XWalkStderrGuard`. Redirect in
  construction, restore in destruction, and retain no ownership of the platform
  callback context. A throwing restore callback terminates the process.
- Keep `XWalkLazyReader<ValueType>` callback-based and generic. Acquire on the
  first read and refresh only when elapsed monotonic time is strictly greater
  than the configured millisecond interval. Cache only value-like data or a
  non-owning pointer, never a project component object by value. Require
  external serialization.
- Preserve linear-map extrapolation, but reject non-finite inputs and a zero-width
  input range instead of permitting undefined division behavior.
- Keep `XWalkFirmwareInfo` limited to firmware reads and static compatibility
  metadata. Do not add global device objects, command-line
  parsing, console output, process termination, or unrelated package imports.
- Create `XWalkI2c` in `main()`, pass it to `XWalkFirmwareInfo` by reference,
  and store only a non-owning pointer. The I2C object and its backend must outlive
  the firmware-information reader.
- Probe firmware addresses `0x14` and `0x15` in that order and retain the first
  responding address. Reject the no-device case instead of falling back to a
  known-unresponsive `0x14` address.
- Read exactly three bytes beginning at register `0x05` through one atomic
  `XWalkI2c::readRegister()` operation. Reject missing register-read support or
  a response that is not exactly three bytes before indexing it.
- Represent firmware version components as a documented structure containing
  unsigned major, minor, and patch bytes. Format each component through a shared
  Common conversion function and join them as `major.minor.patch`.
- Preserve Robot HAT compatibility version `2.5.5` as static non-owning metadata.
  Do not confuse this package version with the firmware bytes read from hardware.
- Keep `XWalkTrace` limited to severity selection and diagnostic forwarding. Do
  not introduce a global
  logger, hidden console output, worker thread, or operating-system dependency.
- Preserve the severity ordering from zero through four: critical, error,
  warning, info, and debug. Accept exact lowercase names, use warning by default,
  and reject numeric values or names outside that contract.
- Inject a non-null synchronous trace-output callback and an optional non-owning
  context. The application composition root owns the output backend, timestamp
  source, and final record formatting. The callback must not retain message views.
- Forward a record when its numeric severity is no greater than the configured
  threshold. Report a threshold change at debug severity, so it remains filtered
  unless the newly selected threshold accepts debug records.
- Require external synchronization when several tasks or interrupt contexts use
  one trace object. Do not imply interrupt safety for an arbitrary output callback.
- Keep speech recognition and speech synthesis in the combined `xWalkGPT`
  module. Retain `XWalkSpeechToText` and `XWalkTextToSpeech` as separate classes,
  headers, source files, and test executables within that module so each class
  preserves one responsibility and each file contains only one class definition.
- Keep `XWalkTextToSpeech` limited to the common behavior required by Piper,
  Pico2Wave, Espeak, OpenAI TTS, and EdgeTTS providers. Consolidate those
  providers into one coordinator rather than duplicating five
  classes whose only Robot HAT responsibility is speaker activation.
- Create `XWalkBoardControl` and the speech backend in `main()`. Pass board
  control by reference, store only a non-owning pointer, and inject speech output
  through a non-null synchronous callback with an optional non-owning context.
- Validate the speech callback before changing hardware. Enable and prime speaker
  output during construction, propagate board-control failures, and do not disable
  shared speaker power automatically during destruction.
- Forward speech text without modifying its encoding, language, content, or empty
  state. The backend owns its text limits, model, voice, synthesis, playback,
  process, filesystem, and network policy and must not retain the text view.
- Do not link the hardware-independent coordinator directly to Piper, Pico2Wave,
  Espeak, OpenAI, EdgeTTS, ONNX, process-execution, or network libraries. Supply
  those capabilities through an application-owned platform backend.
- Serialize access externally when a board controller or speech backend is shared
  between tasks. Do not claim real-time or interrupt safety for an arbitrary TTS
  backend, because synthesis, playback, and network operations can block.
- Keep `XWalkVoiceAssistant` limited to synchronous orchestration of caller-created
  `XWalkSpeechToText`, `XWalkLanguageModel`, and `XWalkTextToSpeech` objects.
  Create those objects in `main()`, pass them by reference, and store non-owning
  pointers that remain non-null and valid for the coordinator's full lifetime.
- Preserve one-round flow as listen, skip silence, prompt, optionally parse, speak
  non-empty output, and report completion. Optional null lifecycle callbacks are
  no-ops, and a null response parser returns an unmodified owned response copy.
- Compose the target pipeline in this order: shared ALSA audio, ALSA
  speech-to-text adapter and coordinator, Ollama provider and coordinator, ALSA
  text-to-speech adapter and coordinator, then `XWalkVoiceAssistant`. Preserve
  reverse destruction order so the assistant is destroyed first.
- Keep the full-stack hardware smoke test explicit and opt-in. Require capture,
  playback, mixer, Ollama endpoint, model, approved prompt, and response-fixture
  selection; never emit captured PCM, prompts, responses, fixtures, or credentials.
- Keep wake detection, trigger policies, keyboard polling, camera capture,
  continuous loops, streaming, threads, and scheduling in the application or an
  injected backend. Require external serialization because injected operations
  may block and are not assumed to be thread-safe or interrupt-safe.
- Keep `XWalkSpeechToText` limited to synchronous readiness queries, bounded
  microphone listening, audio-file transcription, and explicit stop control.
  Do not infer platform ownership or hidden setup behavior in this coordinator.
- Inject one complete callback table for speech readiness, microphone listening,
  file transcription, and stop control. Reject a table containing any null
  callback during construction; the optional context pointer is non-owning and
  may be null when the backend does not require state.
- Accept listen timeouts from 1 through 300,000 milliseconds and use 30,000
  milliseconds by default. Reject an empty audio-file path before invoking the
  backend, but preserve an empty transcript because it can represent silence or
  unrecognized speech rather than an interface failure.
- Keep microphone enumeration, capture resources, speech models, language
  selection, streaming partial results, wake-word threads, filesystem access,
  process execution, and network access inside the application-owned backend.
- Call the speech stop callback during destruction. A throwing callback
  terminates the process because destructors do not install exception handlers.
  Backends must tolerate stop requests while idle and repeated stop requests.
- Serialize access externally when a speech-to-text object is shared between
  tasks. Listening and transcription callbacks may block and must not be called
  from interrupt context unless a backend explicitly proves that use safe.
- Keep real microphone ownership in `XWalkSpeechToTextAlsa`. Capture 16 kHz mono
  signed-16 PCM, limit one ALSA read to 1,024 frames, apply the coordinator's
  bounded listen timeout, and close the capture handle before recognition.
- Inject one complete recognizer operation table into the ALSA adapter. The
  application selects one local or remote provider and owns its model, process
  or HTTP transport, credentials, language policy, and provider-specific data.
- Make capture recovery and cancellation bounded. A stop request may interrupt
  capture or recognition, repeated cancellation must be tolerated, and normal
  diagnostics must never contain microphone PCM or credentials.
- Implement synthesis playback in `XWalkTextToSpeechAlsa`, which observes one
  caller-owned `XWalkAudioAlsa`. The audio owner outlives the adapter, and the
  adapter outlives `XWalkTextToSpeech`.
- Inject one non-null synthesis operation returning at most 16 MiB of interleaved
  signed sixteen-bit little-endian PCM. Require positive sample rate, one through
  eight channels, complete frames, and no more than 1,024 frames per ALSA write.
- Keep models, voices, credentials, process or HTTP transport, provider formats,
  and generated-file policy in the selected application-owned synthesis provider.
  Empty provider PCM completes without opening a stream or changing mixer volume.
- Keep `XWalkLanguageModel` limited to provider-neutral system instructions,
  welcome text, retained-message limits, conversation messages, and synchronous
  final-response prompting. Consolidate general, Deepseek, Grok,
  Doubao, Gemini, Qwen, OpenAI, and Ollama classes behind one coordinator.
- Inject one complete callback table for instructions, welcome text, message
  limits, message insertion, and prompting. Reject a table containing any null
  callback during construction; the optional context pointer is non-owning and
  may be null when the backend does not require state.
- Represent conversation participants with `XWalkLanguageModelRole`. Preserve
  system, user, and assistant roles without forwarding unchecked role strings.
- Use 20 retained messages by default and reject zero. Leave any provider- or
  deployment-specific upper limit to the backend instead of inventing one in
  the hardware-independent interface.
- Preserve empty instructions, welcome text, message content, prompt content,
  and final responses. Use an empty image-path view to represent no attached
  image and keep image validation and encoding inside the backend.
- Keep credentials, authorization, provider URLs, model selection, HTTP, JSON,
  process execution, image encoding, history storage, history truncation,
  streaming delivery, and provider errors inside the application-owned backend.
  Never compile credentials into a module or write them to diagnostic output.
- Serialize access externally when a language-model object is shared between
  tasks. Prompt callbacks may block on inference, a process, or network I/O and
  must not be used from interrupt context.
- Use `XWalkLanguageModelOllama` as the first concrete provider. It owns bounded
  history, Ollama JSON conversion, optional base64 image conversion, and real
  libcurl HTTP transport while remaining separate from the neutral coordinator.
- Limit Ollama state to 200 messages, each text value to 256 KiB, one raw image
  to 4 MiB, one JSON request to 8 MiB, and one HTTP response to 1 MiB. Bound
  request timeout to 1 through 300,000 milliseconds and default to 120,000.
- Send non-streaming `/api/chat` requests and retain a prompt and final response
  only after successful parsing. The built-in transport sends no authorization
  header; deployment owns endpoint protection, TLS, proxies, and credentials.
- Never emit Ollama requests, responses, images, prompts, or credentials through
  normal diagnostics. Require explicit endpoint, model, and prompt selection in
  the opt-in provider smoke test.
- Validate both left and right commands before changing either motor. A failed
  range or finite-value check must not leave only one motor updated.
- Do not embed one project class as a by-value object or reference data member
  inside another project class. Store an injected class dependency as a
  non-owning pointer.
- Accept every required class dependency as a constructor reference, then store
  its address in the non-owning pointer member. The constructor reference makes
  null invalid at the API boundary; the pointer supports the project's explicit
  dependency representation inside the class.
- Create concrete backends, interfaces, shared state, and consumers in `main()`
  or the equivalent test entry point. Construct dependencies before consumers,
  pass them by reference, and ensure consumers are destroyed first.
- Keep ownership outside dependent classes. A dependent class must not allocate,
  delete, or otherwise release its injected pointer.
- Register variable-count Robot servo dependencies individually through
  `XWalkRobot::addServo(XWalkServo&)`. Store them as bounded non-owning pointers,
  complete registration before `initialize()`, and prohibit registration after
  initialization. The application creates PWM and Servo objects before the
  Robot and destroys the Robot first.
- Keep articulated-robot configuration separate from hardware construction.
  Inject `XWalkConfigStore` by reference, derive a robot-specific offset key,
  and reject malformed or incorrectly sized persisted offset lists.
- Limit coordinated Robot motion to a bounded servo count and validate every
  complete frame before changing hardware. Interpolate all registered servos
  using one shared completion ratio so a failed frame cannot partially change
  only a subset because of a length or finite-value error.
- Construct ultrasonic trigger and echo GPIO objects in the application, pass
  them to `XWalkUltrasonic` by reference, and store only non-owning pointers.
  Configure the trigger as an output and the echo as a pull-down input without
  constructing replacement GPIO objects inside the sensor.
- Use a monotonic microsecond clock for ultrasonic pulse timing. Generate the
  inactive settling interval and active trigger pulse explicitly, apply one
  bounded timeout to both echo transitions, and leave the trigger inactive.
- Preserve the ultrasonic result contract: return `-1.0` for timeout,
  retry timeout results only, and return `-2.0` without retry when no complete
  pulse can be measured. Document all successful values in centimeters.
- Construct the three line-tracker ADC channels in the application, pass them
  by reference in left, middle, and right order, and store bounded non-owning
  pointers. Use fixed three-element project types for readings, statuses,
  slopes, and offsets so incomplete channel data is not representable.
- Validate line-tracker calibration coefficients before storing them. Perform
  calibration arithmetic in `float64`, round only at the documented boundary,
  and reject a result before conversion when it exceeds the signed count range.
- Preserve strict tracker comparisons: a cliff value is below its
  threshold, a line requires spread greater than the configured difference,
  and a grayscale value equal to its reference is classified as black.
- This rule applies to project component classes. Value-like standard-library
  state such as containers, mutexes, optionals, and fixed arrays remains stored
  by value.
- State ownership and lifetime through construction. Do not hide opening of
  hardware devices inside hardware-independent objects.
- Delete copy and move operations for objects whose callbacks, references,
  mutexes, or OS handles make relocation unsafe.
- Use RAII for resources. Acquire a Linux file descriptor in construction,
  validate it immediately, and close it in destruction.
- Keep every class data member private. A `private` access section must contain
  variables only; do not declare constructors, destructors, operators, static
  helpers, or other member functions there.
- Declare every non-public member function in a `protected` access section,
  including validation, conversion, lifecycle, and hardware helper functions.
  This project convention applies even when no derived class currently uses the
  function. Do not change a function's implementation merely to change access.
- Mark getters `const`; mark non-throwing accessors `noexcept`.
- Use function-local static state when project-wide shared state needs safe C++11
  initialization, as in the default PWM timer state.

## Validation, errors, and numeric behavior

- Do not use `try` or `catch` statements in workspace production code or tests.
  Functions throw through the project exception macros and ordinary callers let
  those failures escape. Provide cleanup through scope-bound guards and
  destructors so stack cleanup requests actuator shutdown without exception
  interception.
- When an operation must continue after one failed backend attempt, provide an
  explicit non-throwing Boolean status operation. The fail-safe I2C/PWM write
  path and the SelfDrive delay callback use this pattern. Callers must check the
  status, attempt every independent safety output, and latch failure for the
  controlling thread.
- Callbacks invoked by unrelated destructors or `noexcept` workers remain
  non-throwing; a violation terminates the process.
- Verify expected validation failures in host tests with
  `xwalk::hal::test::expectFailure()`, which isolates the operation in a child
  process and asserts that it does not complete successfully.

For hardware formulas, do not combine multiple conversions, multiplications,
or divisions in one expression. First convert every input to the intended
calculation type, then name each derived quantity:

```cpp
const float64 pwmClockHz = static_cast<float64>(XHAL_RPI5CAR_PWM_CLOCK_HZ);
const float64 servoFrequencyHz = static_cast<float64>(XHAL_RPI5CAR_SERVO_FREQUENCY_HZ);
const float64 servoPeriod = static_cast<float64>(XHAL_RPI5CAR_SERVO_PERIOD);
const float64 servoTimerDivisor = servoFrequencyHz * servoPeriod;
const float64 prescaler = pwmClockHz / servoTimerDivisor;
```

This pattern applies to clock, frequency, period, prescaler, duty-cycle, timing,
unit-conversion, and register-scaling calculations. Intermediate names must
include units when applicable and must describe the physical or mathematical
meaning rather than the order of evaluation. Do not use names such as `temp`,
`value1`, or `result2`.

- Validate inputs at the public boundary or before acquiring a resource.
- Reject invalid states with the exception macros in
  `xHal_Rpi5CarExceptions.h`:
  - invalid form, null callback, or impossible argument: invalid argument;
  - numeric or container limit violation: out of range;
  - failed hardware operation after retries: runtime error.
- Make exception messages concise and specific. Include the subsystem and the
  violated constraint when useful.
- Check floating-point inputs with `XHAL_IS_FINITE` before conversion or range
  comparison.
- Preserve externally visible behavior intentionally. Document a compatibility
  choice where the C++ result might otherwise look accidental.
- Use `XWalkI2c::readRegister()` for register-addressed sensor acquisition.
  Hardware backends perform register selection and data transfer atomically
  under their bus lock; do not compose a register write and sequential read in
  separate unlocked calls.
- Preserve the ADXL345 discarded first sample for every returned
  axis value. Decode the second sample as a signed little-endian 16-bit count
  and scale it by 256 counts per unit of standard gravity.
- Preserve the RGB LED port's component-array, packed `0xRRGGBB`, and
  hexadecimal-text inputs. Strip surrounding `#` characters from text before
  requiring exactly six hexadecimal digits.
- Keep `XWalkLed` and `XWalkRgbLed` in the combined `xWalkLed` submodule. Build,
  test, document, and configure both through the `XWALK_LED_*` options; retain
  `xWalkRgbLed` only as a CMake target alias for compatible consumer linkage.
- Convert each RGB component independently to a PWM percentage. Invert the
  eight-bit component before scaling for common-anode wiring; do not invert it
  for common-cathode wiring.
- Construct the red, green, and blue PWM objects in the application entry point
  and pass them by reference to `XWalkRgbLed`. The controller stores only
  non-owning pointers, and the PWM objects must outlive it.
- Represent active and passive buzzers with separate `XWalkBuzzer` constructor
  overloads. Accept an `XWalkGpio&` for an active buzzer or an `XWalkPwm&` for a
  passive buzzer, and store the selected dependency as a non-owning pointer.
- Place every buzzer into the inactive state during construction. Use logical
  GPIO polarity for active buzzers and zero or 50 percent PWM duty cycles for
  passive-buzzer off or on operations respectively.
- Permit frequency selection and playback only for passive buzzers. Playback
  without duration remains active; finite playback divides its total seconds
  into equal sounding and silent microsecond intervals.
- Validate playback duration before changing frequency or output state. Reject
  non-finite, negative, and unrepresentable duration values.
- Construct an LED GPIO in the application entry point and pass it by reference
  to `XWalkLed`. The LED controller stores a non-owning pointer and must not
  release or reconfigure ownership of the caller's GPIO.
- Stop and join an existing LED worker before direct on, off, toggle, close, or
  replacement-blink operations. Normal worker shutdown leaves the LED inactive.
- Keep LED blink continuation and output state atomic. Backend operations on the
  `noexcept` worker must not throw; a violation terminates the process.
- Divide each LED blink sequence into two transitions per requested cycle,
  followed by the inactive pause. Use bounded delay chunks so stop latency does
  not exceed the documented worker polling interval during normal scheduling.
- Call mutating `XWalkLed` operations from one controlling execution context.
  Only its atomic state accessors are intended for concurrent observation.
- Construct the USER GPIO as a pull-up input in the application entry point and
  pass it by reference to `XWalkUserButton`. The monitor stores a non-owning
  pointer and must not close or release the caller-owned GPIO.
- Poll the active-low user button at the shared 50-millisecond interval. Record
  press timing with the common monotonic clock and report durations in seconds.
- Treat the first GPIO sample after `start()` as the transition baseline and do
  not dispatch an event for that sample, preserving the established behavior.
- Clamp the shared user-button long-press threshold to 2.0 through 5.0 seconds.
  Capture the configured threshold and whether long callbacks exist when a
  press begins; later callback configuration does not alter the active press.
- Dispatch button callbacks outside the state mutex on the monitoring worker.
  Callback contexts are non-owning and must remain valid until cleared and the
  worker is joined. Callbacks must not stop or destroy their own monitor.
- Require GPIO and callback operations on the user-button worker to be
  non-throwing. A violation terminates the process.
- Keep `XWalkMusic` independent of `pygame`, `pyaudio`, and platform audio
  libraries. The application creates an audio backend and passes one non-owning
  context plus a complete `XWalkMusicCallbacks` operation table. Validate the
  full table before invoking the output-enable callback during construction.
- Keep sound-file interpretation, asynchronous playback, streamed transport,
  and physical PCM output inside the injected music backend. Keep time
  signatures, tempo, key displacement, note-frequency calculations, volume
  normalization, and PCM sample generation in the hardware-independent class.
- Preserve the music tone-data contract deliberately: halve the requested
  duration, generate truncated signed 16-bit mono sine frames at 44,100 Hertz,
  then append `frameCount % sampleRate` silent frames. Emit explicit
  little-endian bytes and document this compatibility behavior.
- Do not disable or release the injected music backend during destruction. The
  backend is caller-owned, and speaker activation has no matching destruction
  operation.
- Implement Linux music callbacks in `XWalkMusicAlsa`, which observes one
  caller-owned `XWalkAudioAlsa`. The audio owner outlives the adapter, and the
  adapter outlives `XWalkMusic`; no adapter releases the shared audio owner.
- Keep file decoding injectable for device-free host tests. The built-in music
  decoder accepts non-empty uncompressed 16-bit PCM RIFF/WAVE data with one
  through eight channels and rejects incomplete frames and malformed chunks.
- Provide compressed Music formats through the optional stateless libsndfile
  decoder operation. Convert decoded samples explicitly to signed 16-bit
  little-endian interleaved PCM, retain at most 64 MiB per operation, and keep
  libsndfile out of the hardware-independent Music target.
- Retain at most one background sound worker and one streamed-music worker.
  Write at most 1,024 frames per ALSA operation, observe pause and stop between
  writes, join replaced workers, and close every temporary PCM stream.
- Keep `XWalkSpeaker` independent of `pyaudio`, `soundfile`, `librosa`, and
  NumPy. Inject one non-owning backend context with callbacks for output power,
  decoding, stream opening, bounded frame writing, stream closing, and unique
  task identifiers.
- Bound one speaker controller to eight retained tasks and each backend write to
  1,024 complete frames. Decode before occupying a task slot, require complete
  interleaved frames with finite samples, and reject zero sample rates or
  channel counts.
- Treat backend stream handles as opaque, nullable values until open succeeds.
  The backend owns each opened stream; the speaker controller must close it
  exactly once without deleting or casting the handle.
- Call speaker task mutations from one controlling execution context. Coordinate
  playback workers through the state mutex. Backend operations on `noexcept`
  workers and during destruction must not throw; a violation terminates the
  process.
- Preserve the speaker format groups: WAV, FLAC, and OGG use the SoundFile
  decoder family; MP3, M4A, AAC, and WMA use the compressed-audio family.
- Implement Linux Speaker callbacks in `XWalkSpeakerAlsa`, which observes one
  caller-owned `XWalkAudioAlsa`. The audio owner outlives the adapter, and the
  adapter outlives the controller and all controller-owned workers.
- Bound the built-in PCM WAVE decoder to 16 MiB of input and 2,000,000 decoded
  interleaved samples per task. Require optional FLAC, OGG, or compressed codec
  libraries behind the injected decoder operation instead of linking the core.
- Convert normalized `float64` samples to explicit float32 little-endian PCM at
  the adapter boundary. Preserve the controller's 1,024-frame write limit and
  close every shared-audio stream exactly once through the adapter callback.
- Keep reusable Linux PCM and mixer ownership in `xWalkAudio`. Do not make
  Music depend on Speaker or Speaker depend on Music to obtain audio output.
- Create `XWalkAudioAlsa` before every audio adapter and consumer. Its injected
  ALSA-operation context is non-owning and must outlive the backend.
- Select PCM device, mixer device, and simple-element names through deployment
  configuration. Do not assume ALSA card zero; `default` and `PCM` are only
  configurable defaults.
- Retain at most eight PCM playback handles. Negotiate interleaved signed
  sixteen-bit little-endian or float32 little-endian samples, positive rate,
  one through eight channels, a period of at most 4,096 frames, and positive
  latency before publishing a stream handle.
- Bound each PCM write to one configured period, complete short writes, and
  allow at most three ALSA recovery attempts. Destruction closes every retained
  stream before the persistent mixer, and close callbacks must not throw.
- Use named register, address, clock, retry, and protocol constants from
  `xHal_Rpi5CarCommon.h`. Do not scatter unexplained hardware literals through
  production code.
- Place public hardware constants shared by module declarations, sources, or
  tests in `xWalkCommon/include/xHal_Rpi5CarCommon.h`. Module headers must not
  redeclare those constants. This includes ultrasonic sound-speed, timing,
  attempt-count, timeout-result, and invalid-pulse-result macros.
- Make byte order explicit when encoding register data.

## Concurrency and hardware I/O

- Protect a shared device handle or shared timer state with the project
  `mutexhandle` and `mutexlock` types.
- Keep lock scope large enough to make a complete hardware transaction atomic,
  including address selection and the following operation.
- Keep platform calls qualified with the global scope operator, for example
  `::open`, `::ioctl`, and `::close`.
- Bound retry loops with the configured retry count. Return immediately on
  success and throw a runtime error only after all write attempts fail.
- Keep hardware code optional, Linux-gated, and separate from host logic.

## Tests

- Add or update host tests for every behavior change. Host tests must use
  in-memory callbacks and must not open `/dev/i2c-*`.
- Configuration-store host tests may use the real filesystem only below their
  module-local CMake binary directory. They must not write to `/opt`, a user
  directory, or a deployed robot configuration path.
- Use small test functions named `test<Behavior>` and plain `assert` checks,
  matching the existing lightweight test executables.
- Keep reusable fake hardware in test helper classes such as
  `XWalkPwmTestI2c`. Record the interaction needed for assertions: probes,
  addresses, registers, and payloads.
- Test public results and observable bus traffic, including register selection,
  byte order, state shared between channels, and validation failures.
- Add a selector in the test main and a separately named CTest entry when adding
  a PWM or Servo test scenario.
- Label simulated tests `host` and physical-device tests `hardware`.
- Keep all hardware-test build options `OFF` by default. Never run a hardware
  test as part of ordinary verification or without the correct Raspberry Pi and
  Robot HAT safety setup.
- Compile-check the hardware targets after changes to `xWalkCommon`, `xWalkI2c`, `xWalkSpi`,
  `xWalkAudio`, `xWalkMusic`, `xWalkSpeaker`, `xWalkPwm`, `xWalkServo`, `xWalkGpio`,
  `xWalkUltrasonic`,
  `xWalkLineTracker`, `xWalkAdxl345`, `xWalkBuzzer`, `xWalkLed`,
  `xWalkUserButton`, `xWalkBoardControl`, `xWalkTrace`, `xWalkGPT`,
  `xWalkLanguageModel`, `xWalkVoiceAssistant`, `xWalkUtils`, their public
  headers, or their CMake configuration.
  Compilation is safe on a Linux host and does not access the physical I2C
  device.
- Use `ctest -N -L hardware` only to list registered hardware tests during a
  compile check. Do not omit `-N`, because doing so would execute the tests.
- In the aggregate host build directory, run plain `ctest` to execute every
  registered host and unit test.
- In the aggregate RPI build directory, run plain `ctest` only on the connected
  target after the hardware setup has passed its safety review. This executes
  every registered hardware test.

## CMake conventions

- Require at least CMake 3.16 and declare `LANGUAGES CXX` for concrete modules.
- Use target-based commands: `target_include_directories`,
  `target_compile_features`, `target_link_libraries`, and
  `target_compile_options`.
- Use `PUBLIC`, `PRIVATE`, and `INTERFACE` deliberately; do not introduce global
  include directories or compiler flags.
- Let a module add an adjacent dependency only when its target does not already
  exist. Give the dependency a dedicated binary directory.
- Resolve the workspace-level `xWalkCommon` source from `../../xWalkCommon`
  when configuring an individual HAL submodule. The `xWalkHal` aggregate imports
  it from `../xWalkCommon` into its own binary tree.
- Name feature options `XWALK_<MODULE>_BUILD_<MODE>_TESTS` and default them to
  `OFF`.
- Register tests with descriptive stable names and labels.
- Keep every fenced shell-command example on one physical line. Do not use a trailing backslash to wrap CMake
  configure, build, or CTest commands. These command lines are exempt from the 115-character limit.
- Use module-local build directories such as `xWalkServo/build-host` and
  `xWalkServo/build-rpi` in documented commands. Do not require `/tmp` paths.
- Use the workspace root and its presets for release-wide sanity, Clang-Tidy,
  sanitizer, coverage, staging, and package builds. Keep those outputs in the
  corresponding `build-host/<purpose>` or `build-rpi/cmake` directory and use
  `cmake --fresh` when changing source trees or build modes.
- Install mutable deployment configuration separately from read-only binaries,
  media, and documentation. Default the active PiCar-X configuration to an
  explicitly writable system location rather than a source-tree path.
- Use `GNUInstallDirs`, `/usr` as the package prefix, and `DESTDIR` for local
  staging. Install immutable resources under the configured data directory,
  administrator configuration under `/etc/xwalk`, and runtime state under
  `/var/lib/xwalk`, `/var/cache/xwalk`, or `/run/xwalk`.
- Keep Raspberry Pi setup idempotent and dry-run-first. Report privileged
  changes before applying them, require an explicit Robot HAT profile, and do
  not infer or install a board overlay from failed discovery.
- Grant device permissions through standard operating-system groups and exact
  configured I2C, GPIO, and SPI node matches. Do not add broad device wildcards.

## Command-line application conventions

- Keep the standalone `xWalkCLI` aggregate beside `xWalkAgent` and `xWalkHal`.
  Keep its `xWalkController` module under `xWalkCLI`. Do not place either
  `xWalkCLI` or `xWalkController` inside the standalone `xWalkAgent` tree.
- Let `xWalkCLI` import the sibling `xWalkAgent` aggregate. Agent owns
  `xWalkPicarx`, `xWalkLineTracking`, `xWalkSelfDrive`, and `xWalkBoot`; Controller links
  those targets without duplicating their source directories.
- Keep `XWALK_CLI_BUILD_HOST` and `XWALK_CLI_BUILD_RPI` mutually exclusive and
  `OFF` by default. Map them to the Controller test options and use separate
  `build-host` and `build-rpi` directories. Label the RPI test `hardware` and
  do not execute it on Ubuntu.
- Use `xwalk-picarx-control <command> [options]` as the stable command shape.
- Accept named options as `--name value`, `--name=value`, or `name=value`.
  Accept one flat JSON object through `--config FILE.json`; direct options take
  precedence over JSON values.
- Keep JSON parsing bounded to scalar configuration. Reject nested arrays and
  objects rather than silently ignoring unsupported configuration.
- Execute a command only when the CLI owns a complete safe composition. Return
  a distinct backend-unavailable status for optional services such as audio.
- Enter RPI backend composition through one automatic `XWalkBootRpi` object.
  Invoke its application callback at most once, consume a failed run attempt,
  and retain the stack-owned backend graph until command completion. Return help
  before constructing the boot object so discovery claims no platform resource.
- Boot every backend required by the selected command exactly once. Do not
  initialize HAL backends that lack a CLI command or explicit deployment
  configuration, including microphone, recognizer, synthesizer, model endpoint,
  and unrelated GPIO roles.
- Load deployment configuration before opening hardware. Pass configured I2C,
  GPIO, and Device Tree paths into their Linux owners. Never select the first
  `/dev/gpiochip*`; verify optional exact chip identity and fail before MCU
  reset when automatic board detection cannot establish a supported mapping.
- Provide explicit Robot HAT v4 and v5 deployment profiles. Automatic and v5
  selection require the supported v5 UUID; v4 selection must be explicit and
  must reject a detected v5 overlay. Provision one GPIO path, kernel chip name,
  and label before actuator boot, then retain runtime validation before reset.
- Select camera deployment through configuration. Use `csi` for a Raspberry Pi
  Camera Serial Interface device and `usb` for a V4L2 webcam. Keep capture
  executables and device paths outside the Agent, invoke providers without a
  shell, and enforce the HAL capture deadline in the parent process.
- Keep host command parsing independent of Linux hardware headers. Inject
  console, timing, cancellation, and audio callbacks into `XWalkController`.
  Store the
  caller-owned PiCar-X coordinator and callback context as non-owning pointers.
- Use one process cancellation query for every moving CLI command. Poll it in
  delay slices no longer than 20 milliseconds for move, turn, and self-drive,
  latch emergency stop on cancellation, and install SIGINT and SIGTERM handlers
  before any non-help Raspberry Pi command boots hardware.
- Add command-specific Agent services through constructor-reference overloads.
  Store their addresses as nullable non-owning pointers, report status three
  when a command is invoked without its service, and compose only the service
  selected by the process command.
- Keep `line-track` limited to `start` and `stop`. Run `start` in the foreground
  by repeatedly calling the bounded line-tracking step while the injected
  cancellation query permits it, and stop the motors before returning. Let the
  RPI application map SIGINT and SIGTERM to that cancellation query.
- Execute `self-drive <action>` synchronously through one caller-owned
  `XWalkSelfDrive`. Publish every action with a canonical hyphenated CLI name,
  normalize hyphens to the Agent's exact spaced action name, and continue to
  accept separate action words for compatibility. Let the Agent validate the
  complete upstream action name before changing hardware.
- Build the `xWalkBootRpi` composition root only for RPI mode. Enable the
  Linux I2C and GPIO backend targets through the PiCar-X hardware dependency.
- Never claim GPIO lines, move actuators, enable powered outputs, start audio,
  or contact an external service merely to discover a command or report status.
- Construct project objects in `XWalkBootRpi::run()`, pass dependencies by
  reference, and preserve the same ownership and lifetime rules as any other
  application composition root. Command-specific optional graphs may remain in
  their selected branch but must outlive the command invocation.

## Agent conventions

- Keep `xWalkAgent` beside `xWalkHal`. Normal Agent modules coordinate caller-owned
  HAL objects and must not duplicate physical I/O backends or own injected
  project dependencies. `xWalkBoot` is the intentional composition-boundary
  exception: its optional RPi target owns platform backends only for one
  synchronous application callback, while its core target remains device-free.
- Name agent public headers and sources `xAgent_Rpi5Car<Component>.h` and
  `xAgent_Rpi5Car<Component>.cpp`. Put production declarations in
  `namespace xwalk::agent` while retaining project scalar and container types
  from `xwalk::hal`.
- Give every agent submodule independent host and hardware test options. The
  aggregate `XWALK_AGENT_BUILD_HOST` and `XWALK_AGENT_BUILD_RPI` options must be
  mutually exclusive and default to `OFF`.
- Keep MCU reset and other temporary hardware claims in `xWalkBootRpi`
  root when a later dependency must claim the same physical resource. Destroy
  the temporary backend before constructing the long-lived dependency.
- Keep CLI parsing and sequencing hardware-independent. Inject console, delay,
  audio, and other platform operations, and bind Linux hardware only in the
  optional `xWalkBootRpi` composition target. Application `main()` selects a
  boot mode and consumes non-owning services during the boot callback.
- Keep `XWalkSelfDrive` limited to named gesture, movement, sound, status, and
  queue behavior. Inject `XWalkPicarx`,
  `XWalkMusic`, and timing; the coordinator must not create hardware, audio,
  clock, random, or process backends.
- Preserve the preset action names, ordered actuator commands, relative sound
  paths, volumes, and millisecond delays. Reject unknown queued actions without
  partial execution. The worker delay callback is a non-throwing Boolean status
  operation. A false result latches emergency stop, terminates action processing,
  and makes `waitActionsDone()` return false. Other worker callbacks must remain
  non-throwing because workspace code does not install exception handlers.
- Keep `XWalkLineTracking` limited to line-status decision behavior. Inject
  `XWalkPicarx` and timing, expose one
  bounded step, and leave repeated scheduling, cancellation, and diagnostics
  in the application.
- Preserve line-status priority, steering signs, default movement values, and
  last-direction recovery. Bound recovery sampling and stop the motors when it
  reaches that bound or has no prior left or right direction.

## Documentation and change checklist

Follow the complete Doxygen and MISRA C++-oriented comment standard in
[`DOCUMENTATION_GUIDELINES.md`](DOCUMENTATION_GUIDELINES.md). That document is
authoritative for file headers, section comments, API documentation, namespace
comments, ownership statements, and documentation review.

For a new implementation or a changed public behavior:

1. Place the declaration and implementation in the correct responsibility
   files.
2. Move reusable non-member production logic into `xWalkCommon` and
   `xwalk::hal::common`; keep class-specific logic in methods.
3. Preserve dependency injection and the host/hardware boundary.
4. Validate inputs, conversions, register limits, and ownership assumptions.
5. Add host coverage and, only when necessary, an opt-in hardware test.
6. Add new sources and tests to CMake with the correct visibility and label.
7. Update the module README when its API, layout, build options, test commands,
   hardware behavior, or validation status changes.
8. Update `Doc` when cross-module API mapping, board behavior, protocol,
   installation, project architecture, or safety guidance changes. Keep
   `PORTING_MANIFEST.md` aligned with the upstream documentation coverage.
9. Build with warnings enabled and run the relevant host CTest suite.
10. Re-read this guide. Update it only if the change deliberately establishes a
   reusable project convention.

Typical host verification commands are:

```bash
cmake -S xWalkCLI -B xWalkCLI/build-host -DXWALK_CLI_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkCLI/build-host --parallel
ctest --test-dir xWalkCLI/build-host --output-on-failure

cmake -S xWalkAgent -B xWalkAgent/build-host -DXWALK_AGENT_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAgent/build-host --parallel
ctest --test-dir xWalkAgent/build-host --output-on-failure

cmake -S xWalkI2c -B xWalkI2c/build-host -DXWALK_I2C_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkI2c/build-host --parallel
ctest --test-dir xWalkI2c/build-host --output-on-failure

cmake -S xWalkAudio -B xWalkAudio/build-host -DXWALK_AUDIO_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAudio/build-host --parallel
ctest --test-dir xWalkAudio/build-host --output-on-failure

cmake -S xWalkMusic -B xWalkMusic/build-host -DXWALK_MUSIC_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMusic/build-host --parallel
ctest --test-dir xWalkMusic/build-host --output-on-failure

cmake -S xWalkSpeaker -B xWalkSpeaker/build-host -DXWALK_SPEAKER_BUILD_HOST_TESTS=ON
cmake --build xWalkSpeaker/build-host --parallel
ctest --test-dir xWalkSpeaker/build-host --output-on-failure

cmake -S xWalkPwm -B xWalkPwm/build-host -DXWALK_PWM_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkPwm/build-host --parallel
ctest --test-dir xWalkPwm/build-host --output-on-failure

cmake -S xWalkServo -B xWalkServo/build-host -DXWALK_SERVO_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkServo/build-host --parallel
ctest --test-dir xWalkServo/build-host --output-on-failure

cmake -S xWalkAdc -B xWalkAdc/build-host -DXWALK_ADC_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdc/build-host --parallel
ctest --test-dir xWalkAdc/build-host --output-on-failure

cmake -S xWalkGpio -B xWalkGpio/build-host -DXWALK_GPIO_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGpio/build-host --parallel
ctest --test-dir xWalkGpio/build-host --output-on-failure

cmake -S xWalkMotor -B xWalkMotor/build-host -DXWALK_MOTOR_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMotor/build-host --parallel
ctest --test-dir xWalkMotor/build-host --output-on-failure

cmake -S xWalkConfig -B xWalkConfig/build-host -DXWALK_CONFIG_BUILD_HOST_TESTS=ON
cmake --build xWalkConfig/build-host --parallel
ctest --test-dir xWalkConfig/build-host --output-on-failure

cmake -S xWalkBoardControl -B xWalkBoardControl/build-host -DXWALK_BOARD_CONTROL_BUILD_HOST_TESTS=ON
cmake --build xWalkBoardControl/build-host --parallel
ctest --test-dir xWalkBoardControl/build-host --output-on-failure

cmake -S xWalkTrace -B xWalkTrace/build-host -DXWALK_TRACE_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-host --parallel
ctest --test-dir xWalkTrace/build-host --output-on-failure

cmake -S xWalkGPT -B xWalkGPT/build-host -DXWALK_GPT_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGPT/build-host --parallel
ctest --test-dir xWalkGPT/build-host --output-on-failure

cmake -S xWalkLanguageModel -B xWalkLanguageModel/build-host -DXWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS=ON
cmake --build xWalkLanguageModel/build-host --parallel
ctest --test-dir xWalkLanguageModel/build-host --output-on-failure

cmake -S xWalkVoiceAssistant -B build-va-host -DXWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS=ON
cmake --build build-va-host --parallel
ctest --test-dir build-va-host --output-on-failure

cmake -S xWalkUtils -B build-utils-host -DXWALK_UTILS_BUILD_HOST_TESTS=ON
cmake --build build-utils-host --parallel
ctest --test-dir build-utils-host --output-on-failure

cmake -S xWalkRobot -B xWalkRobot/build-host -DXWALK_ROBOT_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkRobot/build-host --parallel
ctest --test-dir xWalkRobot/build-host --output-on-failure

cmake -S xWalkUltrasonic -B xWalkUltrasonic/build-host -DXWALK_ULTRASONIC_BUILD_HOST_TESTS=ON
cmake --build xWalkUltrasonic/build-host --parallel
ctest --test-dir xWalkUltrasonic/build-host --output-on-failure

cmake -S xWalkLineTracker -B xWalkLineTracker/build-host -DXWALK_LINE_TRACKER_BUILD_HOST_TESTS=ON
cmake --build xWalkLineTracker/build-host --parallel
ctest --test-dir xWalkLineTracker/build-host --output-on-failure

cmake -S xWalkAdxl345 -B xWalkAdxl345/build-host -DXWALK_ADXL345_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdxl345/build-host --parallel
ctest --test-dir xWalkAdxl345/build-host --output-on-failure

cmake -S xWalkBuzzer -B xWalkBuzzer/build-host -DXWALK_BUZZER_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkBuzzer/build-host --parallel
ctest --test-dir xWalkBuzzer/build-host --output-on-failure

cmake -S xWalkLed -B xWalkLed/build-host -DXWALK_LED_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkLed/build-host --parallel
ctest --test-dir xWalkLed/build-host --output-on-failure

cmake -S xWalkUserButton -B xWalkUserButton/build-host -DXWALK_USER_BUTTON_BUILD_HOST_TESTS=ON
cmake --build xWalkUserButton/build-host --parallel
ctest --test-dir xWalkUserButton/build-host --output-on-failure

```

Typical Linux hardware compilation commands are:

```bash
cmake -S xWalkCLI -B xWalkCLI/build-rpi -DXWALK_CLI_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkCLI/build-rpi --parallel
ctest --test-dir xWalkCLI/build-rpi -N -L hardware

cmake -S xWalkAgent -B xWalkAgent/build-rpi -DXWALK_AGENT_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAgent/build-rpi --parallel
ctest --test-dir xWalkAgent/build-rpi -N -L hardware

cmake -S xWalkI2c -B xWalkI2c/build-rpi -DXWALK_I2C_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkI2c/build-rpi --parallel
ctest --test-dir xWalkI2c/build-rpi -N -L hardware

cmake -S xWalkAudio -B xWalkAudio/build-rpi -DXWALK_AUDIO_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAudio/build-rpi --parallel
ctest --test-dir xWalkAudio/build-rpi -N -L hardware

cmake -S xWalkMusic -B xWalkMusic/build-rpi -DXWALK_MUSIC_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMusic/build-rpi --parallel
ctest --test-dir xWalkMusic/build-rpi -N -L hardware

cmake -S xWalkSpeaker -B xWalkSpeaker/build-rpi -DXWALK_SPEAKER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkSpeaker/build-rpi --parallel
ctest --test-dir xWalkSpeaker/build-rpi -N -L hardware

cmake -S xWalkPwm -B xWalkPwm/build-rpi -DXWALK_PWM_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkPwm/build-rpi --parallel
ctest --test-dir xWalkPwm/build-rpi -N -L hardware

cmake -S xWalkServo -B xWalkServo/build-rpi -DXWALK_SERVO_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkServo/build-rpi --parallel
ctest --test-dir xWalkServo/build-rpi -N -L hardware

cmake -S xWalkAdc -B xWalkAdc/build-rpi -DXWALK_ADC_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdc/build-rpi --parallel
ctest --test-dir xWalkAdc/build-rpi -N -L hardware

cmake -S xWalkGpio -B xWalkGpio/build-rpi -DXWALK_GPIO_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGpio/build-rpi --parallel
ctest --test-dir xWalkGpio/build-rpi -N -L hardware

cmake -S xWalkMotor -B xWalkMotor/build-rpi -DXWALK_MOTOR_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMotor/build-rpi --parallel
ctest --test-dir xWalkMotor/build-rpi -N -L hardware

cmake -S xWalkConfig -B xWalkConfig/build-rpi -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkConfig/build-rpi --parallel

cmake -S xWalkBoardControl -B xWalkBoardControl/build-rpi -DXWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkBoardControl/build-rpi --parallel
ctest --test-dir xWalkBoardControl/build-rpi -N -L hardware

cmake -S xWalkTrace -B xWalkTrace/build-rpi -DXWALK_TRACE_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkTrace/build-rpi --parallel
ctest --test-dir xWalkTrace/build-rpi -N -L hardware

cmake -S xWalkGPT -B xWalkGPT/build-rpi -DXWALK_GPT_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGPT/build-rpi --parallel
ctest --test-dir xWalkGPT/build-rpi -N -L hardware

cmake -S xWalkLanguageModel -B xWalkLanguageModel/build-rpi -DXWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkLanguageModel/build-rpi --parallel
ctest --test-dir xWalkLanguageModel/build-rpi -N -L hardware

cmake -S xWalkVoiceAssistant -B build-va-rpi -DXWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS=ON
cmake --build build-va-rpi --parallel
ctest --test-dir build-va-rpi -N -L hardware

cmake -S xWalkUtils -B build-utils-rpi -DXWALK_UTILS_BUILD_HARDWARE_TESTS=ON
cmake --build build-utils-rpi --parallel
ctest --test-dir build-utils-rpi -N -L hardware

cmake -S xWalkRobot -B xWalkRobot/build-rpi -DXWALK_ROBOT_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkRobot/build-rpi --parallel
ctest --test-dir xWalkRobot/build-rpi -N -L hardware

cmake -S xWalkUltrasonic -B xWalkUltrasonic/build-rpi -DXWALK_ULTRASONIC_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkUltrasonic/build-rpi --parallel
ctest --test-dir xWalkUltrasonic/build-rpi -N -L hardware

cmake -S xWalkLineTracker -B xWalkLineTracker/build-rpi -DXWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkLineTracker/build-rpi --parallel
ctest --test-dir xWalkLineTracker/build-rpi -N -L hardware

cmake -S xWalkAdxl345 -B xWalkAdxl345/build-rpi -DXWALK_ADXL345_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkAdxl345/build-rpi --parallel
ctest --test-dir xWalkAdxl345/build-rpi -N -L hardware

cmake -S xWalkBuzzer -B xWalkBuzzer/build-rpi -DXWALK_BUZZER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkBuzzer/build-rpi --parallel
ctest --test-dir xWalkBuzzer/build-rpi -N -L hardware

cmake -S xWalkLed -B xWalkLed/build-rpi -DXWALK_LED_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkLed/build-rpi --parallel
ctest --test-dir xWalkLed/build-rpi -N -L hardware

cmake -S xWalkUserButton -B xWalkUserButton/build-rpi -DXWALK_USER_BUTTON_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkUserButton/build-rpi --parallel
ctest --test-dir xWalkUserButton/build-rpi -N -L hardware

```

These commands compile the Linux backend and hardware-test executables but do
not run them. A successful Servo hardware build also compiles its PWM and I2C
hardware dependencies. `xWalkConfig` has no physical-hardware tests; its target
command compiles only the production filesystem library.

## Current verification status

As of 2026-08-03:

- The standalone xWalkCLI host suite passes, and its Controller executable
  builds in the aggregate host and Ubuntu/RPI configurations.
- The xWalk Agent PiCar-X host suite passes with in-memory I2C and GPIO
  callbacks, and its Linux/RPI hardware test compiles without being executed.
- The xWalk SelfDrive host suite passes with in-memory timing, audio, I2C, and
  GPIO callbacks, and its Linux/RPI production library compiles successfully.
- The xWalk LineTracking host suite passes with in-memory timing, I2C, and GPIO
  callbacks, and its Linux/RPI production library compiles successfully.
- The PiCar-X CLI host suite passes with injected console, delay, audio,
  I2C, and GPIO backends. Its Raspberry Pi main executable compiles, while
  physical execution remains opt-in.
- The aggregate host suite includes CLI parsing and dispatch for PiCar-X,
  foreground line tracking, and preset self-drive actions. External-service
  commands remain unavailable until their safe process-level backends are composed.
- The I2C Linux backend and `xWalkI2cLinuxHardwareTest` compile successfully.
- The xWalkIW Protobuf/YAML contract validates, its generated gRPC C++ library
  compiles, and its host schema test passes in the aggregate suite.
- The SPI core, Agent transaction service, and CLI dispatch host tests pass with
  injected transfers; the Linux spidev backend and opt-in hardware test compile.
- The CLI `spi transfer <HEX>` path compiles in the complete Raspberry Pi
  executable without initializing the Robot HAT or other PiCar-X services.
- The shared ALSA backend software test passes with injected operations, and its
  silent hardware test compiles without being executed.
- The PWM hardware target and `xWalkPwmHardwareTest` compile successfully.
- The Servo hardware target and `xWalkServoHardwareTest` compile successfully.
- The ADC, GPIO, and Motor hardware-test targets compile successfully.
- The I2C, PWM, Servo, ADC, GPIO, Motor, and combined Config host suites pass.
- The combined BoardControl, Device, and FirmwareInfo host suites pass with their dependencies.
- Their combined target and Linux ADC, I2C, and GPIO dependencies compile successfully.
- The Trace host suite passes, and its hardware-independent target compile test builds successfully.
- Trace output remains callback-defined; the port has no physical-hardware test or built-in console backend.
- The GPT host suites and their BoardControl, ADC, I2C, and GPIO dependency suites pass.
- The GPT microphone and playback targets compile and require explicit devices before physical access.
- The LanguageModel coordinator and Ollama provider host suites pass with fake transports.
- The Ollama target compiles and requires explicit endpoint, model, and prompt before network access.
- The VoiceAssistant host suite passes neutral coordinator and completed ALSA/Ollama composition tests.
- The VoiceAssistant full-stack target compiles and requires explicit microphone,
  playback, model, and fixture input.
- The Utils host suite passes with injected platform, clock, and standard-error backends.
- The Utils target compile test builds without executing platform or operating-system services.
- The Robot host suite and its Servo, PWM, I2C, and Config dependency suites pass.
- The combined Config target-oriented production library compiles successfully.
- The Robot target and its Servo, PWM, and I2C hardware dependencies compile successfully.
- The Ultrasonic host suite and its GPIO dependency suite pass.
- The Ultrasonic target and its Linux GPIO hardware dependency compile successfully.
- The LineTracker host suite and its ADC and I2C dependency suites pass.
- The LineTracker target and its ADC and Linux I2C hardware dependencies compile successfully.
- The ADXL345 host suite and its I2C dependency suite pass.
- The ADXL345 target and its Linux I2C hardware dependency compile successfully.
- The Buzzer host suite and its PWM, I2C, and GPIO dependency suites pass.
- The Buzzer target and its PWM, I2C, and GPIO hardware dependencies compile successfully.
- The combined LED host suites and their GPIO, PWM, and I2C dependency suites pass.
- The combined LED target and its Linux GPIO and I2C hardware dependencies compile successfully.
- The UserButton host suite and its GPIO dependency suite pass.
- The UserButton target and its Linux GPIO hardware dependency compile successfully.
- The bounded D0 button-event sequence passes with an in-memory host backend,
  and its Linux adapter and central hardware selector compile successfully;
  physical button events have not been exercised.
- The three-servo initialization-angle sequence passes with in-memory GPIO and
  I2C backends. Its MCU-reset and PWM hardware composition compiles; physical
  servo movement has not been exercised.
- The bounded Robot HAT v5 four-motor sequence passes with an in-memory I2C
  backend and guaranteed cleanup attempts. Its physical dual-PWM composition
  compiles; motor movement has not been exercised.
- The bounded Robot HAT two-motor sequence passes with in-memory I2C and GPIO
  backends and guaranteed cleanup attempts. Its P13/D4 and P12/D5 physical
  composition compiles; motor movement has not been exercised.
- The bounded Robot HAT servo sequence passes with in-memory GPIO and I2C
  backends across all 16 PWM servos and five ADC channels. Its physical
  composition compiles; MCU reset and servo movement have not been exercised.
- The bounded 12-channel servo sweep passes with an in-memory I2C backend and
  verifies progressive channel order at negative and positive 20 degrees. Its
  physical composition compiles; servo movement has not been exercised.
- The Piper stream-comparison sequence passes with injected provider, clock,
  and output callbacks. No physical case is registered because the workspace
  has no C++ Piper provider supporting both streamed and buffered modes.
- The 17-measure Robot HAT tone sequence passes through the real Music
  abstraction with an in-memory audio backend. Its ALSA hardware composition
  compiles and remains disabled by default; physical playback has not been exercised.
- The ported Robot HAT LED example passes with injected LED, timing, and output
  callbacks. Its GPIO26 Linux composition and central example selector compile;
  the physical 19-second LED flow remains disabled by default and has not run.
- The ported DeepSeek chat example passes with an injected language model and
  console. Its authenticated HTTPS adapter and central example selector compile;
  the disabled live case has not sent prompts or consumed provider service.
- The ported Doubao image-chat example passes with injected camera, language
  model, and console dependencies. Its camera and authenticated HTTPS adapters
  compile; the disabled live case has not captured or uploaded an image.
- The ported Doubao text-chat example passes with an injected language model
  and console. Its authenticated HTTPS adapter compiles; the disabled live case
  has not sent a prompt or consumed provider service.
- The ported Gemini chat example passes with an injected language model and
  console. Its authenticated OpenAI-compatible HTTPS adapter compiles; the
  disabled live case has not sent a prompt or consumed provider service.
- The ported Grok chat example passes with an injected language model and
  console. Its authenticated OpenAI-compatible HTTPS adapter compiles; the
  disabled live case has not sent a prompt or consumed provider service.
- The ported Ollama text-chat example passes with an injected language model
  and console. Its native Ollama adapter compiles with the upstream localhost
  endpoint and `deepseek-r1:1.5b` model; the disabled live case has not sent a
  prompt.
- The ported Ollama image-chat example passes with injected camera, language
  model, and console dependencies. Its 1280-by-720 camera and native Ollama
  adapters compile; the disabled live case has not captured or uploaded an image.
- The ported OpenAI image-chat example passes with injected camera, language
  model, and console dependencies. Its 640-by-480 camera and authenticated
  OpenAI-compatible HTTPS adapters compile; the disabled live case has not
  captured or uploaded an image.
- The ported OpenAI text-chat example passes with an injected language model
  and console. Its authenticated OpenAI-compatible HTTPS adapter compiles; the
  disabled live case has not sent a prompt or consumed provider service.
- The ported generic-provider chat template passes with an injected language
  model and console. Its runtime-selected OpenAI-compatible HTTPS adapter
  compiles; the disabled live case has not sent a prompt or consumed provider
  service.
- The ported Qwen chat example passes with an injected language model and
  console. Its configurable DashScope OpenAI-compatible HTTPS adapter compiles;
  the disabled live case has not sent a prompt or consumed provider service.
- The ported pin-input example passes with injected input, timing, and reporting
  operations. Its bounded D3/GPIO22 pull-up adapter compiles; the disabled live
  case has not opened or sampled a physical GPIO line.
- The ported servo example passes with injected angle, timing, and reporting
  operations. Its bounded PWM-channel-one adapter compiles; the disabled live
  case has not opened I2C or moved a physical servo.
- The Music host suite passes, and its hardware-independent target compiles successfully.
- Music callbacks are connected to shared ALSA and its opt-in hardware target compiles.
- Native Music MP3 decoding is covered by a device-free libsndfile host test.
- The xWalkBoot host stub passes one-shot lifecycle tests, and its RPi composition target compiles.
- The Speaker host suite passes, and its hardware-independent target compiles successfully.
- Speaker decoding and shared-ALSA playback are covered by host and opt-in hardware targets.
- The host and hardware compilation paths are warning-clean under the configured
  GCC warning options.
- Host coverage passes enforced minimums of 79 percent for lines and 40 percent
  for branches.
- Hardware tests have not been executed as part of this verification.
