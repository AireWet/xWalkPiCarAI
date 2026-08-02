cmake_minimum_required(VERSION 3.16)

project(xWalkHal VERSION 1.0.0 LANGUAGES CXX)

option(XWALK_HAL_BUILD_HOST
    "Build every xWalk HAL module and deterministic host test"
    OFF)
option(XWALK_HAL_BUILD_RPI
    "Build every xWalk HAL module, Linux backend, and hardware-labelled executable"
    OFF)
option(XWALK_HAL_BUILD_CLI
    "Build the xWalk HAL terminal application"
    OFF)

if(XWALK_HAL_BUILD_HOST AND XWALK_HAL_BUILD_RPI)
    message(FATAL_ERROR
        "Host and RPI verification require separate build directories; enable only one aggregate flag")
endif()

if(XWALK_HAL_BUILD_RPI AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR
        "XWALK_HAL_BUILD_RPI requires Linux headers and can be cross-compiled or built on Ubuntu")
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
            "The RPI build requires Linux GPIO, I2C, and SPI UAPI headers; install linux-libc-dev")
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
    XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS
)

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
    XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS
)

foreach(host_option_name IN LISTS XWALK_HAL_HOST_OPTION_NAMES)
    set(${host_option_name} ${XWALK_HAL_BUILD_HOST} CACHE BOOL
        "Controlled by XWALK_HAL_BUILD_HOST" FORCE)
    mark_as_advanced(${host_option_name})
endforeach()

foreach(rpi_option_name IN LISTS XWALK_HAL_RPI_OPTION_NAMES)
    set(${rpi_option_name} ${XWALK_HAL_BUILD_RPI} CACHE BOOL
        "Controlled by XWALK_HAL_BUILD_RPI" FORCE)
    mark_as_advanced(${rpi_option_name})
endforeach()

if(XWALK_HAL_BUILD_HOST OR XWALK_HAL_BUILD_RPI)
    set(BUILD_TESTING ON CACHE BOOL "Enable xWalk HAL aggregate tests" FORCE)
    include(CTest)
endif()

set(XWALK_GPT_BUILD_VOSK_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Vosk provider in the HAL RPi aggregate" FORCE)
set(XWALK_GPT_BUILD_ESPEAK_PROVIDER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build the Espeak provider in the HAL RPi aggregate" FORCE)
set(XWALK_GPT_BUILD_STT_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA speech capture in the HAL RPi aggregate" FORCE)
set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA speech playback in the HAL RPi aggregate" FORCE)
set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build ALSA music in the HAL RPi aggregate" FORCE)
set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_HAL_BUILD_RPI} CACHE BOOL
    "Build compressed-audio decoding in the HAL RPi aggregate" FORCE)
mark_as_advanced(XWALK_GPT_BUILD_VOSK_PROVIDER XWALK_GPT_BUILD_ESPEAK_PROVIDER)
mark_as_advanced(
    XWALK_GPT_BUILD_STT_ALSA_BACKEND
    XWALK_GPT_BUILD_TTS_ALSA_BACKEND
    XWALK_MUSIC_BUILD_ALSA_BACKEND
    XWALK_MUSIC_BUILD_SNDFILE_DECODER)

if(XWALK_GPT_BUILD_TTS_ALSA_BACKEND)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Required by the xWalkGPT text-to-speech ALSA backend" FORCE)
endif()

if(XWALK_HAL_BUILD_HOST)
    message(STATUS "xWalk HAL host mode: plain ctest runs every registered host and unit test")
elseif(XWALK_HAL_BUILD_RPI)
    message(STATUS "xWalk HAL RPI mode: plain ctest runs every registered hardware test")
else()
    message(STATUS "xWalk HAL production mode: all aggregate verification flags are OFF")
endif()

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCommon
    ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon
)
add_subdirectory(xWalkI2c)
add_subdirectory(xWalkSpi)
add_subdirectory(xWalkGpio)
add_subdirectory(xWalkAudio)
add_subdirectory(xWalkConfig)
add_subdirectory(xWalkTrace)
add_subdirectory(xWalkUtils)
add_subdirectory(xWalkLanguageModel)
add_subdirectory(xWalkMusic)
add_subdirectory(xWalkSpeaker)
add_subdirectory(xWalkPwm)
add_subdirectory(xWalkAdc)
add_subdirectory(xWalkServo)
add_subdirectory(xWalkAdxl345)
add_subdirectory(xWalkLineTracker)
add_subdirectory(xWalkUltrasonic)
add_subdirectory(xWalkMotor)
add_subdirectory(xWalkLed)
add_subdirectory(xWalkBuzzer)
add_subdirectory(xWalkCamera)
add_subdirectory(xWalkUserButton)
add_subdirectory(xWalkBoardControl)
add_subdirectory(xWalkRobot)
add_subdirectory(xWalkGPT)
add_subdirectory(xWalkVoiceAssistant)

if(XWALK_HAL_BUILD_CLI OR XWALK_HAL_BUILD_HOST OR XWALK_HAL_BUILD_RPI)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCLI
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCLI
    )
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
    xWalkCommon
    xWalkConfig
    xWalkGpio
    xWalkGPT
    xWalkI2c
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
    xWalkVoiceAssistant
)

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
        xWalkUtilsLinux
    )
endif()
