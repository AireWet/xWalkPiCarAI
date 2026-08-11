if(NOT DEFINED XWALK_EXPECTED_ARCHITECTURE)
    message(FATAL_ERROR "XWALK_EXPECTED_ARCHITECTURE is required")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../VoskModel.cmake)
get_filename_component(XWALK_TEST_LIBRARY_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)

set(XWALK_LIBRARY_COMMON_PUBLIC_HEADERS
    xHal_Rpi5CarCommon.h
    xHal_Rpi5CarCommonFunctions.h
    xHal_Rpi5CarFileFunctions.h
    xHal_Rpi5CarLinuxHeaders.h
    xHal_Rpi5CarMath.h
    xHal_Rpi5CarStandardHeaders.h
    xHal_Rpi5CarTestFunctions.h
    xHal_Rpi5CarTypes.h
    xWalkControllerConfigTypes.h)

foreach(XWALK_LIBRARY_COMMON_PUBLIC_HEADER IN LISTS XWALK_LIBRARY_COMMON_PUBLIC_HEADERS)
    if(NOT EXISTS
        "${XWALK_TEST_LIBRARY_ROOT}/common/include/${XWALK_LIBRARY_COMMON_PUBLIC_HEADER}")
        message(FATAL_ERROR
            "The xWalkLibraryCommon public header is missing: ${XWALK_LIBRARY_COMMON_PUBLIC_HEADER}")
    endif()
endforeach()

if(NOT XWALK_LIBRARY_ARCHITECTURE STREQUAL XWALK_EXPECTED_ARCHITECTURE)
    message(FATAL_ERROR
        "Expected ${XWALK_EXPECTED_ARCHITECTURE}, selected ${XWALK_LIBRARY_ARCHITECTURE}")
endif()
if(NOT XWALK_LIBRARY_NATIVE_PREFIX STREQUAL
    "${XWALK_TEST_LIBRARY_ROOT}/${XWALK_EXPECTED_ARCHITECTURE}")
    message(FATAL_ERROR "The native project dependency prefix is incorrect")
endif()
if(NOT EXISTS "${XWALK_VOSK_LIBRARY_PATH}")
    message(FATAL_ERROR "The selected Vosk library is missing: ${XWALK_VOSK_LIBRARY_PATH}")
endif()
if(NOT IS_DIRECTORY "${XWALK_VOSK_MODEL_PATH}")
    message(FATAL_ERROR "The shared Vosk model is missing: ${XWALK_VOSK_MODEL_PATH}")
endif()
