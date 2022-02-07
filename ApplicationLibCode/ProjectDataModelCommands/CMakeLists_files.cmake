set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryResampleData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcProject.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcElasticProperties.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelTemplateCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModel.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcSurfaceCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerDouble.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerString.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerTime.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogTrack.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellPathGeometryDef.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcModeledWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcFractureTemplateCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimcIntersection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcSummaryResampleData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcProject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcElasticProperties.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelTemplateCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModelPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcStimPlanModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcSurfaceCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerDouble.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerString.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcDataContainerTime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellLogTrack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellPathGeometryDef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcModeledWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcFractureTemplateCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimcIntersection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModelCommands"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
