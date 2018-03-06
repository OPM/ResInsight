
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicPasteEllipseFractureFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteStimPlanFractureFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToFieldFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToMetricFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateDuplicateTemplateInOtherUnitSystemFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicFractureNameGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RicNewEllipseFractureTemplateFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellFractureAtPosFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellFractureFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanFractureTemplateFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureAtPosFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicPasteEllipseFractureFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteStimPlanFractureFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToFieldFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToMetricFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateDuplicateTemplateInOtherUnitSystemFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFractureNameGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewEllipseFractureTemplateFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellFractureAtPosFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellFractureFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanFractureTemplateFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureAtPosFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureFeature.cpp
)


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Fracture" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
