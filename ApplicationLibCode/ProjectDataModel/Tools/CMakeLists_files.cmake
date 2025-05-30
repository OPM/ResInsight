set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaVariableMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotAxisTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAutomationSettings.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaVariableMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotAxisTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAutomationSettings.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
