cmake_minimum_required(VERSION 3.15)

project(PopulateConfigureFilesForRipsPackage)

# cmake configuration used to produce the Python files based on version info in repository

execute_process(COMMAND "python" -m grpc_tools.protoc --version
                OUTPUT_VARIABLE PYTHON_GRPC_PROTOC_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Using Python grpc protoc compiler: ${PYTHON_GRPC_PROTOC_VERSION}")

message(STATUS "CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")

# Assign CMake version variables
include(${CMAKE_SOURCE_DIR}/../../ResInsightVersion.cmake)

configure_file(${CMAKE_SOURCE_DIR}/../../ApplicationLibCode/Adm/RiaVersionInfo.py.cmake
               ${CMAKE_SOURCE_DIR}/../Python/rips/generated/RiaVersionInfo.py)
configure_file(${CMAKE_SOURCE_DIR}/../Python/setup.py.cmake
               ${CMAKE_SOURCE_DIR}/../Python/setup.py)
