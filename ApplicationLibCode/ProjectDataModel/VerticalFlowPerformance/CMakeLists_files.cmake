set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpPlot_deprecated.h
    ${CMAKE_CURRENT_LIST_DIR}/RimCustomVfpPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpTableData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpTable.h
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpDataCollection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpDefines.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpPlot_deprecated.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCustomVfpPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpTableData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimVfpDataCollection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "VFP Plots" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
                    ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
