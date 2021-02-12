
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimStreamline.h
${CMAKE_CURRENT_LIST_DIR}/RimStreamlineInViewCollection.h
${CMAKE_CURRENT_LIST_DIR}/StreamlineGenerator.h
${CMAKE_CURRENT_LIST_DIR}/StreamlineGeneratorBase.h
${CMAKE_CURRENT_LIST_DIR}/StreamlineDataAccess.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimStreamline.cpp
${CMAKE_CURRENT_LIST_DIR}/RimStreamlineInViewCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/StreamlineGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/StreamlineGeneratorBase.cpp
${CMAKE_CURRENT_LIST_DIR}/StreamlineDataAccess.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Streamlines" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
