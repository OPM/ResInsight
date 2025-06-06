set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartGeometryGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgrCache.h
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechVizLogic.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPickSourceInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFemElmVisibilityCalculator.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgrCache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechVizLogic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPickSourceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemElmVisibilityCalculator.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
