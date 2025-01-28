set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigResultAccessorFactory.h
    ${CMAKE_CURRENT_LIST_DIR}/RigAllGridCellsResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigActiveCellsResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCellEdgeResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCombTransResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCombMultResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigTernaryResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigTimeHistoryResultAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigDepthResultAccessor.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
