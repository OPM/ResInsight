
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddScriptPathFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteScriptPathFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEditScriptFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptForCasesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewOctaveScriptFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPythonScriptFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicScriptFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicRefreshScriptsFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddScriptPathFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteScriptPathFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEditScriptFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptForCasesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewOctaveScriptFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPythonScriptFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicScriptFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRefreshScriptsFeature.cpp
)

set (COMMAND_QT_MOC_HEADERS
${COMMAND_QT_MOC_HEADERS}
${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptForCasesFeature.h
)

list(APPEND COMMAND_CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND COMMAND_CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Script" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
