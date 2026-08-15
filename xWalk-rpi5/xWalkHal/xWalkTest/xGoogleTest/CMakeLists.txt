cmake_minimum_required(VERSION 3.16)

project(xGoogleTest LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

find_package(GTest CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

set(XWALK_HAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(XWALK_WORKSPACE_SOURCE_DIRECTORY ${XWALK_HAL_SOURCE_DIRECTORY}/..)
include(${XWALK_WORKSPACE_SOURCE_DIRECTORY}/xWalkLibrary/VoskModel.cmake)

set(XWALK_GOOGLE_TEST_HOST_CONFIG_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/config/test_config.xml)
set(XWALK_GOOGLE_TEST_HOST_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/test_config.xml)
set(XWALK_GOOGLE_TEST_HARDWARE_CONFIG_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/config/hardware_test_config.xml)
set(XWALK_GOOGLE_TEST_HARDWARE_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/hardware_test_config.xml)
configure_file(
    ${XWALK_GOOGLE_TEST_HOST_CONFIG_SOURCE}
    ${XWALK_GOOGLE_TEST_HOST_CONFIG_OUTPUT}
    COPYONLY)
configure_file(
    ${XWALK_GOOGLE_TEST_HARDWARE_CONFIG_SOURCE}
    ${XWALK_GOOGLE_TEST_HARDWARE_CONFIG_OUTPUT}
    COPYONLY)
configure_file(config/xHal_Rpi5CarGoogleTestConfig.yml
    ${CMAKE_BINARY_DIR}/xHal_Rpi5CarGoogleTestConfig.yml @ONLY)

add_executable(xGoogleTest
    src/main.cpp
    src/TestConfig.cpp
    src/TestRunner.cpp
    src/xHal_Rpi5CarSimulationArgumentsTest.cpp
)
target_link_libraries(xGoogleTest PRIVATE xWalkTrace)
set_target_properties(xGoogleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
target_include_directories(xGoogleTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/simulation/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/simulation/include
)
target_compile_features(xGoogleTest PRIVATE cxx_std_17)
if(XWALK_ENABLE_THREAD_SANITIZER)
    target_compile_definitions(xGoogleTest PRIVATE
        XWALK_GOOGLE_TEST_THREAD_SANITIZER=1)
endif()

function(xwalk_add_legacy_test_source object_target entry_point source_path)
    add_library(${object_target} OBJECT ${source_path})
    get_filename_component(XWALK_LEGACY_TEST_SOURCE_DIRECTORY
        "${source_path}" DIRECTORY)
    get_filename_component(XWALK_LEGACY_TEST_SOURCE_DIRECTORY
        "${XWALK_LEGACY_TEST_SOURCE_DIRECTORY}" ABSOLUTE)
    string(REGEX REPLACE "/src$" "/include"
        XWALK_LEGACY_TEST_INCLUDE_DIRECTORY
        "${XWALK_LEGACY_TEST_SOURCE_DIRECTORY}")
    target_compile_features(${object_target} PRIVATE cxx_std_17)
    target_compile_definitions(${object_target} PRIVATE main=${entry_point})
    target_include_directories(${object_target} PRIVATE
        ${XWALK_LEGACY_TEST_INCLUDE_DIRECTORY}
        ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/include)
    target_link_libraries(${object_target} PRIVATE xWalkTrace ${ARGN})
    target_sources(xGoogleTest PRIVATE $<TARGET_OBJECTS:${object_target}>)
endfunction()

if(NOT XWALK_HAL_BUILD_RPI)
    set(XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY
        ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_GOOGLE_TEST_I2C_TRACE_GENERATOR
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/simulation/config/xHal_Rpi5CarI2cTraceConfig.py)
    set(XWALK_GOOGLE_TEST_I2C_TRACE_CONFIG_PATH
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkI2cTrace.xml)
    add_custom_command(
        OUTPUT ${XWALK_GOOGLE_TEST_I2C_TRACE_CONFIG_PATH}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_I2C_TRACE_GENERATOR}
            --input ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            --output ${XWALK_GOOGLE_TEST_I2C_TRACE_CONFIG_PATH}
        DEPENDS ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            ${XWALK_GOOGLE_TEST_I2C_TRACE_GENERATOR}
        COMMENT "Preparing the persistent centralized I2C trace catalogue"
        VERBATIM)
    add_custom_target(xGoogleTestI2cTraceConfig
        DEPENDS ${XWALK_GOOGLE_TEST_I2C_TRACE_CONFIG_PATH})
    add_dependencies(xGoogleTestI2cTraceConfig xWalkTraceMetadata)

    add_library(xGoogleTestI2c OBJECT
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/simulation/src/xHal_Rpi5CarI2cHostStub.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/simulation/src/xHal_Rpi5CarI2cSimulationArguments.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/test/src/xHal_Rpi5CarI2cTest.cpp
    )
    target_compile_features(xGoogleTestI2c PRIVATE cxx_std_17)
    target_compile_definitions(xGoogleTestI2c PRIVATE
        XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_I2C_TRACE_CONFIG_PATH}"
        XWALK_I2C_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkI2cTest.log"
    )
    target_include_directories(xGoogleTestI2c PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/simulation/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/hardware/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkI2c/core/include
    )
    target_link_libraries(xGoogleTestI2c PRIVATE GTest::gtest xWalkI2cLinux xWalkTrace)
    add_dependencies(xGoogleTestI2c xGoogleTestI2cTraceConfig)
    target_sources(xGoogleTest PRIVATE $<TARGET_OBJECTS:xGoogleTestI2c>)

    set(XWALK_GOOGLE_TEST_SPI_TRACE_GENERATOR
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/simulation/config/xHal_Rpi5CarSpiTraceConfig.py)
    set(XWALK_GOOGLE_TEST_SPI_TRACE_CONFIG_PATH
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkSpiTrace.xml)
    add_custom_command(
        OUTPUT ${XWALK_GOOGLE_TEST_SPI_TRACE_CONFIG_PATH}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_SPI_TRACE_GENERATOR}
            --input ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            --output ${XWALK_GOOGLE_TEST_SPI_TRACE_CONFIG_PATH}
        DEPENDS ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            ${XWALK_GOOGLE_TEST_SPI_TRACE_GENERATOR}
        COMMENT "Preparing the persistent centralized SPI trace catalogue"
        VERBATIM)
    add_custom_target(xGoogleTestSpiTraceConfig
        DEPENDS ${XWALK_GOOGLE_TEST_SPI_TRACE_CONFIG_PATH})
    add_dependencies(xGoogleTestSpiTraceConfig xWalkTraceMetadata)

    add_library(xGoogleTestSpi OBJECT
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/simulation/src/xHal_Rpi5CarSpiHostStub.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/simulation/src/xHal_Rpi5CarSpiSimulationArguments.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/test/src/xHal_Rpi5CarSpiTest.cpp
    )
    target_compile_features(xGoogleTestSpi PRIVATE cxx_std_17)
    target_compile_definitions(xGoogleTestSpi PRIVATE
        XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_SPI_TRACE_CONFIG_PATH}"
        XWALK_SPI_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkSpiTest.log"
    )
    target_include_directories(xGoogleTestSpi PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/simulation/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/hardware/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkSpi/core/include
    )
    target_link_libraries(xGoogleTestSpi PRIVATE GTest::gtest xWalkSpiLinux xWalkTrace)
    add_dependencies(xGoogleTestSpi xGoogleTestSpiTraceConfig)
    target_sources(xGoogleTest PRIVATE $<TARGET_OBJECTS:xGoogleTestSpi>)

    set(XWALK_GOOGLE_TEST_GPIO_TRACE_GENERATOR
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/simulation/config/xHal_Rpi5CarGpioTraceConfig.py)
    set(XWALK_GOOGLE_TEST_GPIO_TRACE_CONFIG_PATH
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGpioTrace.xml)
    add_custom_command(
        OUTPUT ${XWALK_GOOGLE_TEST_GPIO_TRACE_CONFIG_PATH}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_GPIO_TRACE_GENERATOR}
            --input ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            --output ${XWALK_GOOGLE_TEST_GPIO_TRACE_CONFIG_PATH}
        DEPENDS ${XWALK_GOOGLE_TEST_I2C_TRACE_INVENTORY}
            ${XWALK_GOOGLE_TEST_GPIO_TRACE_GENERATOR}
        COMMENT "Preparing the persistent centralized GPIO trace catalogue"
        VERBATIM)
    add_custom_target(xGoogleTestGpioTraceConfig
        DEPENDS ${XWALK_GOOGLE_TEST_GPIO_TRACE_CONFIG_PATH})
    add_dependencies(xGoogleTestGpioTraceConfig xWalkTraceMetadata)

    add_library(xGoogleTestGpio OBJECT
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/simulation/src/xHal_Rpi5CarGpioHostStub.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/simulation/src/xHal_Rpi5CarGpioSimulationArguments.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/test/src/xHal_Rpi5CarGpioTest.cpp
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/test/src/xHal_Rpi5CarGpioTestSupport.cpp
    )
    target_compile_features(xGoogleTestGpio PRIVATE cxx_std_17)
    target_compile_definitions(xGoogleTestGpio PRIVATE
        XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_GPIO_TRACE_CONFIG_PATH}"
        XWALK_GPIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkGpioTest.log"
    )
    target_include_directories(xGoogleTestGpio PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/simulation/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/hardware/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkGpio/core/include
    )
    target_link_libraries(xGoogleTestGpio PRIVATE GTest::gtest xWalkGpioLinux xWalkTrace)
    add_dependencies(xGoogleTestGpio xGoogleTestGpioTraceConfig)
    target_sources(xGoogleTest PRIVATE $<TARGET_OBJECTS:xGoogleTestGpio>)
endif()
set(XWALK_GOOGLE_TEST_AUDIO_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_AUDIO_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/config/xHal_Rpi5CarAudioTraceConfig.py)
set(XWALK_GOOGLE_TEST_AUDIO_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAudioTrace.xml)
add_custom_command(
    OUTPUT ${XWALK_GOOGLE_TEST_AUDIO_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_AUDIO_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_AUDIO_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_AUDIO_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_AUDIO_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_AUDIO_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Audio trace catalogue"
    VERBATIM)
add_custom_target(xGoogleTestAudioTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_AUDIO_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestAudioTraceConfig xWalkTraceMetadata)

xwalk_add_legacy_test_source(xGoogleTestAudioAlsa xWalkAudioAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/hardware/test/src/xHal_Rpi5CarAudioAlsaTest.cpp
    xWalkAudioAlsa)
target_sources(xGoogleTestAudioAlsa PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/hardware/test/src/xHal_Rpi5CarAudioAlsaTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/src/xHal_Rpi5CarAudioHandler.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/src/xHal_Rpi5CarAudioHostStub.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/src/xHal_Rpi5CarAudioSimulationArguments.cpp)
target_include_directories(xGoogleTestAudioAlsa PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/hardware/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkAudio/simulation/include)
target_compile_definitions(xGoogleTestAudioAlsa PRIVATE
    XWALK_AUDIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_AUDIO_TRACE_CONFIG_PATH}"
    XWALK_AUDIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkAudioTest.log")
target_link_libraries(xGoogleTestAudioAlsa PRIVATE xWalkTrace)
add_dependencies(xGoogleTestAudioAlsa xGoogleTestAudioTraceConfig)
set(XWALK_GOOGLE_TEST_CONFIG_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_CONFIG_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/simulation/config/xHal_Rpi5CarConfigTraceConfig.py)
set(XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkConfigTrace.xml)
add_custom_command(
    OUTPUT ${XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_CONFIG_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_CONFIG_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_CONFIG_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_CONFIG_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Config trace catalogue"
    VERBATIM)
add_custom_target(xGoogleTestConfigTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestConfigTraceConfig xWalkTraceMetadata)

xwalk_add_legacy_test_source(xGoogleTestConfig xWalkConfigLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/test/src/xHal_Rpi5CarConfigTest.cpp
    xWalkConfig)
target_sources(xGoogleTestConfig PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/test/src/xHal_Rpi5CarConfigTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/simulation/src/xHal_Rpi5CarConfigSimulationArguments.cpp)
target_include_directories(xGoogleTestConfig PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/simulation/include)
target_compile_definitions(xGoogleTestConfig PRIVATE
    XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH}"
    XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkConfigTest.log")
target_link_libraries(xGoogleTestConfig PRIVATE xWalkTrace)
add_dependencies(xGoogleTestConfig xGoogleTestConfigTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestConfigStore xWalkConfigStoreLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/test/src/xHal_Rpi5CarConfigStoreTest.cpp
    xWalkConfig)
target_include_directories(xGoogleTestConfigStore PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkConfig/simulation/include)
target_compile_definitions(xGoogleTestConfigStore PRIVATE
    XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_CONFIG_TRACE_CONFIG_PATH}"
    XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkConfigStoreTest.log")
target_link_libraries(xGoogleTestConfigStore PRIVATE xWalkTrace)
add_dependencies(xGoogleTestConfigStore xGoogleTestConfigTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestTrace xWalkTraceLegacyMain
    ${XWALK_WORKSPACE_SOURCE_DIRECTORY}/xWalkTrace/test/src/xHal_Rpi5CarTraceTest.cpp
    xWalkTrace)
set(XWALK_GOOGLE_TEST_UTILS_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_UTILS_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/simulation/config/xHal_Rpi5CarUtilsTraceConfig.py)
set(XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUtilsTrace.xml)
add_custom_command(
    OUTPUT ${XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_UTILS_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_UTILS_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_UTILS_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_UTILS_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Utils trace catalogue" VERBATIM)
add_custom_target(xGoogleTestUtilsTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestUtilsTraceConfig xWalkTraceMetadata)

xwalk_add_legacy_test_source(xGoogleTestUtils xWalkUtilsLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/test/src/xHal_Rpi5CarUtilsTest.cpp
    xWalkUtils)
target_sources(xGoogleTestUtils PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/test/src/xHal_Rpi5CarUtilsTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/simulation/src/xHal_Rpi5CarUtilsSimulationArguments.cpp)
target_include_directories(xGoogleTestUtils PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/simulation/include)
target_compile_definitions(xGoogleTestUtils PRIVATE
    XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH}"
    XWALK_UTILS_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkUtilsTest.log")
target_link_libraries(xGoogleTestUtils PRIVATE xWalkTrace)
add_dependencies(xGoogleTestUtils xGoogleTestUtilsTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestUtilsLinux xWalkUtilsLinuxLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/hardware/test/src/xHal_Rpi5CarUtilsLinuxTest.cpp
    xWalkUtilsLinux)
target_include_directories(xGoogleTestUtilsLinux PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkUtils/simulation/include)
target_compile_definitions(xGoogleTestUtilsLinux PRIVATE
    XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_UTILS_TRACE_CONFIG_PATH}"
    XWALK_UTILS_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkUtilsLinuxTest.log")
target_link_libraries(xGoogleTestUtilsLinux PRIVATE xWalkTrace)
add_dependencies(xGoogleTestUtilsLinux xGoogleTestUtilsTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestLanguageModel xWalkLanguageModelLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/test/src/xHal_Rpi5CarLanguageModelTest.cpp
    xWalkLanguageModel)
set(XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/simulation/config/xHal_Rpi5CarLanguageModelTraceConfig.py)
set(XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLanguageModelTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized LanguageModel trace catalogue" VERBATIM)
add_custom_target(xGoogleTestLanguageModelTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestLanguageModelTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestLanguageModel PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/test/src/xHal_Rpi5CarLanguageModelTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/simulation/src/xHal_Rpi5CarLanguageModelSimulationArguments.cpp)
target_include_directories(xGoogleTestLanguageModel PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/simulation/include)
target_compile_definitions(xGoogleTestLanguageModel PRIVATE
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH}"
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkLanguageModelTest.log")
target_link_libraries(xGoogleTestLanguageModel PRIVATE xWalkTrace)
add_dependencies(xGoogleTestLanguageModel xGoogleTestLanguageModelTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestLanguageModelOllama
    xWalkLanguageModelOllamaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/hardware/test/src/xHal_Rpi5CarLanguageModelOllamaTest.cpp
    xWalkLanguageModelOllama)
target_include_directories(xGoogleTestLanguageModelOllama PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/interface/xWalkLanguageModel/simulation/include)
target_compile_definitions(xGoogleTestLanguageModelOllama PRIVATE
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_LANGUAGE_MODEL_TRACE_CONFIG_PATH}"
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkLanguageModelOllamaTest.log")
target_link_libraries(xGoogleTestLanguageModelOllama PRIVATE xWalkTrace)
add_dependencies(xGoogleTestLanguageModelOllama xGoogleTestLanguageModelTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestMusic xWalkMusicLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/test/src/xHal_Rpi5CarMusicTest.cpp
    xWalkMusic)
set(XWALK_GOOGLE_TEST_MUSIC_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_MUSIC_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/simulation/config/xHal_Rpi5CarMusicTraceConfig.py)
set(XWALK_GOOGLE_TEST_MUSIC_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMusicTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_MUSIC_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_MUSIC_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_MUSIC_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_MUSIC_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_MUSIC_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_MUSIC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Music trace catalogue" VERBATIM)
add_custom_target(xGoogleTestMusicTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_MUSIC_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestMusicTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestMusic PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/test/src/xHal_Rpi5CarMusicTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/simulation/src/xHal_Rpi5CarMusicSimulationArguments.cpp)
target_include_directories(xGoogleTestMusic PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/simulation/include)
target_compile_definitions(xGoogleTestMusic PRIVATE
    XWALK_MUSIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_MUSIC_TRACE_CONFIG_PATH}"
    XWALK_MUSIC_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkMusicTest.log")
target_link_libraries(xGoogleTestMusic PRIVATE xWalkTrace)
add_dependencies(xGoogleTestMusic xGoogleTestMusicTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestMusicAlsa xWalkMusicAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkMusic/hardware/test/src/xHal_Rpi5CarMusicAlsaTest.cpp
    xWalkMusicAlsa)
xwalk_add_legacy_test_source(xGoogleTestSpeaker xWalkSpeakerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/test/src/xHal_Rpi5CarSpeakerTest.cpp
    xWalkSpeaker)
set(XWALK_GOOGLE_TEST_SPEAKER_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_SPEAKER_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/simulation/config/xHal_Rpi5CarSpeakerTraceConfig.py)
set(XWALK_GOOGLE_TEST_SPEAKER_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkSpeakerTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Speaker trace catalogue" VERBATIM)
add_custom_target(xGoogleTestSpeakerTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_SPEAKER_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestSpeakerTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestSpeaker PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/test/src/xHal_Rpi5CarSpeakerTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/simulation/src/xHal_Rpi5CarSpeakerSimulationArguments.cpp)
target_include_directories(xGoogleTestSpeaker PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/simulation/include)
target_compile_definitions(xGoogleTestSpeaker PRIVATE
    XWALK_SPEAKER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_SPEAKER_TRACE_CONFIG_PATH}"
    XWALK_SPEAKER_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkSpeakerTest.log")
target_link_libraries(xGoogleTestSpeaker PRIVATE xWalkTrace)
add_dependencies(xGoogleTestSpeaker xGoogleTestSpeakerTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestSpeakerAlsa xWalkSpeakerAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkSpeaker/hardware/test/src/xHal_Rpi5CarSpeakerAlsaTest.cpp
    xWalkSpeakerAlsa)
xwalk_add_legacy_test_source(xGoogleTestPwm xWalkPwmLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestMain.cpp
    xWalkPwm)
set(XWALK_GOOGLE_TEST_PWM_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_PWM_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/simulation/config/xHal_Rpi5CarPwmTraceConfig.py)
set(XWALK_GOOGLE_TEST_PWM_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkPwmTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_PWM_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_PWM_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_PWM_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_PWM_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_PWM_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_PWM_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized PWM trace catalogue" VERBATIM)
add_custom_target(xGoogleTestPwmTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_PWM_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestPwmTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestPwm PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestTrace.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/simulation/src/xHal_Rpi5CarPwmSimulationArguments.cpp)
target_include_directories(xGoogleTestPwm PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/simulation/include)
target_compile_definitions(xGoogleTestPwm PRIVATE
    XWALK_PWM_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_PWM_TRACE_CONFIG_PATH}"
    XWALK_PWM_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkPwmTest.log")
target_link_libraries(xGoogleTestPwm PRIVATE xWalkTrace)
add_dependencies(xGoogleTestPwm xGoogleTestPwmTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestAdc xWalkAdcLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/test/src/xHal_Rpi5CarAdcTest.cpp
    xWalkAdc)
set(XWALK_GOOGLE_TEST_ADC_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_ADC_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/simulation/config/xHal_Rpi5CarAdcTraceConfig.py)
set(XWALK_GOOGLE_TEST_ADC_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdcTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_ADC_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_ADC_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_ADC_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_ADC_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_ADC_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_ADC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized ADC trace catalogue" VERBATIM)
add_custom_target(xGoogleTestAdcTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_ADC_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestAdcTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestAdc PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/test/src/xHal_Rpi5CarAdcTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/simulation/src/xHal_Rpi5CarAdcSimulationArguments.cpp)
target_include_directories(xGoogleTestAdc PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdc/simulation/include)
target_compile_definitions(xGoogleTestAdc PRIVATE
    XWALK_ADC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_ADC_TRACE_CONFIG_PATH}"
    XWALK_ADC_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkAdcTest.log")
target_link_libraries(xGoogleTestAdc PRIVATE xWalkTrace)
add_dependencies(xGoogleTestAdc xGoogleTestAdcTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestServo xWalkServoLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestMain.cpp
    xWalkServo)
set(XWALK_GOOGLE_TEST_SERVO_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_SERVO_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/simulation/config/xHal_Rpi5CarServoTraceConfig.py)
set(XWALK_GOOGLE_TEST_SERVO_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkServoTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_SERVO_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_SERVO_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_SERVO_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_SERVO_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_SERVO_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_SERVO_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Servo trace catalogue" VERBATIM)
add_custom_target(xGoogleTestServoTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_SERVO_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestServoTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestServo PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestTrace.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/simulation/src/xHal_Rpi5CarServoSimulationArguments.cpp)
target_include_directories(xGoogleTestServo PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/simulation/include)
target_compile_definitions(xGoogleTestServo PRIVATE
    XWALK_SERVO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_SERVO_TRACE_CONFIG_PATH}"
    XWALK_SERVO_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkServoTest.log")
target_link_libraries(xGoogleTestServo PRIVATE xWalkTrace)
add_dependencies(xGoogleTestServo xGoogleTestServoTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestAdxl345 xWalkAdxl345LegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/test/src/xHal_Rpi5CarAdxl345Test.cpp
    xWalkAdxl345)
set(XWALK_GOOGLE_TEST_ADXL345_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_ADXL345_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/simulation/config/xHal_Rpi5CarAdxl345TraceConfig.py)
set(XWALK_GOOGLE_TEST_ADXL345_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdxl345Trace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_ADXL345_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_ADXL345_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_ADXL345_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_ADXL345_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_ADXL345_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_ADXL345_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized ADXL345 trace catalogue" VERBATIM)
add_custom_target(xGoogleTestAdxl345TraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_ADXL345_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestAdxl345TraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestAdxl345 PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/test/src/xHal_Rpi5CarAdxl345TestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/simulation/src/xHal_Rpi5CarAdxl345SimulationArguments.cpp)
target_include_directories(xGoogleTestAdxl345 PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkAdxl345/simulation/include)
target_compile_definitions(xGoogleTestAdxl345 PRIVATE
    XWALK_ADXL345_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_ADXL345_TRACE_CONFIG_PATH}"
    XWALK_ADXL345_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkAdxl345Test.log")
target_link_libraries(xGoogleTestAdxl345 PRIVATE xWalkTrace)
add_dependencies(xGoogleTestAdxl345 xGoogleTestAdxl345TraceConfig)
xwalk_add_legacy_test_source(xGoogleTestLineTracker xWalkLineTrackerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/test/src/xHal_Rpi5CarLineTrackerTest.cpp
    xWalkLineTracker)
set(XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/simulation/config/xHal_Rpi5CarLineTrackerTraceConfig.py)
set(XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLineTrackerTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized LineTracker trace catalogue" VERBATIM)
add_custom_target(xGoogleTestLineTrackerTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestLineTrackerTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestLineTracker PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/test/src/xHal_Rpi5CarLineTrackerTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/simulation/src/xHal_Rpi5CarLineTrackerSimulationArguments.cpp)
target_include_directories(xGoogleTestLineTracker PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLineTracker/simulation/include)
target_compile_definitions(xGoogleTestLineTracker PRIVATE
    XWALK_LINE_TRACKER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_LINE_TRACKER_TRACE_CONFIG_PATH}"
    XWALK_LINE_TRACKER_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkLineTrackerTest.log")
target_link_libraries(xGoogleTestLineTracker PRIVATE xWalkTrace)
add_dependencies(xGoogleTestLineTracker xGoogleTestLineTrackerTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestUltrasonic xWalkUltrasonicLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/test/src/xHal_Rpi5CarUltrasonicTest.cpp
    xWalkUltrasonic)
set(XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/simulation/config/xHal_Rpi5CarUltrasonicTraceConfig.py)
set(XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUltrasonicTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Ultrasonic trace catalogue" VERBATIM)
add_custom_target(xGoogleTestUltrasonicTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestUltrasonicTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestUltrasonic PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/test/src/xHal_Rpi5CarUltrasonicTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/simulation/src/xHal_Rpi5CarUltrasonicSimulationArguments.cpp)
target_include_directories(xGoogleTestUltrasonic PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUltrasonic/simulation/include)
target_compile_definitions(xGoogleTestUltrasonic PRIVATE
    XWALK_ULTRASONIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_ULTRASONIC_TRACE_CONFIG_PATH}"
    XWALK_ULTRASONIC_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkUltrasonicTest.log")
target_link_libraries(xGoogleTestUltrasonic PRIVATE xWalkTrace)
add_dependencies(xGoogleTestUltrasonic xGoogleTestUltrasonicTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestMotor xWalkMotorLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/test/src/xHal_Rpi5CarMotorTest.cpp
    xWalkMotor)
set(XWALK_GOOGLE_TEST_MOTOR_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_MOTOR_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/simulation/config/xHal_Rpi5CarMotorTraceConfig.py)
set(XWALK_GOOGLE_TEST_MOTOR_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMotorTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_MOTOR_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_MOTOR_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_MOTOR_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_MOTOR_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_MOTOR_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_MOTOR_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Motor trace catalogue" VERBATIM)
add_custom_target(xGoogleTestMotorTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_MOTOR_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestMotorTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestMotor PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/test/src/xHal_Rpi5CarMotorTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/simulation/src/xHal_Rpi5CarMotorSimulationArguments.cpp)
target_include_directories(xGoogleTestMotor PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkMotor/simulation/include)
target_compile_definitions(xGoogleTestMotor PRIVATE
    XWALK_MOTOR_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_MOTOR_TRACE_CONFIG_PATH}"
    XWALK_MOTOR_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkMotorTest.log")
target_link_libraries(xGoogleTestMotor PRIVATE xWalkTrace)
add_dependencies(xGoogleTestMotor xGoogleTestMotorTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestLed xWalkLedLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/test/src/xHal_Rpi5CarLedTest.cpp
    xWalkLed)
xwalk_add_legacy_test_source(xGoogleTestRgbLed xWalkRgbLedLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/test/src/xHal_Rpi5CarRgbLedTest.cpp
    xWalkLed)
set(XWALK_GOOGLE_TEST_LED_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_LED_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/simulation/config/xHal_Rpi5CarLedTraceConfig.py)
set(XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLedTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_LED_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_LED_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_LED_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_LED_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized LED trace catalogue" VERBATIM)
add_custom_target(xGoogleTestLedTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestLedTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestLed PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/test/src/xHal_Rpi5CarLedTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/simulation/src/xHal_Rpi5CarLedSimulationArguments.cpp)
target_include_directories(xGoogleTestLed PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/simulation/include)
target_include_directories(xGoogleTestRgbLed PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkLed/simulation/include)
target_compile_definitions(xGoogleTestLed PRIVATE
    XWALK_LED_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH}"
    XWALK_LED_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkLedTest.log")
target_compile_definitions(xGoogleTestRgbLed PRIVATE
    XWALK_LED_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_LED_TRACE_CONFIG_PATH}"
    XWALK_LED_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkLedTest.log")
target_link_libraries(xGoogleTestLed PRIVATE xWalkTrace)
target_link_libraries(xGoogleTestRgbLed PRIVATE xWalkTrace)
add_dependencies(xGoogleTestLed xGoogleTestLedTraceConfig)
add_dependencies(xGoogleTestRgbLed xGoogleTestLedTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestBuzzer xWalkBuzzerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/test/src/xHal_Rpi5CarBuzzerTest.cpp
    xWalkBuzzer)
set(XWALK_GOOGLE_TEST_BUZZER_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_BUZZER_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/simulation/config/xHal_Rpi5CarBuzzerTraceConfig.py)
set(XWALK_GOOGLE_TEST_BUZZER_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBuzzerTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_BUZZER_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_BUZZER_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_BUZZER_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_BUZZER_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_BUZZER_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_BUZZER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Buzzer trace catalogue" VERBATIM)
add_custom_target(xGoogleTestBuzzerTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_BUZZER_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestBuzzerTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestBuzzer PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/test/src/xHal_Rpi5CarBuzzerTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/simulation/src/xHal_Rpi5CarBuzzerSimulationArguments.cpp)
target_include_directories(xGoogleTestBuzzer PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/sensor/xWalkBuzzer/simulation/include)
target_compile_definitions(xGoogleTestBuzzer PRIVATE
    XWALK_BUZZER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_BUZZER_TRACE_CONFIG_PATH}"
    XWALK_BUZZER_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkBuzzerTest.log")
target_link_libraries(xGoogleTestBuzzer PRIVATE xWalkTrace)
add_dependencies(xGoogleTestBuzzer xGoogleTestBuzzerTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestCamera xWalkCameraLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/test/src/xHal_Rpi5CarCameraTest.cpp
    xWalkCamera)
set(XWALK_GOOGLE_TEST_CAMERA_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_CAMERA_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/simulation/config/xHal_Rpi5CarCameraTraceConfig.py)
set(XWALK_GOOGLE_TEST_CAMERA_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkCameraTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_CAMERA_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_CAMERA_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_CAMERA_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_CAMERA_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_CAMERA_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_CAMERA_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Camera trace catalogue" VERBATIM)
add_custom_target(xGoogleTestCameraTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_CAMERA_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestCameraTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestCamera PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/test/src/xHal_Rpi5CarCameraTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/simulation/src/xHal_Rpi5CarCameraSimulationArguments.cpp)
target_include_directories(xGoogleTestCamera PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkCamera/simulation/include)
target_compile_definitions(xGoogleTestCamera PRIVATE
    XWALK_CAMERA_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_CAMERA_TRACE_CONFIG_PATH}"
    XWALK_CAMERA_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkCameraTest.log")
target_link_libraries(xGoogleTestCamera PRIVATE xWalkTrace)
add_dependencies(xGoogleTestCamera xGoogleTestCameraTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestUserButton xWalkUserButtonLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/test/src/xHal_Rpi5CarUserButtonTest.cpp
    xWalkUserButton)
set(XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/simulation/config/xHal_Rpi5CarUserButtonTraceConfig.py)
set(XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUserButtonTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized UserButton trace catalogue" VERBATIM)
add_custom_target(xGoogleTestUserButtonTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestUserButtonTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestUserButton PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/test/src/xHal_Rpi5CarUserButtonTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/simulation/src/xHal_Rpi5CarUserButtonSimulationArguments.cpp)
target_include_directories(xGoogleTestUserButton PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkUserButton/simulation/include)
target_compile_definitions(xGoogleTestUserButton PRIVATE
    XWALK_USER_BUTTON_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_USER_BUTTON_TRACE_CONFIG_PATH}"
    XWALK_USER_BUTTON_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkUserButtonTest.log")
target_link_libraries(xGoogleTestUserButton PRIVATE xWalkTrace)
add_dependencies(xGoogleTestUserButton xGoogleTestUserButtonTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestBoardControl xWalkBoardControlLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/test/src/xHal_Rpi5CarBoardControlTest.cpp
    xWalkBoardControl)
xwalk_add_legacy_test_source(xGoogleTestDevice xWalkDeviceLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/test/src/xHal_Rpi5CarDeviceTest.cpp
    xWalkBoardControl)
xwalk_add_legacy_test_source(xGoogleTestFirmwareInfo xWalkFirmwareInfoLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/test/src/xHal_Rpi5CarFirmwareInfoTest.cpp
    xWalkBoardControl)
set(XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/simulation/config/xHal_Rpi5CarBoardControlTraceConfig.py)
set(XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBoardControlTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized BoardControl trace catalogue" VERBATIM)
add_custom_target(xGoogleTestBoardControlTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestBoardControlTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestBoardControl PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/test/src/xHal_Rpi5CarBoardControlTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/simulation/src/xHal_Rpi5CarBoardControlSimulationArguments.cpp)
foreach(XWALK_BOARD_CONTROL_TEST_TARGET IN ITEMS
    xGoogleTestBoardControl xGoogleTestDevice xGoogleTestFirmwareInfo)
    target_include_directories(${XWALK_BOARD_CONTROL_TEST_TARGET} PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkBoardControl/simulation/include)
    target_compile_definitions(${XWALK_BOARD_CONTROL_TEST_TARGET} PRIVATE
        XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_BOARD_CONTROL_TRACE_CONFIG_PATH}"
        XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkBoardControlTest.log")
    target_link_libraries(${XWALK_BOARD_CONTROL_TEST_TARGET} PRIVATE xWalkTrace)
    add_dependencies(${XWALK_BOARD_CONTROL_TEST_TARGET} xGoogleTestBoardControlTraceConfig)
endforeach()
xwalk_add_legacy_test_source(xGoogleTestRobot xWalkRobotLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/test/src/xHal_Rpi5CarRobotTest.cpp
    xWalkRobot)
set(XWALK_GOOGLE_TEST_ROBOT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_ROBOT_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/simulation/config/xHal_Rpi5CarRobotTraceConfig.py)
set(XWALK_GOOGLE_TEST_ROBOT_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkRobotTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_ROBOT_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_ROBOT_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_ROBOT_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_ROBOT_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_ROBOT_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_ROBOT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized Robot trace catalogue" VERBATIM)
add_custom_target(xGoogleTestRobotTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_ROBOT_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestRobotTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestRobot PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/test/src/xHal_Rpi5CarRobotTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/simulation/src/xHal_Rpi5CarRobotSimulationArguments.cpp)
target_include_directories(xGoogleTestRobot PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkRobot/simulation/include)
target_compile_definitions(xGoogleTestRobot PRIVATE
    XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_ROBOT_TRACE_CONFIG_PATH}"
    XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkRobotTest.log")
target_link_libraries(xGoogleTestRobot PRIVATE xWalkTrace)
add_dependencies(xGoogleTestRobot xGoogleTestRobotTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestSpeechToText xWalkSpeechToTextLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/test/src/xHal_Rpi5CarSpeechToTextTest.cpp
    xWalkGPT)
xwalk_add_legacy_test_source(xGoogleTestTextToSpeech xWalkTextToSpeechLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/test/src/xHal_Rpi5CarTextToSpeechTest.cpp
    xWalkGPT)
set(XWALK_GOOGLE_TEST_GPT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_GPT_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/simulation/config/xHal_Rpi5CarGptTraceConfig.py)
set(XWALK_GOOGLE_TEST_GPT_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGptTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_GPT_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_GPT_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_GPT_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_GPT_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_GPT_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_GPT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized GPT trace catalogue" VERBATIM)
add_custom_target(xGoogleTestGptTraceConfig DEPENDS ${XWALK_GOOGLE_TEST_GPT_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestGptTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestSpeechToText PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/test/src/xHal_Rpi5CarGptTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/simulation/src/xHal_Rpi5CarGptSimulationArguments.cpp)
foreach(XWALK_GPT_TEST_TARGET IN ITEMS xGoogleTestSpeechToText xGoogleTestTextToSpeech)
    target_include_directories(${XWALK_GPT_TEST_TARGET} PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/simulation/include)
    target_compile_definitions(${XWALK_GPT_TEST_TARGET} PRIVATE
        XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_GPT_TRACE_CONFIG_PATH}"
        XWALK_GPT_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkGptTest.log")
    target_link_libraries(${XWALK_GPT_TEST_TARGET} PRIVATE xWalkTrace)
    add_dependencies(${XWALK_GPT_TEST_TARGET} xGoogleTestGptTraceConfig)
endforeach()
xwalk_add_legacy_test_source(xGoogleTestSpeechToTextAlsa xWalkSpeechToTextAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/hardware/test/src/xHal_Rpi5CarSpeechToTextAlsaTest.cpp
    xWalkSpeechToTextAlsa)
xwalk_add_legacy_test_source(xGoogleTestTextToSpeechAlsa xWalkTextToSpeechAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkGPT/hardware/test/src/xHal_Rpi5CarTextToSpeechAlsaTest.cpp
    xWalkTextToSpeechAlsa)
xwalk_add_legacy_test_source(xGoogleTestVoiceAssistant xWalkVoiceAssistantLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/test/src/xHal_Rpi5CarVoiceAssistantTest.cpp
    xWalkVoiceAssistant)
set(XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_GENERATOR
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/simulation/config/xHal_Rpi5CarVoiceAssistantTraceConfig.py)
set(XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkVoiceAssistantTrace.xml)
add_custom_command(OUTPUT ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_CONFIG_PATH}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_GENERATOR}
        --input ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_INVENTORY}
        --output ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_CONFIG_PATH}
    DEPENDS ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_INVENTORY}
        ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent centralized VoiceAssistant trace catalogue" VERBATIM)
add_custom_target(xGoogleTestVoiceAssistantTraceConfig
    DEPENDS ${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_CONFIG_PATH})
add_dependencies(xGoogleTestVoiceAssistantTraceConfig xWalkTraceMetadata)
target_sources(xGoogleTestVoiceAssistant PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/test/src/xHal_Rpi5CarVoiceAssistantTestSupport.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/simulation/src/xHal_Rpi5CarVoiceAssistantSimulationArguments.cpp)
target_include_directories(xGoogleTestVoiceAssistant PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/simulation/include)
target_compile_definitions(xGoogleTestVoiceAssistant PRIVATE
    XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GOOGLE_TEST_VOICE_ASSISTANT_TRACE_CONFIG_PATH}"
    XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH="${CMAKE_BINARY_DIR}/log/xWalkVoiceAssistantTest.log")
target_link_libraries(xGoogleTestVoiceAssistant PRIVATE xWalkTrace)
add_dependencies(xGoogleTestVoiceAssistant xGoogleTestVoiceAssistantTraceConfig)
xwalk_add_legacy_test_source(xGoogleTestVoiceAssistantBackends
    xWalkVoiceAssistantBackendsLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/layer1/xWalkVoiceAssistant/hardware/test/src/xHal_Rpi5CarVoiceAssistantBackendsTest.cpp
    xWalkVoiceAssistant xWalkSpeechToTextAlsa xWalkTextToSpeechAlsa
    xWalkLanguageModelOllama)
xwalk_add_legacy_test_source(xGoogleTestButtonEventSequence
    xWalkButtonEventSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarButtonEventSequenceTest.cpp
    xWalkButtonEventSequence)
xwalk_add_legacy_test_source(xGoogleTestInitAnglesSequence
    xWalkInitAnglesSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarInitAnglesSequenceTest.cpp
    xWalkInitAnglesSequence)
xwalk_add_legacy_test_source(xGoogleTestRobotHat5MotorSequence
    xWalkRobotHat5MotorSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarRobotHat5MotorSequenceTest.cpp
    xWalkRobotHat5MotorSequence)
xwalk_add_legacy_test_source(xGoogleTestMotorSequence
    xWalkMotorSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarMotorSequenceTest.cpp
    xWalkMotorSequence)
xwalk_add_legacy_test_source(xGoogleTestServoHatSequence
    xWalkServoHatSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarServoHatSequenceTest.cpp
    xWalkServoHatSequence)
xwalk_add_legacy_test_source(xGoogleTestServoSequence
    xWalkServoSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarServoSequenceTest.cpp
    xWalkServoSequence)
xwalk_add_legacy_test_source(xGoogleTestPiperStreamSequence
    xWalkPiperStreamSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarPiperStreamSequenceTest.cpp
    xWalkPiperStreamSequence)
xwalk_add_legacy_test_source(xGoogleTestToneSequence
    xWalkToneSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarToneSequenceTest.cpp
    xWalkToneSequence)

target_sources(xGoogleTest PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestAddress.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2c.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2cLifecycle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestOutput.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestTimer.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkPwm/test/src/xHal_Rpi5CarPwmTestValidation.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestAngle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestI2c.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestI2cLifecycle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestInitialization.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestPulse.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/device/xWalkServo/test/src/xHal_Rpi5CarServoTestValidation.cpp
)

target_link_libraries(xGoogleTest PRIVATE
    GTest::gtest
    tinyxml2::tinyxml2
    yaml-cpp::yaml-cpp
    xWalkHal
    xWalkI2cLinux
    xWalkGpioLinux
    xWalkSpiLinux
    xWalkAudioAlsa
    xWalkButtonEventSequence
    xWalkInitAnglesSequence
    xWalkMotorSequence
    xWalkPiperStreamSequence
    xWalkRobotHat5MotorSequence
    xWalkServoHatSequence
    xWalkServoSequence
    xWalkToneSequence
    xWalkLanguageModelOllama
    xWalkMusicAlsa
    xWalkSpeakerAlsa
    xWalkSpeechToTextAlsa
    xWalkTextToSpeechAlsa
    xWalkUtilsLinux
)

if(XWALK_HAL_BUILD_RPI)
    set(XWALK_HARDWARE_TEST_TARGETS
        xWalkAdcHardwareTest
        xWalkAdxl345HardwareTest
        xWalkAudioAlsaHardwareTest
        xWalkBoardControlHardwareTest
        xWalkDeviceHardwareTest
        xWalkFirmwareInfoHardwareTest
        xWalkBuzzerHardwareTest
        xWalkCameraHardwareTest
        xWalkGpioHardwareTest
        xWalkSpeechToTextHardwareTest
        xWalkTextToSpeechHardwareTest
        xWalkI2cLinuxHardwareTest
        xWalkLanguageModelHardwareTest
        xWalkLedHardwareTest
        xWalkRgbLedHardwareTest
        xWalkMotorHardwareTest
        xWalkMusicAlsaHardwareTest
        xWalkPwmHardwareTest
        xWalkServoHardwareTest
        xWalkSpeakerAlsaHardwareTest
        xWalkSpiLinuxHardwareTest
        xWalkTraceHardwareTest
        xWalkUserButtonHardwareTest
        xWalkUtilsLinuxHardwareTest
        xWalkVoiceAssistantHardwareTest
        xSequenceTest
    )
    add_dependencies(xGoogleTest ${XWALK_HARDWARE_TEST_TARGETS})
    target_compile_definitions(xGoogleTest PRIVATE
        XWALK_GOOGLE_TEST_HARDWARE_PROFILE=1
        XWALK_HARDWARE_ADC_TEST="$<TARGET_FILE:xWalkAdcHardwareTest>"
        XWALK_HARDWARE_ADXL345_TEST="$<TARGET_FILE:xWalkAdxl345HardwareTest>"
        XWALK_HARDWARE_AUDIO_TEST="$<TARGET_FILE:xWalkAudioAlsaHardwareTest>"
        XWALK_HARDWARE_BOARD_CONTROL_TEST="$<TARGET_FILE:xWalkBoardControlHardwareTest>"
        XWALK_HARDWARE_DEVICE_TEST="$<TARGET_FILE:xWalkDeviceHardwareTest>"
        XWALK_HARDWARE_FIRMWARE_TEST="$<TARGET_FILE:xWalkFirmwareInfoHardwareTest>"
        XWALK_HARDWARE_BUZZER_TEST="$<TARGET_FILE:xWalkBuzzerHardwareTest>"
        XWALK_HARDWARE_CAMERA_TEST="$<TARGET_FILE:xWalkCameraHardwareTest>"
        XWALK_HARDWARE_GPIO_TEST="$<TARGET_FILE:xWalkGpioHardwareTest>"
        XWALK_HARDWARE_GPT_STT_TEST="$<TARGET_FILE:xWalkSpeechToTextHardwareTest>"
        XWALK_HARDWARE_GPT_TTS_TEST="$<TARGET_FILE:xWalkTextToSpeechHardwareTest>"
        XWALK_HARDWARE_I2C_TEST="$<TARGET_FILE:xWalkI2cLinuxHardwareTest>"
        XWALK_HARDWARE_LANGUAGE_MODEL_TEST="$<TARGET_FILE:xWalkLanguageModelHardwareTest>"
        XWALK_HARDWARE_LED_TEST="$<TARGET_FILE:xWalkLedHardwareTest>"
        XWALK_HARDWARE_RGB_LED_TEST="$<TARGET_FILE:xWalkRgbLedHardwareTest>"
        XWALK_HARDWARE_MOTOR_TEST="$<TARGET_FILE:xWalkMotorHardwareTest>"
        XWALK_HARDWARE_MUSIC_TEST="$<TARGET_FILE:xWalkMusicAlsaHardwareTest>"
        XWALK_HARDWARE_PWM_TEST="$<TARGET_FILE:xWalkPwmHardwareTest>"
        XWALK_HARDWARE_SERVO_TEST="$<TARGET_FILE:xWalkServoHardwareTest>"
        XWALK_HARDWARE_SPEAKER_TEST="$<TARGET_FILE:xWalkSpeakerAlsaHardwareTest>"
        XWALK_HARDWARE_SPI_TEST="$<TARGET_FILE:xWalkSpiLinuxHardwareTest>"
        XWALK_HARDWARE_TRACE_TEST="$<TARGET_FILE:xWalkTraceHardwareTest>"
        XWALK_HARDWARE_USER_BUTTON_TEST="$<TARGET_FILE:xWalkUserButtonHardwareTest>"
        XWALK_HARDWARE_UTILS_TEST="$<TARGET_FILE:xWalkUtilsLinuxHardwareTest>"
        XWALK_HARDWARE_VOICE_ASSISTANT_TEST="$<TARGET_FILE:xWalkVoiceAssistantHardwareTest>"
        XWALK_HARDWARE_SEQUENCE_TEST="$<TARGET_FILE:xSequenceTest>"
    )
endif()

if(XWALK_HAL_BUILD_HOST)
    add_test(NAME xGoogleTest COMMAND $<TARGET_FILE:xGoogleTest>)
    set_tests_properties(xGoogleTest PROPERTIES
        LABELS "host;unit"
        TIMEOUT 120)
endif()
