set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivSeismicSectionPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivSeismicSectionSourceInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/RivTexturePartMgr.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivSeismicSectionPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivSeismicSectionSourceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivTexturePartMgr.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ModelVisualization\\Seismic"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
