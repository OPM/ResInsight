set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellIntegrityAnalysisFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRunWellIntegrityAnalysisFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFaultReactModelingFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRunFaultReactModelingFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowFaultReactModelFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
