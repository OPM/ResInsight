set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimDoubleParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimIntegerParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStringParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimListParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroup.h
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroups.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDoubleParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntegerParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStringParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimListParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterGroups.cpp)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Parameters"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
