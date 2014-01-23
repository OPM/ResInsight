cmake_minimum_required(VERSION 2.8)

project(LibIo)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


include_directories(../LibCore)


set(CEE_HEADER_FILES
cvfFile.h
cvfLibIo.h
cvfMemoryFile.h
cvfPropertyXmlSerializer.h
cvfTinyXmlFused.hpp
cvfXml.h
)

set(CEE_SOURCE_FILES
cvfFile.cpp
cvfMemoryFile.cpp
cvfPropertyXmlSerializer.cpp
cvfXml.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

