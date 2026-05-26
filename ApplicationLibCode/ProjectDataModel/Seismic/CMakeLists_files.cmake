set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDataCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSectionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicAlphaMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSEGYConvertOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDataInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDifferenceData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicView.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
