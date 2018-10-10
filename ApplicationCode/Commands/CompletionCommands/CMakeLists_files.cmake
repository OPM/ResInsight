
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicEditPerforationCollectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesLateralsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsAtMeasuredDepthFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalAtMeasuredDepthFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewValveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportCompletionsFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportPerforationIntervalsFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicEditPerforationCollectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesLateralsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsAtMeasuredDepthFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalAtMeasuredDepthFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewValveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportCompletionsFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportPerforationIntervalsFeature.cpp
)


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Completion" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
