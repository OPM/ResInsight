set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDoubleParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntegerParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStringParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimListParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroups.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterList.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
