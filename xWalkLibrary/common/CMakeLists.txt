cmake_minimum_required(VERSION 3.16)
project(xWalkLibraryCommon LANGUAGES CXX)

add_library(xWalkLibraryCommon INTERFACE)

target_include_directories(xWalkLibraryCommon
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(xWalkLibraryCommon INTERFACE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLibraryCommon INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)

    if(XWALK_HAL_BUILD_HOST)
        # Host verification must execute assertions even when the selected
        # configuration normally defines NDEBUG, because existing tests use
        # assertions as both checks and operation boundaries.
        target_compile_options(xWalkLibraryCommon INTERFACE -UNDEBUG)
    endif()

    if(XWALK_ENABLE_STRICT_WARNINGS)
        target_compile_options(xWalkLibraryCommon INTERFACE -Wshadow)
    endif()

    if(XWALK_ENABLE_SANITIZERS)
        target_compile_options(xWalkLibraryCommon INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer -g)
        target_link_options(xWalkLibraryCommon INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()

    if(XWALK_ENABLE_THREAD_SANITIZER)
        target_compile_options(xWalkLibraryCommon INTERFACE
            -fsanitize=thread -fno-omit-frame-pointer -fPIE -g -O1)
        target_link_options(xWalkLibraryCommon INTERFACE
            -fsanitize=thread -fno-omit-frame-pointer -pie)
    endif()

    if(XWALK_ENABLE_COVERAGE)
        if(XWALK_COVERAGE_BACKEND STREQUAL "clang")
            target_compile_options(xWalkLibraryCommon INTERFACE
                -fprofile-instr-generate -fcoverage-mapping -O0 -g)
            target_link_options(xWalkLibraryCommon INTERFACE
                -fprofile-instr-generate -fcoverage-mapping)
        else()
            target_compile_options(xWalkLibraryCommon INTERFACE --coverage -O0 -g)
            target_link_options(xWalkLibraryCommon INTERFACE --coverage)
            target_compile_definitions(xWalkLibraryCommon INTERFACE XWALK_GCC_COVERAGE=1)
        endif()
    endif()
endif()
