
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()


set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RiaArgumentParser.h
${CEE_CURRENT_LIST_DIR}RiaDateStringParser.h
${CEE_CURRENT_LIST_DIR}RiaColorTables.h
${CEE_CURRENT_LIST_DIR}RiaColorTools.h
${CEE_CURRENT_LIST_DIR}RiaEclipseUnitTools.h
${CEE_CURRENT_LIST_DIR}RiaImageCompareReporter.h
${CEE_CURRENT_LIST_DIR}RiaImageFileCompare.h
${CEE_CURRENT_LIST_DIR}RiaLogging.h
${CEE_CURRENT_LIST_DIR}RiaProjectModifier.h
${CEE_CURRENT_LIST_DIR}RiaRegressionTest.h
${CEE_CURRENT_LIST_DIR}RiaImportEclipseCaseTools.h
${CEE_CURRENT_LIST_DIR}RiaQDateTimeTools.h
${CEE_CURRENT_LIST_DIR}RiaSummaryTools.h
${CEE_CURRENT_LIST_DIR}RiaWellNameComparer.h
${CEE_CURRENT_LIST_DIR}RiaStdStringTools.h
${CEE_CURRENT_LIST_DIR}RiaSummaryCurveAnalyzer.h
${CEE_CURRENT_LIST_DIR}RiaSimWellBranchTools.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RiaArgumentParser.cpp
${CEE_CURRENT_LIST_DIR}RiaDateStringParser.cpp
${CEE_CURRENT_LIST_DIR}RiaColorTables.cpp
${CEE_CURRENT_LIST_DIR}RiaColorTools.cpp
${CEE_CURRENT_LIST_DIR}RiaEclipseUnitTools.cpp
${CEE_CURRENT_LIST_DIR}RiaImageCompareReporter.cpp
${CEE_CURRENT_LIST_DIR}RiaImageFileCompare.cpp
${CEE_CURRENT_LIST_DIR}RiaLogging.cpp
${CEE_CURRENT_LIST_DIR}RiaProjectModifier.cpp
${CEE_CURRENT_LIST_DIR}RiaRegressionTest.cpp
${CEE_CURRENT_LIST_DIR}RiaImportEclipseCaseTools.cpp
${CEE_CURRENT_LIST_DIR}RiaQDateTimeTools.cpp
${CEE_CURRENT_LIST_DIR}RiaSummaryTools.cpp
${CEE_CURRENT_LIST_DIR}RiaWellNameComparer.cpp
${CEE_CURRENT_LIST_DIR}RiaStdStringTools.cpp
${CEE_CURRENT_LIST_DIR}RiaSummaryCurveAnalyzer.cpp
${CEE_CURRENT_LIST_DIR}RiaSimWellBranchTools.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
)


source_group( "Application\\Tools" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
