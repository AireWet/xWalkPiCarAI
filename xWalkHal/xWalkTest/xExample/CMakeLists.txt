cmake_minimum_required(VERSION 3.16)

project(xExample LANGUAGES CXX)

set(XWALK_EXAMPLE_OLLAMA_ENDPOINT
    "http://192.168.100.145:11434/api/chat" CACHE STRING
    "Deployment-selected Ollama chat endpoint used by ported examples")
set(XWALK_EXAMPLE_OLLAMA_TEXT_ENDPOINT
    "http://localhost:11434/api/chat" CACHE STRING
    "Deployment-selected Ollama endpoint used by the text-only example")
set(XWALK_EXAMPLE_OTHERS_ENDPOINT
    "https://example.invalid/v1/chat/completions" CACHE STRING
    "Deployment-selected compatible endpoint used by the generic example")
set(XWALK_EXAMPLE_OTHERS_MODEL
    "deployment-model" CACHE STRING
    "Deployment-selected model used by the generic provider example")
set(XWALK_EXAMPLE_QWEN_ENDPOINT
    "https://dashscope-intl.aliyuncs.com/compatible-mode/v1/chat/completions"
    CACHE STRING
    "Deployment-selected Qwen OpenAI-compatible chat endpoint")
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/VoskModel.cmake)
set(XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/config)
file(MAKE_DIRECTORY ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
set(XWALK_EXAMPLE_IMAGE_OUTPUT_PATH
    ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY}/llm-img.jpg)
configure_file(config/xHal_Rpi5CarExampleConfig.h.in
    ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY}/xHal_Rpi5CarExampleConfig.h @ONLY)
configure_file(config/xHal_Rpi5CarExampleConfig.yml
    ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY}/xHal_Rpi5CarExampleConfig.yml @ONLY)

add_library(xWalkLedExample
    core/src/xHal_Rpi5CarLedExample.cpp)
target_link_libraries(xWalkLedExample PRIVATE xWalkTrace)
target_include_directories(xWalkLedExample PUBLIC core/include)
target_compile_features(xWalkLedExample PUBLIC cxx_std_17)
target_link_libraries(xWalkLedExample PUBLIC xWalkLibraryCommon)

add_library(xWalkDeepseekExample
    core/src/xHal_Rpi5CarDeepseekExample.cpp)
target_link_libraries(xWalkDeepseekExample PRIVATE xWalkTrace)
target_include_directories(xWalkDeepseekExample PUBLIC core/include)
target_compile_features(xWalkDeepseekExample PUBLIC cxx_std_17)
target_link_libraries(xWalkDeepseekExample PUBLIC xWalkLanguageModel)

add_library(xWalkDoubaoImageExample
    core/src/xHal_Rpi5CarDoubaoImageExample.cpp)
target_link_libraries(xWalkDoubaoImageExample PRIVATE xWalkTrace)
target_include_directories(xWalkDoubaoImageExample PUBLIC core/include)
target_compile_features(xWalkDoubaoImageExample PUBLIC cxx_std_17)
target_link_libraries(xWalkDoubaoImageExample PUBLIC
    xWalkCamera
    xWalkLanguageModel)

add_library(xWalkDoubaoExample
    core/src/xHal_Rpi5CarDoubaoExample.cpp)
target_link_libraries(xWalkDoubaoExample PRIVATE xWalkTrace)
target_include_directories(xWalkDoubaoExample PUBLIC core/include)
target_compile_features(xWalkDoubaoExample PUBLIC cxx_std_17)
target_link_libraries(xWalkDoubaoExample PUBLIC xWalkLanguageModel)

add_library(xWalkGeminiExample
    core/src/xHal_Rpi5CarGeminiExample.cpp)
target_link_libraries(xWalkGeminiExample PRIVATE xWalkTrace)
target_include_directories(xWalkGeminiExample PUBLIC core/include)
target_compile_features(xWalkGeminiExample PUBLIC cxx_std_17)
target_link_libraries(xWalkGeminiExample PUBLIC xWalkLanguageModel)

add_library(xWalkGrokExample
    core/src/xHal_Rpi5CarGrokExample.cpp)
target_link_libraries(xWalkGrokExample PRIVATE xWalkTrace)
target_include_directories(xWalkGrokExample PUBLIC core/include)
target_compile_features(xWalkGrokExample PUBLIC cxx_std_17)
target_link_libraries(xWalkGrokExample PUBLIC xWalkLanguageModel)

add_library(xWalkOllamaImageExample
    core/src/xHal_Rpi5CarOllamaImageExample.cpp)
target_link_libraries(xWalkOllamaImageExample PRIVATE xWalkTrace)
target_include_directories(xWalkOllamaImageExample PUBLIC core/include)
target_compile_features(xWalkOllamaImageExample PUBLIC cxx_std_17)
target_link_libraries(xWalkOllamaImageExample PUBLIC
    xWalkCamera
    xWalkLanguageModel)

add_library(xWalkOllamaExample
    core/src/xHal_Rpi5CarOllamaExample.cpp)
target_link_libraries(xWalkOllamaExample PRIVATE xWalkTrace)
target_include_directories(xWalkOllamaExample PUBLIC core/include)
target_compile_features(xWalkOllamaExample PUBLIC cxx_std_17)
target_link_libraries(xWalkOllamaExample PUBLIC xWalkLanguageModel)

add_library(xWalkOpenAiImageExample
    core/src/xHal_Rpi5CarOpenAiImageExample.cpp)
target_link_libraries(xWalkOpenAiImageExample PRIVATE xWalkTrace)
target_include_directories(xWalkOpenAiImageExample PUBLIC core/include)
target_compile_features(xWalkOpenAiImageExample PUBLIC cxx_std_17)
target_link_libraries(xWalkOpenAiImageExample PUBLIC
    xWalkCamera
    xWalkLanguageModel)

add_library(xWalkOpenAiExample
    core/src/xHal_Rpi5CarOpenAiExample.cpp)
target_link_libraries(xWalkOpenAiExample PRIVATE xWalkTrace)
target_include_directories(xWalkOpenAiExample PUBLIC core/include)
target_compile_features(xWalkOpenAiExample PUBLIC cxx_std_17)
target_link_libraries(xWalkOpenAiExample PUBLIC xWalkLanguageModel)

add_library(xWalkOthersExample
    core/src/xHal_Rpi5CarOthersExample.cpp)
target_link_libraries(xWalkOthersExample PRIVATE xWalkTrace)
target_include_directories(xWalkOthersExample PUBLIC core/include)
target_compile_features(xWalkOthersExample PUBLIC cxx_std_17)
target_link_libraries(xWalkOthersExample PUBLIC xWalkLanguageModel)

add_library(xWalkQwenExample
    core/src/xHal_Rpi5CarQwenExample.cpp)
target_link_libraries(xWalkQwenExample PRIVATE xWalkTrace)
target_include_directories(xWalkQwenExample PUBLIC core/include)
target_compile_features(xWalkQwenExample PUBLIC cxx_std_17)
target_link_libraries(xWalkQwenExample PUBLIC xWalkLanguageModel)

add_library(xWalkPinInputExample
    core/src/xHal_Rpi5CarPinInputExample.cpp)
target_link_libraries(xWalkPinInputExample PRIVATE xWalkTrace)
target_include_directories(xWalkPinInputExample PUBLIC core/include)
target_compile_features(xWalkPinInputExample PUBLIC cxx_std_17)
target_link_libraries(xWalkPinInputExample PUBLIC xWalkLibraryCommon)

add_library(xWalkUltrasonicExample
    core/src/xHal_Rpi5CarUltrasonicExample.cpp)
target_link_libraries(xWalkUltrasonicExample PRIVATE xWalkTrace)
target_include_directories(xWalkUltrasonicExample PUBLIC core/include)
target_compile_features(xWalkUltrasonicExample PUBLIC cxx_std_17)
target_link_libraries(xWalkUltrasonicExample PUBLIC xWalkLibraryCommon)

add_library(xWalkVoiceAssistantExample
    core/src/xHal_Rpi5CarVoiceAssistantExample.cpp)
target_link_libraries(xWalkVoiceAssistantExample PRIVATE xWalkTrace)
target_include_directories(xWalkVoiceAssistantExample PUBLIC core/include)
target_compile_features(xWalkVoiceAssistantExample PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceAssistantExample PUBLIC xWalkLibraryCommon)

add_library(xWalkServoExample
    core/src/xHal_Rpi5CarServoExample.cpp)
target_link_libraries(xWalkServoExample PRIVATE xWalkTrace)
target_include_directories(xWalkServoExample PUBLIC core/include)
target_compile_features(xWalkServoExample PUBLIC cxx_std_17)
target_link_libraries(xWalkServoExample PUBLIC xWalkLibraryCommon)

add_library(xWalkSttVoskStreamExample
    core/src/xHal_Rpi5CarSttVoskStreamExample.cpp)
target_link_libraries(xWalkSttVoskStreamExample PRIVATE xWalkTrace)
target_include_directories(xWalkSttVoskStreamExample PUBLIC core/include)
target_compile_features(xWalkSttVoskStreamExample PUBLIC cxx_std_17)
target_link_libraries(xWalkSttVoskStreamExample PUBLIC xWalkLibraryCommon)

add_library(xWalkSttVoskWakeWordThreadExample
    core/src/xHal_Rpi5CarSttVoskWakeWordThreadExample.cpp)
target_link_libraries(xWalkSttVoskWakeWordThreadExample PRIVATE xWalkTrace)
target_include_directories(xWalkSttVoskWakeWordThreadExample PUBLIC core/include)
target_compile_features(xWalkSttVoskWakeWordThreadExample PUBLIC cxx_std_17)
target_link_libraries(xWalkSttVoskWakeWordThreadExample PUBLIC xWalkLibraryCommon)

add_library(xWalkSttVoskWakeWordExample
    core/src/xHal_Rpi5CarSttVoskWakeWordExample.cpp)
target_link_libraries(xWalkSttVoskWakeWordExample PRIVATE xWalkTrace)
target_include_directories(xWalkSttVoskWakeWordExample PUBLIC core/include)
target_compile_features(xWalkSttVoskWakeWordExample PUBLIC cxx_std_17)
target_link_libraries(xWalkSttVoskWakeWordExample PUBLIC xWalkLibraryCommon)

add_library(xWalkSttVoskWithoutStreamExample
    core/src/xHal_Rpi5CarSttVoskWithoutStreamExample.cpp)
target_link_libraries(xWalkSttVoskWithoutStreamExample PRIVATE xWalkTrace)
target_include_directories(xWalkSttVoskWithoutStreamExample PUBLIC core/include)
target_compile_features(xWalkSttVoskWithoutStreamExample PUBLIC cxx_std_17)
target_link_libraries(xWalkSttVoskWithoutStreamExample PUBLIC xWalkLibraryCommon)

add_library(xWalkTtsEdgeExample
    core/src/xHal_Rpi5CarTtsEdgeExample.cpp)
target_link_libraries(xWalkTtsEdgeExample PRIVATE xWalkTrace)
target_include_directories(xWalkTtsEdgeExample PUBLIC core/include)
target_compile_features(xWalkTtsEdgeExample PUBLIC cxx_std_17)
target_link_libraries(xWalkTtsEdgeExample PUBLIC xWalkLibraryCommon)

add_library(xWalkTtsEspeakExample
    core/src/xHal_Rpi5CarTtsEspeakExample.cpp)
target_link_libraries(xWalkTtsEspeakExample PRIVATE xWalkTrace)
target_include_directories(xWalkTtsEspeakExample PUBLIC core/include)
target_compile_features(xWalkTtsEspeakExample PUBLIC cxx_std_17)
target_link_libraries(xWalkTtsEspeakExample PUBLIC xWalkLibraryCommon)

add_library(xWalkTtsOpenAiExample
    core/src/xHal_Rpi5CarTtsOpenAiExample.cpp)
target_link_libraries(xWalkTtsOpenAiExample PRIVATE xWalkTrace)
target_include_directories(xWalkTtsOpenAiExample PUBLIC core/include)
target_compile_features(xWalkTtsOpenAiExample PUBLIC cxx_std_17)
target_link_libraries(xWalkTtsOpenAiExample PUBLIC xWalkLibraryCommon)

add_library(xWalkTtsPico2WaveExample
    core/src/xHal_Rpi5CarTtsPico2WaveExample.cpp)
target_link_libraries(xWalkTtsPico2WaveExample PRIVATE xWalkTrace)
target_include_directories(xWalkTtsPico2WaveExample PUBLIC core/include)
target_compile_features(xWalkTtsPico2WaveExample PUBLIC cxx_std_17)
target_link_libraries(xWalkTtsPico2WaveExample PUBLIC xWalkLibraryCommon)

add_library(xWalkTtsPiperExample
    core/src/xHal_Rpi5CarTtsPiperExample.cpp)
target_link_libraries(xWalkTtsPiperExample PRIVATE xWalkTrace)
target_include_directories(xWalkTtsPiperExample PUBLIC core/include)
target_compile_features(xWalkTtsPiperExample PUBLIC cxx_std_17)
target_link_libraries(xWalkTtsPiperExample PUBLIC xWalkLibraryCommon)

if(XWALK_HAL_BUILD_RPI)
    find_package(yaml-cpp CONFIG REQUIRED)

    add_library(xWalkLedExampleLinux
        hardware/src/xHal_Rpi5CarLedExampleLinux.cpp)
target_link_libraries(xWalkLedExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkLedExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkLedExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkLedExampleLinux PUBLIC
        xWalkGpioLinux
        xWalkLed
        xWalkLedExample)

    add_library(xWalkDeepseekExampleLinux
        hardware/src/xHal_Rpi5CarDeepseekExampleLinux.cpp)
    target_include_directories(xWalkDeepseekExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkDeepseekExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkDeepseekExampleLinux PUBLIC
        xWalkDeepseekExample
        xWalkLanguageModelOllama)

    add_library(xWalkDoubaoImageExampleLinux
        hardware/src/xHal_Rpi5CarDoubaoImageExampleLinux.cpp)
    target_include_directories(xWalkDoubaoImageExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkDoubaoImageExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkDoubaoImageExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkDoubaoImageExampleLinux PUBLIC
        xWalkCameraLinux
        xWalkDoubaoImageExample
        xWalkLanguageModelOllama)

    add_library(xWalkDoubaoExampleLinux
        hardware/src/xHal_Rpi5CarDoubaoExampleLinux.cpp)
    target_include_directories(xWalkDoubaoExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkDoubaoExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkDoubaoExampleLinux PUBLIC
        xWalkDoubaoExample
        xWalkLanguageModelOllama)

    add_library(xWalkGeminiExampleLinux
        hardware/src/xHal_Rpi5CarGeminiExampleLinux.cpp)
    target_include_directories(xWalkGeminiExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkGeminiExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkGeminiExampleLinux PUBLIC
        xWalkGeminiExample
        xWalkLanguageModelOllama)

    add_library(xWalkGrokExampleLinux
        hardware/src/xHal_Rpi5CarGrokExampleLinux.cpp)
    target_include_directories(xWalkGrokExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkGrokExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkGrokExampleLinux PUBLIC
        xWalkGrokExample
        xWalkLanguageModelOllama)

    add_library(xWalkOllamaImageExampleLinux
        hardware/src/xHal_Rpi5CarOllamaImageExampleLinux.cpp)
    target_include_directories(xWalkOllamaImageExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkOllamaImageExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkOllamaImageExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkOllamaImageExampleLinux PUBLIC
        xWalkCameraLinux
        xWalkLanguageModelOllama
        xWalkOllamaImageExample)

    add_library(xWalkOllamaExampleLinux
        hardware/src/xHal_Rpi5CarOllamaExampleLinux.cpp)
    target_include_directories(xWalkOllamaExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkOllamaExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkOllamaExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkOllamaExampleLinux PUBLIC
        xWalkLanguageModelOllama
        xWalkOllamaExample)

    add_library(xWalkOpenAiImageExampleLinux
        hardware/src/xHal_Rpi5CarOpenAiImageExampleLinux.cpp)
    target_include_directories(xWalkOpenAiImageExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkOpenAiImageExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkOpenAiImageExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkOpenAiImageExampleLinux PUBLIC
        xWalkCameraLinux
        xWalkLanguageModelOllama
        xWalkOpenAiImageExample)

    add_library(xWalkOpenAiExampleLinux
        hardware/src/xHal_Rpi5CarOpenAiExampleLinux.cpp)
    target_include_directories(xWalkOpenAiExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkOpenAiExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkOpenAiExampleLinux PUBLIC
        xWalkLanguageModelOllama
        xWalkOpenAiExample)

    add_library(xWalkOthersExampleLinux
        hardware/src/xHal_Rpi5CarOthersExampleLinux.cpp)
    target_include_directories(xWalkOthersExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkOthersExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkOthersExampleLinux PUBLIC
        xWalkLanguageModelOllama
        xWalkOthersExample)

    add_library(xWalkQwenExampleLinux
        hardware/src/xHal_Rpi5CarQwenExampleLinux.cpp)
    target_include_directories(xWalkQwenExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkQwenExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkQwenExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkQwenExampleLinux PUBLIC
        xWalkLanguageModelOllama
        xWalkQwenExample)

    add_library(xWalkPinInputExampleLinux
        hardware/src/xHal_Rpi5CarPinInputExampleLinux.cpp)
target_link_libraries(xWalkPinInputExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkPinInputExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkPinInputExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkPinInputExampleLinux PUBLIC
        xWalkGpioLinux
        xWalkPinInputExample)

    add_library(xWalkUltrasonicExampleLinux
        hardware/src/xHal_Rpi5CarUltrasonicExampleLinux.cpp)
target_link_libraries(xWalkUltrasonicExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkUltrasonicExampleLinux
        PUBLIC hardware/include)
    target_compile_features(xWalkUltrasonicExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkUltrasonicExampleLinux PUBLIC
        xWalkGpioLinux
        xWalkUltrasonic
        xWalkUltrasonicExample)

    add_library(xWalkVoiceAssistantExampleLinux
        hardware/src/xHal_Rpi5CarVoiceAssistantExampleLinux.cpp)
target_link_libraries(xWalkVoiceAssistantExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkVoiceAssistantExampleLinux BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkVoiceAssistantExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkVoiceAssistantExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkVoiceAssistantExampleLinux PUBLIC
        xWalkCameraLinux
        xWalkLanguageModelOllama
        xWalkSpeechRecognizerVosk
        xWalkSpeechToTextAlsa
        xWalkTtsPiperExampleLinux
        xWalkVoiceAssistantExample)

    add_library(xWalkServoExampleLinux
        hardware/src/xHal_Rpi5CarServoExampleLinux.cpp)
target_link_libraries(xWalkServoExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkServoExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkServoExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkServoExampleLinux PUBLIC
        xWalkI2cLinux
        xWalkServo
        xWalkServoExample)

    add_library(xWalkSttVoskStreamExampleLinux
        hardware/src/xHal_Rpi5CarSttVoskStreamExampleLinux.cpp)
target_link_libraries(xWalkSttVoskStreamExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkSttVoskStreamExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkSttVoskStreamExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkSttVoskStreamExampleLinux PUBLIC
        xWalkSpeechRecognizerVosk
        xWalkSttVoskStreamExample)

    add_library(xWalkSttVoskWakeWordThreadExampleLinux
        hardware/src/xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.cpp)
target_link_libraries(xWalkSttVoskWakeWordThreadExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkSttVoskWakeWordThreadExampleLinux
        PUBLIC hardware/include)
    target_compile_features(xWalkSttVoskWakeWordThreadExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkSttVoskWakeWordThreadExampleLinux PUBLIC
        xWalkSpeechRecognizerVosk
        xWalkSttVoskWakeWordThreadExample)

    add_library(xWalkSttVoskWakeWordExampleLinux
        hardware/src/xHal_Rpi5CarSttVoskWakeWordExampleLinux.cpp)
target_link_libraries(xWalkSttVoskWakeWordExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkSttVoskWakeWordExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkSttVoskWakeWordExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkSttVoskWakeWordExampleLinux PUBLIC
        xWalkSpeechRecognizerVosk
        xWalkSttVoskWakeWordExample)

    add_library(xWalkSttVoskWithoutStreamExampleLinux
        hardware/src/xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.cpp)
target_link_libraries(xWalkSttVoskWithoutStreamExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkSttVoskWithoutStreamExampleLinux
        PUBLIC hardware/include)
    target_compile_features(xWalkSttVoskWithoutStreamExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkSttVoskWithoutStreamExampleLinux PUBLIC
        xWalkSpeechRecognizerVosk
        xWalkSttVoskWithoutStreamExample)

    add_library(xWalkTtsEdgeExampleLinux
        hardware/src/xHal_Rpi5CarTtsEdgeExampleLinux.cpp)
target_link_libraries(xWalkTtsEdgeExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkTtsEdgeExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkTtsEdgeExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkTtsEdgeExampleLinux PUBLIC xWalkTtsEdgeExample)

    add_library(xWalkTtsEspeakExampleLinux
        hardware/src/xHal_Rpi5CarTtsEspeakExampleLinux.cpp)
target_link_libraries(xWalkTtsEspeakExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkTtsEspeakExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkTtsEspeakExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkTtsEspeakExampleLinux PUBLIC xWalkTtsEspeakExample)

    find_package(CURL REQUIRED)
    add_library(xWalkTtsOpenAiExampleLinux
        hardware/src/xHal_Rpi5CarTtsOpenAiExampleLinux.cpp)
target_link_libraries(xWalkTtsOpenAiExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkTtsOpenAiExampleLinux PUBLIC hardware/include)
    target_compile_features(xWalkTtsOpenAiExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkTtsOpenAiExampleLinux PUBLIC
        CURL::libcurl
        xWalkTtsOpenAiExample)

    add_library(xWalkTtsPico2WaveExampleLinux
        hardware/src/xHal_Rpi5CarTtsPico2WaveExampleLinux.cpp)
target_link_libraries(xWalkTtsPico2WaveExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkTtsPico2WaveExampleLinux
        PUBLIC hardware/include)
    target_compile_features(xWalkTtsPico2WaveExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkTtsPico2WaveExampleLinux PUBLIC
        xWalkTtsPico2WaveExample)

    add_library(xWalkTtsPiperExampleLinux
        hardware/src/xHal_Rpi5CarTtsPiperExampleLinux.cpp)
target_link_libraries(xWalkTtsPiperExampleLinux PRIVATE xWalkTrace)
    target_include_directories(xWalkTtsPiperExampleLinux
        PUBLIC hardware/include)
    target_compile_features(xWalkTtsPiperExampleLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkTtsPiperExampleLinux PUBLIC
        xWalkTtsPiperExample)

    add_library(xWalkExampleRunner
        hardware/src/xHal_Rpi5CarExampleRunner.cpp)
    target_include_directories(xWalkExampleRunner BEFORE
        PRIVATE ${XWALK_EXAMPLE_CONFIG_OUTPUT_DIRECTORY})
    target_include_directories(xWalkExampleRunner PUBLIC hardware/include)
    target_compile_features(xWalkExampleRunner PUBLIC cxx_std_17)
    target_link_libraries(xWalkExampleRunner
        PUBLIC xWalkLibraryCommon
        PRIVATE
        xWalkDeepseekExampleLinux
        xWalkDoubaoExampleLinux
        xWalkDoubaoImageExampleLinux
        xWalkGeminiExampleLinux
        xWalkGrokExampleLinux
        xWalkOllamaExampleLinux
        xWalkOllamaImageExampleLinux
        xWalkOpenAiImageExampleLinux
        xWalkOpenAiExampleLinux
        xWalkOthersExampleLinux
        xWalkQwenExampleLinux
        xWalkPinInputExampleLinux
        xWalkUltrasonicExampleLinux
        xWalkVoiceAssistantExampleLinux
        xWalkServoExampleLinux
        xWalkSttVoskStreamExampleLinux
        xWalkSttVoskWakeWordThreadExampleLinux
        xWalkSttVoskWakeWordExampleLinux
        xWalkSttVoskWithoutStreamExampleLinux
        xWalkTtsEdgeExampleLinux
        xWalkTtsEspeakExampleLinux
        xWalkTtsOpenAiExampleLinux
        xWalkTtsPico2WaveExampleLinux
        xWalkTtsPiperExampleLinux
        xWalkLedExampleLinux
        yaml-cpp::yaml-cpp)

    add_executable(xExample main.cpp)
    set_target_properties(xExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    target_compile_features(xExample PRIVATE cxx_std_17)
    target_link_libraries(xExample PRIVATE xWalkExampleRunner)

endif()
