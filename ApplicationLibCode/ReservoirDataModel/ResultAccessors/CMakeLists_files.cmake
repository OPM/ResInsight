set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigResultAccessorFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigAllGridCellsResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigActiveCellsResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellEdgeResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCombTransResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCombMultResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigTernaryResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigTimeHistoryResultAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigDepthResultAccessor.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
