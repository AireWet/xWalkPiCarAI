cmake_minimum_required(VERSION 3.16)

project(xWalkSoundBackgroundMusic LANGUAGES CXX)

if(NOT TARGET xWalkMusic)
    if(NOT DEFINED XWALK_MUSIC_BUILD_HOST_TESTS)
        set(XWALK_MUSIC_BUILD_HOST_TESTS OFF CACHE BOOL
            "Sound-background-music dependency setting")
    endif()
    if(NOT DEFINED XWALK_MUSIC_BUILD_ALSA_BACKEND)
        set(XWALK_MUSIC_BUILD_ALSA_BACKEND OFF CACHE BOOL
            "Sound-background-music dependency setting")
    endif()
    if(NOT DEFINED XWALK_MUSIC_BUILD_SNDFILE_DECODER)
        set(XWALK_MUSIC_BUILD_SNDFILE_DECODER OFF CACHE BOOL
            "Sound-background-music dependency setting")
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/layer1/xWalkMusic
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkMusic)
endif()

add_library(xWalkSoundBackgroundMusic STATIC
    src/xAgent_Rpi5CarSoundBackgroundMusic.cpp
    src/xAgent_Rpi5CarSoundBackgroundMusicLifecycle.cpp)
target_link_libraries(xWalkSoundBackgroundMusic PRIVATE xWalkTrace)
add_library(xWalk::SoundBackgroundMusic ALIAS xWalkSoundBackgroundMusic)
target_include_directories(xWalkSoundBackgroundMusic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSoundBackgroundMusic PUBLIC cxx_std_17)
target_link_libraries(xWalkSoundBackgroundMusic PUBLIC xWalkMusic)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSoundBackgroundMusic PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
