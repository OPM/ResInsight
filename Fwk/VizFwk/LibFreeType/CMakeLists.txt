cmake_minimum_required(VERSION 2.8)

project(LibFreeType)


# Use our strict compile flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


include_directories(../LibCore)
include_directories(../LibIo)
include_directories(../LibGeometry)
include_directories(../LibRender)
include_directories(../ThirdParty/FreeType/include)


set(CEE_HEADER_FILES
cvfFreeTypeFont.h
)

set(CEE_SOURCE_FILES
cvfFreeTypeFont.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
