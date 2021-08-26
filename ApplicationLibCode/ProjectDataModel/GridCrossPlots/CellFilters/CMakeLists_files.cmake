set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellFilter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellFilterCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellPropertyFilter.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotCellPropertyFilter.cpp)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\GridCrossPlots\\CellFilters"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
