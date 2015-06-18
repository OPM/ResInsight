
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivFemPartGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivFemPartPartMgr.h
${CEE_CURRENT_LIST_DIR}RivGeoMechPartMgr.h
${CEE_CURRENT_LIST_DIR}RivFemPickSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivFemElmVisibilityCalculator.h

)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivFemPartGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivFemPartPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivGeoMechPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivFemPickSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivFemElmVisibilityCalculator.cpp

)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "GeoMechViz" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
