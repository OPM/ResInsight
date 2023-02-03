set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSectionCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSectionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSeismicSection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Seismic"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
