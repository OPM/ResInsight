set(SOURCE_GROUP_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/RigMswTableRows.h
                              ${CMAKE_CURRENT_LIST_DIR}/RigMswTableData.h
                              ${CMAKE_CURRENT_LIST_DIR}/RigMswDataFormatter.h
                              ${CMAKE_CURRENT_LIST_DIR}/RigMswUnifiedData.h 
)

set(SOURCE_GROUP_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/RigMswTableData.cpp
                              ${CMAKE_CURRENT_LIST_DIR}/RigMswDataFormatter.cpp
                              ${CMAKE_CURRENT_LIST_DIR}/RigMswUnifiedData.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
