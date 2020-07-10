
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RiaApplication.h
${CMAKE_CURRENT_LIST_DIR}/RiaConsoleApplication.h
${CMAKE_CURRENT_LIST_DIR}/RiaGuiApplication.h
${CMAKE_CURRENT_LIST_DIR}/RiaCompletionTypeCalculationScheduler.h
${CMAKE_CURRENT_LIST_DIR}/RiaDefines.h
${CMAKE_CURRENT_LIST_DIR}/RiaFractureDefines.h
${CMAKE_CURRENT_LIST_DIR}/RiaPreferences.h
${CMAKE_CURRENT_LIST_DIR}/RiaPorosityModel.h
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveDefinition.h
${CMAKE_CURRENT_LIST_DIR}/RiaCurveSetDefinition.h
${CMAKE_CURRENT_LIST_DIR}/RiaRftPltCurveDefinition.h
${CMAKE_CURRENT_LIST_DIR}/RiaViewRedrawScheduler.h
${CMAKE_CURRENT_LIST_DIR}/RiaPlotWindowRedrawScheduler.h
${CMAKE_CURRENT_LIST_DIR}/RiaMemoryCleanup.h
${CMAKE_CURRENT_LIST_DIR}/RiaFontCache.h
${CMAKE_CURRENT_LIST_DIR}/RiaEclipseFileNameTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaFeatureCommandContext.h
${CMAKE_CURRENT_LIST_DIR}/RiaStringListSerializer.h
${CMAKE_CURRENT_LIST_DIR}/RiaNncDefines.h
${CMAKE_CURRENT_LIST_DIR}/RiaFractureModelDefines.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RiaApplication.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaConsoleApplication.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaGuiApplication.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaCompletionTypeCalculationScheduler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaDefines.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaFractureDefines.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaMain.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaPreferences.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaPorosityModel.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveDefinition.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaCurveSetDefinition.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaRftPltCurveDefinition.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaViewRedrawScheduler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaPlotWindowRedrawScheduler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaMemoryCleanup.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaFontCache.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaEclipseFileNameTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaFeatureCommandContext.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaStringListSerializer.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaNncDefines.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaFractureModelDefines.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CMAKE_CURRENT_LIST_DIR}/RiaConsoleApplication.h
${CMAKE_CURRENT_LIST_DIR}/RiaGuiApplication.h
${CMAKE_CURRENT_LIST_DIR}/RiaCompletionTypeCalculationScheduler.h
${CMAKE_CURRENT_LIST_DIR}/RiaViewRedrawScheduler.h
${CMAKE_CURRENT_LIST_DIR}/RiaPlotWindowRedrawScheduler.h
)


source_group( "Application" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
