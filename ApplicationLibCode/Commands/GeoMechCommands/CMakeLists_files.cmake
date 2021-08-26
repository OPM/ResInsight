set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.cpp)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

# set (COMMAND_QT_MOC_HEADERS ${COMMAND_QT_MOC_HEADERS}
# ${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.h )

source_group(
  "CommandFeature\\GeoMechCommands"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
