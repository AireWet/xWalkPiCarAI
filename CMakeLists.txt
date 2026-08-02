cmake_minimum_required(VERSION 3.16)

project(xWalk VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

option(XWALK_BUILD_RPI "Build Raspberry Pi backends and the deployable CLI" OFF)
option(XWALK_ENABLE_STRICT_WARNINGS "Add the host sanity warning extensions" OFF)
option(XWALK_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(XWALK_ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer for host concurrency tests" OFF)
option(XWALK_ENABLE_COVERAGE "Enable GCC gcov coverage instrumentation" OFF)
option(XWALK_ENABLE_PACKAGING "Enable CPack Debian package generation" OFF)

if(XWALK_ENABLE_THREAD_SANITIZER AND
    (XWALK_ENABLE_SANITIZERS OR XWALK_ENABLE_COVERAGE OR XWALK_BUILD_RPI))
    message(FATAL_ERROR
        "ThreadSanitizer requires a separate host build without other instrumentation")
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
set(XWALK_CLI_INSTALL_RUNTIME ON CACHE BOOL "Install the CLI runtime layout" FORCE)

add_subdirectory(xWalkHal)

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
    add_test(NAME xHal_Rpi5CarDependencyInstallerHostTest
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/test/dependency-installer-test.py")
    set_tests_properties(xHal_Rpi5CarDependencyInstallerHostTest PROPERTIES
        LABELS "host;deployment")
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
            --suppress=missingIncludeSystem
            "--suppressions-list=${XWALK_CPPCHECK_SUPPRESSIONS}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM)
endif()

if(XWALK_ENABLE_PACKAGING)
    if(NOT XWALK_BUILD_RPI)
        message(FATAL_ERROR "Debian packaging requires XWALK_BUILD_RPI=ON")
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
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "espeak-ng, i2c-tools, gpiod")
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "rpicam-apps | ffmpeg")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkTool/deployment/debian/conffiles")
    include(CPack)
endif()
