set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlotDataEntry.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotDataFilterCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotDataFilterItem.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlotDataEntry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnalysisPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotDataFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotDataFilterItem.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND QT_MOC_HEADERS)

source_group(
  "ProjectDataModel\\AnalysisPlots"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
