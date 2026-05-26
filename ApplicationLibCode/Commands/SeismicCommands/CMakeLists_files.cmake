set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportSeismicFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewInlineSeismicSectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewXlineSeismicSectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewZSliceSeismicSectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSeismicSectionFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineSeismicSectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellpathSeismicSectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSeismicSectionFromIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSeismicDifferenceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSeismicViewFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
