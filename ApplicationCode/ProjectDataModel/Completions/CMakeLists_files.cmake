
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimCompletionCellIntersectionCalc.h
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesMultipleSubs.h
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesPipeProperties.h
${CMAKE_CURRENT_LIST_DIR}/RimFishboneWellPath.h
${CMAKE_CURRENT_LIST_DIR}/RimFishboneWellPathCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimPerforationCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimPerforationInterval.h
${CMAKE_CURRENT_LIST_DIR}/RimWellPathCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RimEllipseFractureTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RimFracture.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureContainment.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureContainmentTools.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureExportSettings.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureTemplateCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSimWellFracture.h
${CMAKE_CURRENT_LIST_DIR}/RimSimWellFractureCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimStimPlanFractureTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RimWellPathFracture.h
${CMAKE_CURRENT_LIST_DIR}/RimWellPathFractureCollection.h
${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogCurveCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimMswCompletionParameters.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimCompletionCellIntersectionCalc.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesMultipleSubs.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFishbonesPipeProperties.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFishboneWellPath.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFishboneWellPathCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimPerforationCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimPerforationInterval.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellPathCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEllipseFractureTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFracture.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureContainment.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureContainmentTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureExportSettings.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureTemplateCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSimWellFracture.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSimWellFractureCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimStimPlanFractureTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellPathFracture.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellPathFractureCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogCurveCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimMswCompletionParameters.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Completions" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
