
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicfCloseProject.h
${CMAKE_CURRENT_LIST_DIR}/RicfCommandFileExecutor.h
${CMAKE_CURRENT_LIST_DIR}/RicfComputeCaseGroupStatistics.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportMsw.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportMultiCaseSnapshots.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportProperty.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportSimWellFractureCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportSnapshots.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPathCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfLoadCase.h
${CMAKE_CURRENT_LIST_DIR}/RicfOpenProject.h
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceCase.h
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceSourceCases.h
${CMAKE_CURRENT_LIST_DIR}/RicfRunOctaveScript.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetExportFolder.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetMainWindowSize.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetStartDir.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetTimeStep.h
${CMAKE_CURRENT_LIST_DIR}/RicfScaleFractureTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetFractureContainment.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateMultipleFractures.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPaths.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportVisibleCells.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportPropertyInViews.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportLgrForCompletions.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicfCloseProject.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCommandFileExecutor.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfComputeCaseGroupStatistics.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportMsw.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportMultiCaseSnapshots.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportProperty.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportSimWellFractureCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportSnapshots.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPathCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfLoadCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfOpenProject.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceSourceCases.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfRunOctaveScript.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetExportFolder.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetMainWindowSize.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetStartDir.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetTimeStep.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfScaleFractureTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetFractureContainment.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateMultipleFractures.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPaths.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportVisibleCells.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportPropertyInViews.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportLgrForCompletions.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFileInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
