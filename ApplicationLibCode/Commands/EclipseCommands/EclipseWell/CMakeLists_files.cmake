set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseWellFeatureImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseWellShowFeatures.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateWellPathFromSimulationWellFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseWellFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseWellShowFeatures.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateWellPathFromSimulationWellFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
