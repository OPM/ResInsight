
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicPasteEllipseFractureFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteStimPlanFractureFeature.h
${CEE_CURRENT_LIST_DIR}RicConvertAllFractureTemplatesToFieldFeature.h
${CEE_CURRENT_LIST_DIR}RicConvertAllFractureTemplatesToMetricFeature.h
${CEE_CURRENT_LIST_DIR}RicConvertFractureTemplateUnitFeature.h
${CEE_CURRENT_LIST_DIR}RicFractureNameGenerator.h
${CEE_CURRENT_LIST_DIR}RicNewEllipseFractureTemplateFeature.h
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureAtPosFeature.h
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureFeature.h
${CEE_CURRENT_LIST_DIR}RicNewStimPlanFractureTemplateFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureAtPosFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicPasteEllipseFractureFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteStimPlanFractureFeature.cpp
${CEE_CURRENT_LIST_DIR}RicConvertAllFractureTemplatesToFieldFeature.cpp
${CEE_CURRENT_LIST_DIR}RicConvertAllFractureTemplatesToMetricFeature.cpp
${CEE_CURRENT_LIST_DIR}RicConvertFractureTemplateUnitFeature.cpp
${CEE_CURRENT_LIST_DIR}RicFractureNameGenerator.cpp
${CEE_CURRENT_LIST_DIR}RicNewEllipseFractureTemplateFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureAtPosFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewStimPlanFractureTemplateFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureAtPosFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureFeature.cpp
)


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Fracture" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
