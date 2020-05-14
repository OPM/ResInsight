
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportColorCategoriesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCopyStandardLegendFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicInsertColorLegendFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicInsertColorLegendItemFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportColorCategoriesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCopyStandardLegendFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicInsertColorLegendFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicInsertColorLegendItemFeature.cpp
)


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ColorLegend" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )

# cotire
caf_cotire_start_unity_at_first_item(SOURCE_GROUP_SOURCE_FILES)
