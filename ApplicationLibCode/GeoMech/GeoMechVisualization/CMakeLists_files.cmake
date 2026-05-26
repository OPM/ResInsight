set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPartPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechPartMgrCache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivGeoMechVizLogic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemPickSourceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemElmVisibilityCalculator.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
