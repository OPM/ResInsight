cmake_minimum_required(VERSION 2.8)

project(LibRegGrid2D)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


include_directories(../LibCore)
include_directories(../LibIo)
include_directories(../LibGeometry)
include_directories(../LibRender)


set(CEE_SOURCE_FILES
cvfRegGrid2D.cpp
cvfRegGrid2DExportXml.cpp
cvfRegGrid2DGeometry.cpp
cvfRegGrid2DImportXml.cpp
)

add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})
