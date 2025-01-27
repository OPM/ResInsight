set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultGeometryGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivReservoirFaultsPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultReactivationModelPartMgr.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivReservoirFaultsPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFaultReactivationModelPartMgr.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

