set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportOsduFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOilFieldEntry.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOilRegionEntry.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathImport.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellsEntry.h
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportOsduFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOilFieldEntry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOilRegionEntry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathImport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellsEntry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND COMMAND_QT_MOC_HEADERS
     ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.h
)

source_group(
  "CommandFeature\\SsiHub"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
