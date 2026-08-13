cmake_minimum_required(VERSION 3.16)

project(xWalk VERSION 1.0.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/xWalkLibrary/XWalkDependencies.cmake)
include(CTest)
include(GNUInstallDirs)

option(XWALK_BUILD_RPI "Build Raspberry Pi backends and the deployable CLI" OFF)
option(XWALK_ENABLE_STRICT_WARNINGS "Add the host sanity warning extensions" OFF)
option(XWALK_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(XWALK_ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer for host concurrency tests" OFF)
option(XWALK_ENABLE_COVERAGE "Enable GCC gcov coverage instrumentation" OFF)
set(XWALK_COVERAGE_BACKEND "gcc" CACHE STRING "Coverage backend: gcc or clang")
set_property(CACHE XWALK_COVERAGE_BACKEND PROPERTY STRINGS gcc clang)
option(XWALK_ENABLE_PACKAGING "Enable CPack Debian package generation" OFF)
option(XWALK_BUILD_FUZZERS "Build Clang libFuzzer harnesses" OFF)
option(XWALK_AARCH64_DEPENDENCY_AUDIT
    "Audit required target dependencies before an AArch64 RPI configure" ON)

add_custom_target(host-dependency-check
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/shell/check-host-quality-dependencies.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Reporting host-quality tool availability")
add_custom_target(host-shellcheck
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/shell/run-host-shellcheck.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Checking every repository-owned shell script")

if(XWALK_BUILD_RPI AND CMAKE_CROSSCOMPILING AND
    CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$" AND
    XWALK_AARCH64_DEPENDENCY_AUDIT)
    execute_process(
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/aarch64-dependency-audit.sh"
            "${XWALK_AARCH64_SYSROOT}"
        RESULT_VARIABLE XWALK_AARCH64_AUDIT_RESULT
        OUTPUT_VARIABLE XWALK_AARCH64_AUDIT_OUTPUT
        ERROR_VARIABLE XWALK_AARCH64_AUDIT_ERROR)
    if(NOT XWALK_AARCH64_AUDIT_RESULT EQUAL 0)
        message(FATAL_ERROR
            "ARM64 target dependency audit failed.\n"
            "${XWALK_AARCH64_AUDIT_OUTPUT}${XWALK_AARCH64_AUDIT_ERROR}\n"
            "Review xWalkTool/deployment/ARM64_CROSS_BUILD.md. Do not disable "
            "the audit for a deployable build.")
    endif()
    message(STATUS "${XWALK_AARCH64_AUDIT_OUTPUT}")
endif()

if(XWALK_ENABLE_THREAD_SANITIZER AND
    (XWALK_ENABLE_SANITIZERS OR XWALK_ENABLE_COVERAGE OR XWALK_BUILD_RPI))
    message(FATAL_ERROR
        "ThreadSanitizer requires a separate host build without other instrumentation")
endif()
if(XWALK_ENABLE_SANITIZERS AND XWALK_ENABLE_COVERAGE)
    message(FATAL_ERROR
        "AddressSanitizer/UndefinedBehaviorSanitizer and coverage require separate build directories")
endif()
if(XWALK_ENABLE_COVERAGE AND NOT XWALK_COVERAGE_BACKEND MATCHES "^(gcc|clang)$")
    message(FATAL_ERROR "XWALK_COVERAGE_BACKEND must be gcc or clang")
endif()
if(XWALK_ENABLE_COVERAGE AND XWALK_COVERAGE_BACKEND STREQUAL "clang" AND
    NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "The clang coverage backend requires Clang")
endif()
if(XWALK_ENABLE_THREAD_SANITIZER AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "ThreadSanitizer requires GCC or Clang")
endif()

if(BUILD_TESTING AND NOT XWALK_BUILD_RPI)
    set(XWALK_ROOT_BUILD_HOST ON)
else()
    set(XWALK_ROOT_BUILD_HOST OFF)
endif()
set(XWALK_HAL_BUILD_HOST ${XWALK_ROOT_BUILD_HOST} CACHE BOOL "Controlled by the workspace root" FORCE)
set(XWALK_HAL_BUILD_RPI ${XWALK_BUILD_RPI} CACHE BOOL "Controlled by the workspace root" FORCE)
set(XWALK_HAL_BUILD_CLI ON CACHE BOOL "Build the workspace CLI" FORCE)
set(XWALK_CLI_INSTALL_RUNTIME
    ${XWALK_ROOT_BUILD_HOST} CACHE BOOL "Install the CLI runtime layout" FORCE)
if(XWALK_BUILD_RPI)
    set(XWALK_CLI_INSTALL_RUNTIME ON CACHE BOOL
        "Install the CLI runtime layout" FORCE)
endif()

if(XWALK_HAL_BUILD_HOST AND XWALK_HAL_BUILD_RPI)
    message(FATAL_ERROR
        "Host and RPI verification require separate build directories")
endif()

if(XWALK_HAL_BUILD_RPI AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "XWALK_HAL_BUILD_RPI requires a Linux build host")
endif()

if(XWALK_HAL_BUILD_RPI)
    include(CheckIncludeFileCXX)
    check_include_file_cxx("linux/gpio.h" XWALK_HAL_HAS_LINUX_GPIO_HEADER)
    check_include_file_cxx("linux/i2c-dev.h" XWALK_HAL_HAS_LINUX_I2C_DEV_HEADER)
    check_include_file_cxx("linux/i2c.h" XWALK_HAL_HAS_LINUX_I2C_HEADER)
    check_include_file_cxx("linux/spi/spidev.h" XWALK_HAL_HAS_LINUX_SPI_HEADER)
    if(NOT XWALK_HAL_HAS_LINUX_GPIO_HEADER OR
        NOT XWALK_HAL_HAS_LINUX_I2C_DEV_HEADER OR
        NOT XWALK_HAL_HAS_LINUX_I2C_HEADER OR
        NOT XWALK_HAL_HAS_LINUX_SPI_HEADER)
        message(FATAL_ERROR
            "The RPI build requires Linux GPIO, I2C, and SPI UAPI headers")
    endif()
endif()

set(XWALK_HAL_HOST_OPTION_NAMES
    XWALK_ADC_BUILD_HOST_TESTS
    XWALK_ADXL345_BUILD_HOST_TESTS
    XWALK_AUDIO_BUILD_HOST_TESTS
    XWALK_BOARD_CONTROL_BUILD_HOST_TESTS
    XWALK_BUZZER_BUILD_HOST_TESTS
    XWALK_CAMERA_BUILD_HOST_TESTS
    XWALK_CONFIG_BUILD_HOST_TESTS
    XWALK_CLI_BUILD_HOST
    XWALK_GPIO_BUILD_HOST_TESTS
    XWALK_GPT_BUILD_HOST_TESTS
    XWALK_I2C_BUILD_HOST_TESTS
    XWALK_IW_BUILD_HOST_TESTS
    XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS
    XWALK_LED_BUILD_HOST_TESTS
    XWALK_LINE_TRACKER_BUILD_HOST_TESTS
    XWALK_MOTOR_BUILD_HOST_TESTS
    XWALK_MUSIC_BUILD_HOST_TESTS
    XWALK_PWM_BUILD_HOST_TESTS
    XWALK_ROBOT_BUILD_HOST_TESTS
    XWALK_SERVO_BUILD_HOST_TESTS
    XWALK_SPEAKER_BUILD_HOST_TESTS
    XWALK_SPI_BUILD_HOST_TESTS
    XWALK_TRACE_BUILD_HOST_TESTS
    XWALK_ULTRASONIC_BUILD_HOST_TESTS
    XWALK_USER_BUTTON_BUILD_HOST_TESTS
    XWALK_UTILS_BUILD_HOST_TESTS
    XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS)

set(XWALK_HAL_RPI_OPTION_NAMES
    XWALK_ADC_BUILD_HARDWARE_TESTS
    XWALK_ADXL345_BUILD_HARDWARE_TESTS
    XWALK_AUDIO_BUILD_HARDWARE_TESTS
    XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS
    XWALK_BUZZER_BUILD_HARDWARE_TESTS
    XWALK_CAMERA_BUILD_HARDWARE_TESTS
    XWALK_CLI_BUILD_RPI
    XWALK_GPIO_BUILD_HARDWARE_TESTS
    XWALK_GPT_BUILD_HARDWARE_TESTS
    XWALK_I2C_BUILD_HARDWARE_TESTS
    XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS
    XWALK_LED_BUILD_HARDWARE_TESTS
    XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS
    XWALK_MOTOR_BUILD_HARDWARE_TESTS
    XWALK_MUSIC_BUILD_HARDWARE_TESTS
    XWALK_PWM_BUILD_HARDWARE_TESTS
    XWALK_ROBOT_BUILD_HARDWARE_TESTS
    XWALK_SERVO_BUILD_HARDWARE_TESTS
    XWALK_SPEAKER_BUILD_HARDWARE_TESTS
    XWALK_SPI_BUILD_HARDWARE_TESTS
    XWALK_TRACE_BUILD_HARDWARE_TESTS
    XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS
    XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS
    XWALK_UTILS_BUILD_HARDWARE_TESTS
    XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS)

set(XWALK_HAL_CENTRAL_TEST_OPTION_NAMES
    XWALK_ADC_BUILD_HOST_TESTS
    XWALK_ADXL345_BUILD_HOST_TESTS
    XWALK_AUDIO_BUILD_HOST_TESTS
    XWALK_BOARD_CONTROL_BUILD_HOST_TESTS
    XWALK_BUZZER_BUILD_HOST_TESTS
    XWALK_CAMERA_BUILD_HOST_TESTS
    XWALK_CONFIG_BUILD_HOST_TESTS
    XWALK_GPIO_BUILD_HOST_TESTS
    XWALK_GPT_BUILD_HOST_TESTS
    XWALK_I2C_BUILD_HOST_TESTS
    XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS
    XWALK_LED_BUILD_HOST_TESTS
    XWALK_LINE_TRACKER_BUILD_HOST_TESTS
    XWALK_MOTOR_BUILD_HOST_TESTS
    XWALK_MUSIC_BUILD_HOST_TESTS
    XWALK_PWM_BUILD_HOST_TESTS
    XWALK_ROBOT_BUILD_HOST_TESTS
    XWALK_SERVO_BUILD_HOST_TESTS
    XWALK_SPEAKER_BUILD_HOST_TESTS
    XWALK_SPI_BUILD_HOST_TESTS
    XWALK_TRACE_BUILD_HOST_TESTS
    XWALK_ULTRASONIC_BUILD_HOST_TESTS
    XWALK_USER_BUTTON_BUILD_HOST_TESTS
    XWALK_UTILS_BUILD_HOST_TESTS
    XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS)

foreach(host_option_name IN LISTS XWALK_HAL_HOST_OPTION_NAMES)
    set(${host_option_name} ${XWALK_HAL_BUILD_HOST} CACHE BOOL
        "Controlled by the workspace root" FORCE)
    mark_as_advanced(${host_option_name})
endforeach()
foreach(rpi_option_name IN LISTS XWALK_HAL_RPI_OPTION_NAMES)
    set(${rpi_option_name} ${XWALK_HAL_BUILD_RPI} CACHE BOOL
        "Controlled by the workspace root" FORCE)
    mark_as_advanced(${rpi_option_name})
endforeach()

if(BUILD_TESTING AND (XWALK_HAL_BUILD_HOST OR XWALK_HAL_BUILD_RPI))
    set(XWALK_HAL_BUILD_CENTRAL_TESTS ON)
else()
    set(XWALK_HAL_BUILD_CENTRAL_TESTS OFF)
endif()
if(XWALK_HAL_BUILD_CENTRAL_TESTS)
    foreach(central_test_option_name IN LISTS XWALK_HAL_CENTRAL_TEST_OPTION_NAMES)
        set(${central_test_option_name} OFF CACHE BOOL
            "Collected by the xGoogleTest executable" FORCE)
    endforeach()
endif()

set(XWALK_GPT_BUILD_VOSK_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Vosk provider" FORCE)
set(XWALK_GPT_BUILD_ESPEAK_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Espeak provider" FORCE)
set(XWALK_GPT_BUILD_PIPER_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Piper provider" FORCE)
set(XWALK_GPT_BUILD_PICO2WAVE_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Pico2Wave provider" FORCE)
set(XWALK_GPT_BUILD_STT_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA speech capture" FORCE)
set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA speech playback" FORCE)
set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA music" FORCE)
set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build compressed-audio decoding" FORCE)
if(XWALK_HAL_BUILD_CENTRAL_TESTS)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by centralized ALSA software tests" FORCE)
    set(XWALK_GPT_BUILD_STT_ALSA_BACKEND ON CACHE BOOL
        "Required by centralized speech capture tests" FORCE)
    set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND ON CACHE BOOL
        "Required by centralized speech playback tests" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER ON CACHE BOOL
        "Required by centralized language-model tests" FORCE)
    set(XWALK_I2C_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by centralized I2C host simulation" FORCE)
    set(XWALK_GPIO_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by centralized GPIO host simulation" FORCE)
    set(XWALK_SPI_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by centralized SPI host simulation" FORCE)
    set(XWALK_MUSIC_BUILD_ALSA_BACKEND ON CACHE BOOL
        "Required by centralized music tests" FORCE)
    set(XWALK_SPEAKER_BUILD_ALSA_BACKEND ON CACHE BOOL
        "Required by centralized speaker tests" FORCE)
    set(XWALK_UTILS_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by centralized Linux utility tests" FORCE)
endif()
mark_as_advanced(
    XWALK_GPT_BUILD_VOSK_PROVIDER
    XWALK_GPT_BUILD_ESPEAK_PROVIDER
    XWALK_GPT_BUILD_PIPER_PROVIDER
    XWALK_GPT_BUILD_PICO2WAVE_PROVIDER
    XWALK_GPT_BUILD_STT_ALSA_BACKEND
    XWALK_GPT_BUILD_TTS_ALSA_BACKEND
    XWALK_MUSIC_BUILD_ALSA_BACKEND
    XWALK_MUSIC_BUILD_SNDFILE_DECODER)

if(XWALK_GPT_BUILD_TTS_ALSA_BACKEND)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by the speech ALSA backend" FORCE)
endif()

if(XWALK_HAL_BUILD_HOST)
    message(STATUS "xWalk HAL host mode: central host tests are enabled")
elseif(XWALK_HAL_BUILD_RPI)
    message(STATUS "xWalk HAL RPI mode: hardware targets are enabled")
else()
    message(STATUS "xWalk HAL production mode: verification targets are disabled")
endif()

add_subdirectory(xWalkLibrary/common)
add_subdirectory(xWalkIW)
add_subdirectory(xWalkTrace)
add_subdirectory(xWalkHal/interface/xWalkI2c)
add_subdirectory(xWalkHal/interface/xWalkSpi)
add_subdirectory(xWalkHal/interface/xWalkGpio)
add_subdirectory(xWalkHal/interface/xWalkAudio)
add_subdirectory(xWalkHal/interface/xWalkConfig)
add_subdirectory(xWalkHal/interface/xWalkUtils)
add_subdirectory(xWalkHal/interface/xWalkLanguageModel)
add_subdirectory(xWalkHal/device/xWalkPwm)
add_subdirectory(xWalkHal/device/xWalkAdc)
add_subdirectory(xWalkHal/device/xWalkServo)
add_subdirectory(xWalkHal/device/xWalkAdxl345)
add_subdirectory(xWalkHal/device/xWalkUltrasonic)
add_subdirectory(xWalkHal/device/xWalkCamera)
add_subdirectory(xWalkHal/device/xWalkUserButton)
add_subdirectory(xWalkHal/sensor/xWalkLineTracker)
add_subdirectory(xWalkHal/sensor/xWalkMotor)
add_subdirectory(xWalkHal/sensor/xWalkLed)
add_subdirectory(xWalkHal/sensor/xWalkBuzzer)
add_subdirectory(xWalkHal/layer1/xWalkMusic)
add_subdirectory(xWalkHal/layer1/xWalkSpeaker)
add_subdirectory(xWalkHal/layer1/xWalkBoardControl)
add_subdirectory(xWalkHal/layer1/xWalkRobot)
add_subdirectory(xWalkHal/layer1/xWalkGPT)
add_subdirectory(xWalkHal/layer1/xWalkVoiceAssistant)

if(XWALK_HAL_BUILD_HOST)
    add_subdirectory(xWalkHal/simulation/xWalkRobotHat)
    add_subdirectory(xWalkHal/interface/test)
    add_subdirectory(xWalkHal/device/test)
    add_subdirectory(xWalkHal/sensor/test)
    add_subdirectory(xWalkHal/layer1/test)
endif()

if(XWALK_HAL_BUILD_RPI)
    add_subdirectory(xWalkHal/xWalkTest/xExample)
endif()
if(BUILD_TESTING)
    add_subdirectory(xWalkHal/xWalkTest/xSequenceTest)
endif()
if(XWALK_HAL_BUILD_CLI OR XWALK_HAL_BUILD_HOST OR XWALK_HAL_BUILD_RPI)
    add_subdirectory(xWalkController)
endif()
if(XWALK_BUILD_FUZZERS)
    add_subdirectory(xWalkTool/fuzz)
endif()

add_library(xWalkHal INTERFACE)
add_library(xWalk::Hal ALIAS xWalkHal)
target_compile_features(xWalkHal INTERFACE cxx_std_17)
target_link_libraries(xWalkHal INTERFACE
    xWalkAdc
    xWalkAdxl345
    xWalkAudio
    xWalkBoardControl
    xWalkBuzzer
    xWalkCamera
    xWalkLibraryCommon
    xWalkConfig
    xWalkGpio
    xWalkGPT
    xWalkI2c
    xWalkIW
    xWalkLanguageModel
    xWalkLed
    xWalkLineTracker
    xWalkMotor
    xWalkMusic
    xWalkPwm
    xWalkRobot
    xWalkServo
    xWalkSpeaker
    xWalkSpi
    xWalkTrace
    xWalkUltrasonic
    xWalkUserButton
    xWalkUtils
    xWalkVoiceAssistant)
if(XWALK_HAL_BUILD_RPI)
    target_link_libraries(xWalkHal INTERFACE
        xWalkGpioLinux
        xWalkCameraLinux
        xWalkI2cLinux
        xWalkSpiLinux
        xWalkLanguageModelOllama
        xWalkAudioAlsa
        xWalkMusicAlsa
        xWalkSpeakerAlsa
        xWalkSpeechToTextAlsa
        xWalkSpeechRecognizerVosk
        xWalkTextToSpeechAlsa
        xWalkTextToSpeechEspeak
        xWalkTextToSpeechPico2Wave
        xWalkUtilsLinux)
endif()
if(XWALK_HAL_BUILD_CENTRAL_TESTS)
    add_subdirectory(xWalkHal/xWalkTest/xGoogleTest)
endif()

if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME xWalkProvisioningHostTest
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/setup-rpi-test.sh")
    set_tests_properties(xWalkProvisioningHostTest PROPERTIES LABELS "host;deployment")
    add_test(NAME xWalkLanguageModelConfigHostTest
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/language-model-config-test.sh")
    set_tests_properties(xWalkLanguageModelConfigHostTest PROPERTIES
        LABELS "host;deployment")
    add_test(NAME xWalkEnvironmentLoaderHostTest
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/environment-loader-test.sh")
    set_tests_properties(xWalkEnvironmentLoaderHostTest PROPERTIES
        LABELS "host;deployment")
    add_test(NAME xWalkLicenseToolHostTest
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/python/test/test_xWalkLicenseTool.py")
    set_tests_properties(xWalkLicenseToolHostTest PROPERTIES
        LABELS "host;deployment;security")
    add_test(NAME xWalkJiraImportHostTest
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover
            -s test -p "test_xWalkJiraImport*.py")
    set_tests_properties(xWalkJiraImportHostTest PROPERTIES
        LABELS "host;tooling;network-free"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/xWalkJiraImport")
    add_test(NAME xWalkGerritHostTest
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover
            -s py-test -p "xWalkGerrit*Test.py")
    set_tests_properties(xWalkGerritHostTest PROPERTIES
        LABELS "host;tooling;network-free"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/gerrit")
    find_program(XWALK_NODE_EXECUTABLE node)
    if(XWALK_NODE_EXECUTABLE)
        add_test(NAME xWalkGerritUiHostTest
            COMMAND "${XWALK_NODE_EXECUTABLE}" --test py-test/xWalkReviewControlsTest.js)
        set_tests_properties(xWalkGerritUiHostTest PROPERTIES
            LABELS "host;tooling;network-free"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/gerrit")
    endif()
    add_test(NAME xHal_Rpi5CarDependencyInstallerHostTest
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/dependency-installer-test.py")
    set_tests_properties(xHal_Rpi5CarDependencyInstallerHostTest PROPERTIES
        LABELS "host;deployment")
    add_test(NAME xWalkAarch64DependencyAuditHostTest
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/aarch64-dependency-audit-test.sh")
    set_tests_properties(xWalkAarch64DependencyAuditHostTest PROPERTIES
        LABELS "host;deployment;dependency")
    add_test(NAME xWalkLibraryX86ArchitectureHostTest
        COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_SYSTEM_PROCESSOR=x86_64
            -DXWALK_EXPECTED_ARCHITECTURE=x86_64
            -P "${CMAKE_CURRENT_SOURCE_DIR}/xWalkLibrary/test/xWalkLibraryArchitectureTest.cmake")
    add_test(NAME xWalkLibraryAarch64ArchitectureHostTest
        COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_SYSTEM_PROCESSOR=aarch64
            -DXWALK_EXPECTED_ARCHITECTURE=aarch64
            -P "${CMAKE_CURRENT_SOURCE_DIR}/xWalkLibrary/test/xWalkLibraryArchitectureTest.cmake")
    add_test(NAME xWalkLibraryUnsupportedArchitectureHostTest
        COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_SYSTEM_PROCESSOR=armv7l
            -DXWALK_EXPECTED_ARCHITECTURE=unsupported
            -P "${CMAKE_CURRENT_SOURCE_DIR}/xWalkLibrary/test/xWalkLibraryArchitectureTest.cmake")
    set_tests_properties(
        xWalkLibraryX86ArchitectureHostTest
        xWalkLibraryAarch64ArchitectureHostTest
        PROPERTIES LABELS "host;dependency")
    set_tests_properties(xWalkLibraryUnsupportedArchitectureHostTest PROPERTIES
        LABELS "host;dependency"
        WILL_FAIL TRUE)
endif()

find_program(XWALK_CPPCHECK_EXECUTABLE cppcheck)
if(XWALK_CPPCHECK_EXECUTABLE AND CMAKE_EXPORT_COMPILE_COMMANDS)
    set(XWALK_CPPCHECK_SUPPRESSIONS
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/environment/.cppcheck-suppressions")
    add_custom_target(cppcheck
        COMMAND "${XWALK_CPPCHECK_EXECUTABLE}"
            "--project=${CMAKE_BINARY_DIR}/compile_commands.json"
            --enable=warning,performance,portability
            --error-exitcode=1
            "-i${CMAKE_CURRENT_SOURCE_DIR}/xWalkIW/auto-gen"
            --suppress=missingIncludeSystem
            --suppress=assertWithSideEffect
            "--suppressions-list=${XWALK_CPPCHECK_SUPPRESSIONS}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM)
    add_custom_target(cppcheck-full
        COMMAND "${XWALK_CPPCHECK_EXECUTABLE}"
            "--project=${CMAKE_BINARY_DIR}/compile_commands.json"
            --enable=warning,style,performance,portability
            --inconclusive
            "-i${CMAKE_CURRENT_SOURCE_DIR}/xWalkIW/auto-gen"
            --suppress=missingIncludeSystem
            "--suppressions-list=${XWALK_CPPCHECK_SUPPRESSIONS}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM)
endif()

if(XWALK_ENABLE_PACKAGING)
    if(NOT XWALK_BUILD_RPI)
        message(FATAL_ERROR "Debian packaging requires XWALK_BUILD_RPI=ON")
    endif()
    if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        message(FATAL_ERROR
            "PiCar-X Debian packaging requires an ARM64 target; the x86 RPI profile is compile-check only")
    endif()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "xwalk-picarx")
    set(CPACK_PACKAGE_VENDOR "xWalk Firmware")
    set(CPACK_PACKAGE_CONTACT "Joxy John")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "xWalk PiCar-X command-line firmware")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "espeak-ng, i2c-tools, gpiod, python3, python3-nacl")
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "rpicam-apps | ffmpeg")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/debian/conffiles")
    include(CPack)
endif()
