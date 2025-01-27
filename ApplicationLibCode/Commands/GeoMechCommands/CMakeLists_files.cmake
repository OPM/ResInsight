set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellIntegrityAnalysisFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRunWellIntegrityAnalysisFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFaultReactModelingFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRunFaultReactModelingFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowFaultReactModelFeature.h
)

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

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

