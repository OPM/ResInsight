set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportOsduFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.h
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellLogImportWizard.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteOsduTokenFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportOsduFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiuWellLogImportWizard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteOsduTokenFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND COMMAND_QT_MOC_HEADERS
     ${CMAKE_CURRENT_LIST_DIR}/RiuWellImportWizard.h
     ${CMAKE_CURRENT_LIST_DIR}/RiuWellLogImportWizard.h
)

source_group(
  "CommandFeature\\Osdu"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
