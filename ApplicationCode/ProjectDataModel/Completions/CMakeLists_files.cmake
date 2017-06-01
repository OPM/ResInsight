
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RimCompletionCellIntersectionCalc.h
${CEE_CURRENT_LIST_DIR}RimFishbonesCollection.h
${CEE_CURRENT_LIST_DIR}RimFishbonesMultipleSubs.h
${CEE_CURRENT_LIST_DIR}RimFishbonesPipeProperties.h
${CEE_CURRENT_LIST_DIR}RimFishboneWellPath.h
${CEE_CURRENT_LIST_DIR}RimFishboneWellPathCollection.h
${CEE_CURRENT_LIST_DIR}RimPerforationCollection.h
${CEE_CURRENT_LIST_DIR}RimPerforationInterval.h
${CEE_CURRENT_LIST_DIR}RimWellPathCompletions.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RimCompletionCellIntersectionCalc.cpp
${CEE_CURRENT_LIST_DIR}RimFishbonesCollection.cpp
${CEE_CURRENT_LIST_DIR}RimFishbonesMultipleSubs.cpp
${CEE_CURRENT_LIST_DIR}RimFishbonesPipeProperties.cpp
${CEE_CURRENT_LIST_DIR}RimFishboneWellPath.cpp
${CEE_CURRENT_LIST_DIR}RimFishboneWellPathCollection.cpp
${CEE_CURRENT_LIST_DIR}RimPerforationCollection.cpp
${CEE_CURRENT_LIST_DIR}RimPerforationInterval.cpp
${CEE_CURRENT_LIST_DIR}RimWellPathCompletions.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Fishbones" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
