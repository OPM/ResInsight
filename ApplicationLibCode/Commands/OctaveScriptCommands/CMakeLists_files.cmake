set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicAddScriptPathFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteScriptPathFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicEditScriptFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExecuteLastUsedScriptFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExecuteScriptForCasesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewOctaveScriptFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPythonScriptFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicScriptFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRefreshScriptsFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
