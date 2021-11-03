set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamline.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineGeneratorBase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineDataAccess.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineGenerator.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineGeneratorBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineDataAccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStreamlineGenerator.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Streamlines"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
