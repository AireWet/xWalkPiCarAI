cmake_minimum_required(VERSION 3.16)
project(xWalkCommon LANGUAGES CXX)

add_library(xWalkCommon INTERFACE)

target_include_directories(xWalkCommon
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(xWalkCommon INTERFACE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCommon INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)

    if(XWALK_HAL_BUILD_HOST)
        # Host verification must execute assertions even when the selected
        # configuration normally defines NDEBUG, because existing tests use
        # assertions as both checks and operation boundaries.
        target_compile_options(xWalkCommon INTERFACE -UNDEBUG)
    endif()

    if(XWALK_ENABLE_STRICT_WARNINGS)
        target_compile_options(xWalkCommon INTERFACE -Wshadow)
    endif()

    if(XWALK_ENABLE_SANITIZERS)
        target_compile_options(xWalkCommon INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(xWalkCommon INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()

    if(XWALK_ENABLE_THREAD_SANITIZER)
        target_compile_options(xWalkCommon INTERFACE
            -fsanitize=thread -fno-omit-frame-pointer)
        target_link_options(xWalkCommon INTERFACE
            -fsanitize=thread -fno-omit-frame-pointer)
    endif()

    if(XWALK_ENABLE_COVERAGE)
        target_compile_options(xWalkCommon INTERFACE --coverage -O0 -g)
        target_link_options(xWalkCommon INTERFACE --coverage)
    endif()
endif()
