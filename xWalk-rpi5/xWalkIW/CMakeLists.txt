cmake_minimum_required(VERSION 3.16)

project(xWalkIW LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/XWalkDependencies.cmake)

option(XWALK_IW_BUILD_HOST_TESTS "Build xWalkIW host schema validation" OFF)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

set(XWALK_IW_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/python/xHal_Rpi5CarIwGenerator)
set(XWALK_IW_GENERATED_INCLUDE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/auto-gen/include)
set(XWALK_IW_GENERATED_SOURCE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/auto-gen/src)

set(XWALK_IW_GENERATED_FILES
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwEnum.pb.h
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwMessageReq.pb.h
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwMessageCfm.pb.h
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwMessageRej.pb.h
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwGrpc.pb.h
    ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarXIwGrpc.grpc.pb.h
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwEnum.pb.cpp
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwMessageReq.pb.cpp
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwMessageCfm.pb.cpp
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwMessageRej.pb.cpp
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwGrpc.pb.cpp
    ${XWALK_IW_GENERATED_SOURCE_DIRECTORY}/xHal_Rpi5CarXIwGrpc.grpc.pb.cpp
)

foreach(generated_file IN LISTS XWALK_IW_GENERATED_FILES)
    if(NOT EXISTS "${generated_file}")
        message(FATAL_ERROR
            "Missing xWalkIW generated file: ${generated_file}. "
            "Run xWalkTool/python/xHal_Rpi5CarIwGenerator --generate-cpp before configuring CMake.")
    endif()
endforeach()

add_custom_target(xWalkIwSchemaCheck ALL
    COMMAND ${Python3_EXECUTABLE} ${XWALK_IW_GENERATOR} --check
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Validating the xWalkIW Protobuf and XML contract"
    VERBATIM)

add_library(xWalkIwGrpc STATIC
    ${XWALK_IW_GENERATED_FILES}
)
add_dependencies(xWalkIwGrpc xWalkIwSchemaCheck)
set_target_properties(xWalkIwGrpc PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(xWalkIwGrpc
    PUBLIC
        ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}
)
target_compile_features(xWalkIwGrpc PUBLIC cxx_std_17)
target_link_libraries(xWalkIwGrpc
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
    PRIVATE
        xWalkLibraryCommon
)

add_library(xWalkIW INTERFACE)
add_library(xWalk::IW ALIAS xWalkIW)
target_include_directories(xWalkIW
    INTERFACE
        ${XWALK_IW_GENERATED_INCLUDE_DIRECTORY}
)
target_compile_features(xWalkIW INTERFACE cxx_std_17)
target_link_libraries(xWalkIW
    INTERFACE
        xWalkIwGrpc
)

if(XWALK_IW_BUILD_HOST_TESTS)
    include(CTest)
    add_test(NAME xWalkIwSchemaHostTest
        COMMAND ${Python3_EXECUTABLE} ${XWALK_IW_GENERATOR} --check)
    set_tests_properties(xWalkIwSchemaHostTest PROPERTIES LABELS "host;schema")
endif()
