set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivSurfacePartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivSurfaceIntersectionGeometryGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RivReservoirSurfaceIntersectionSourceInfo.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivSurfacePartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivSurfaceIntersectionGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivReservoirSurfaceIntersectionSourceInfo.cpp)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ModelVisualization\\Surfaces"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
