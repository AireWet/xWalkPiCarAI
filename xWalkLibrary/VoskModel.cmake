string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" XWALK_VOSK_DETECTED_PROCESSOR)
if(XWALK_VOSK_DETECTED_PROCESSOR MATCHES "^(aarch64|arm64)$")
    set(XWALK_VOSK_DETECTED_ARCHITECTURE aarch64)
elseif(XWALK_VOSK_DETECTED_PROCESSOR MATCHES "^(x86_64|amd64)$")
    set(XWALK_VOSK_DETECTED_ARCHITECTURE x86_64)
else()
    set(XWALK_VOSK_DETECTED_ARCHITECTURE unsupported)
endif()

set(XWALK_VOSK_ARCHITECTURE
    "${XWALK_VOSK_DETECTED_ARCHITECTURE}" CACHE STRING
    "Vosk native runtime architecture: aarch64 or x86_64")
set_property(CACHE XWALK_VOSK_ARCHITECTURE PROPERTY STRINGS
    aarch64 x86_64)

if(NOT XWALK_VOSK_ARCHITECTURE STREQUAL "aarch64" AND
    NOT XWALK_VOSK_ARCHITECTURE STREQUAL "x86_64")
    message(FATAL_ERROR
        "Vosk has no bundled runtime for processor '${CMAKE_SYSTEM_PROCESSOR}'. "
        "Set XWALK_VOSK_ARCHITECTURE to aarch64 or x86_64 for a supported target.")
endif()

set(XWALK_VOSK_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/vosk" CACHE PATH
    "Project-managed Vosk runtime and model root")
if(NOT DEFINED XWALK_VOSK_LIBRARY_PATH OR
    XWALK_VOSK_LIBRARY_PATH STREQUAL "")
    set(XWALK_VOSK_LIBRARY_PATH
        "${XWALK_VOSK_ROOT}/lib/${XWALK_VOSK_ARCHITECTURE}/libvosk.so")
endif()
if(NOT DEFINED XWALK_VOSK_MODEL_PATH OR XWALK_VOSK_MODEL_PATH STREQUAL "")
    set(XWALK_VOSK_MODEL_PATH
        "${XWALK_VOSK_ROOT}/models/vosk-model-small-en-us-0.15")
endif()

if(NOT EXISTS "${XWALK_VOSK_LIBRARY_PATH}")
    message(FATAL_ERROR
        "The ${XWALK_VOSK_ARCHITECTURE} Vosk library does not exist: "
        "${XWALK_VOSK_LIBRARY_PATH}")
endif()
if(NOT IS_DIRECTORY "${XWALK_VOSK_MODEL_PATH}")
    message(FATAL_ERROR
        "The configured Vosk model does not exist: ${XWALK_VOSK_MODEL_PATH}")
endif()
