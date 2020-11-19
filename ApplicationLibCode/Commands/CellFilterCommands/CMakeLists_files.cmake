
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewUserDefinedFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewCellRangeFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceIFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceJFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceKFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSlice3dviewFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewUserDefinedFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewCellRangeFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceIFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceJFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSliceKFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRangeFilterSlice3dviewFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND COMMAND_CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)


source_group( "CommandFeature\\CellFilterCommands" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
