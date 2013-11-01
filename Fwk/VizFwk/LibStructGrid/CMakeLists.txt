cmake_minimum_required(VERSION 2.8)

project(LibStructGrid)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


include_directories("../LibCore")
include_directories("../LibGeometry")
include_directories("../LibRender")
include_directories("../LibViewing")


set(CEE_SOURCE_FILES
cvfRectilinearGrid.cpp
cvfStructGridCutPlane.cpp
cvfStructGridGeometry.cpp
cvfStructGridImport.cpp
cvfStructGridIsosurface.cpp
)

add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})
