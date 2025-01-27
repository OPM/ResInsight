set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDataCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDataInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicDifferenceData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSectionCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicAlphaMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSEGYConvertOptions.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicView.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

