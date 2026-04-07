set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicApplyUserDefinedCameraFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicStoreUserDefinedCameraFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPopOutTo3dViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPopOutToPlotViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicConvert3dToMdiFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
