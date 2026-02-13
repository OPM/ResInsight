set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFileSetFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateEnsembleFromFileSetFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateReservoirGridEnsembleFromFileSetFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFileSetFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateEnsembleFromFileSetFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateReservoirGridEnsembleFromFileSetFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
