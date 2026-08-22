if(NOT DEFINED XWALK_SOURCE_ROOT)
    message(FATAL_ERROR "XWALK_SOURCE_ROOT is required")
endif()

file(GLOB_RECURSE XWALK_NAMING_FILES
    "${XWALK_SOURCE_ROOT}/*.h"
    "${XWALK_SOURCE_ROOT}/*.hpp"
    "${XWALK_SOURCE_ROOT}/*.cpp"
    "${XWALK_SOURCE_ROOT}/*.cc")

foreach(XWALK_NAMING_FILE IN LISTS XWALK_NAMING_FILES)
    if(XWALK_NAMING_FILE MATCHES "/(auto-gen|generated)/")
        continue()
    endif()
    file(READ "${XWALK_NAMING_FILE}" XWALK_NAMING_CONTENT)
    string(REGEX MATCHALL "(CXX|cxx)_[A-Za-z0-9_]+[ \t\r\n]*\\(" XWALK_CALLABLES "${XWALK_NAMING_CONTENT}")
    foreach(XWALK_CALLABLE IN LISTS XWALK_CALLABLES)
        string(REGEX REPLACE "[ \t\r\n]*\\($" "" XWALK_CALLABLE_NAME "${XWALK_CALLABLE}")
        if(NOT XWALK_CALLABLE_NAME MATCHES "_LPP$")
            message(FATAL_ERROR
                "Callable ${XWALK_CALLABLE_NAME} in ${XWALK_NAMING_FILE} must end in _LPP")
        endif()
    endforeach()
endforeach()

message(STATUS "Validated _LPP callable naming")
