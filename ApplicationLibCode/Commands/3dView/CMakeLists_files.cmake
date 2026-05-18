set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicApplyUserDefinedCameraFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicStoreUserDefinedCameraFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDockIn3dViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDockInPlotViewFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
