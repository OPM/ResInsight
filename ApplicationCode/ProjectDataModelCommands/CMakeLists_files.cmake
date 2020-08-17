
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryCase.h
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryResampleData.h
${CMAKE_CURRENT_LIST_DIR}/RimcProject.h
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelPlot.h

${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerDouble.h
${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerString.h
${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerTime.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcSummaryResampleData.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcProject.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcFractureModelPlot.cpp

${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerDouble.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerString.cpp
${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerTime.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModelCommands" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
