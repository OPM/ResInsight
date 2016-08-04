
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicAddScriptPathFeature.h
${CEE_CURRENT_LIST_DIR}RicDeleteScriptPathFeature.h
${CEE_CURRENT_LIST_DIR}RicEditScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicExecuteScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.h
${CEE_CURRENT_LIST_DIR}RicNewScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicScriptFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicRefreshScriptsFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicAddScriptPathFeature.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteScriptPathFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEditScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExecuteScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicScriptFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRefreshScriptsFeature.cpp
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.h
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Script" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
