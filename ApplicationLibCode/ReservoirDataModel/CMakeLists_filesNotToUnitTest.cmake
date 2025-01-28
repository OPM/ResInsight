set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapperTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseRangeFilterMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoirGridTools.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapperTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseRangeFilterMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoirGridTools.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
