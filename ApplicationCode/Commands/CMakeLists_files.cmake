
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.h
${CEE_CURRENT_LIST_DIR}RicPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterExecImpl.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.h
${CEE_CURRENT_LIST_DIR}RicAddEclipseInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.h
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicCreateGridCaseGroupFeature.h
${CEE_CURRENT_LIST_DIR}RicNewStatisticsCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicComputeStatisticsFeature.h

${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsDeleteAllFeature.h

${CEE_CURRENT_LIST_DIR}RicWellLogsImportFileFeature.h

${CEE_CURRENT_LIST_DIR}RicTileWindowsFeature.h
${CEE_CURRENT_LIST_DIR}RicLaunchUnitTestsFeature.h


# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterExecImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.cpp
${CEE_CURRENT_LIST_DIR}RicAddEclipseInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.cpp
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicCreateGridCaseGroupFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewStatisticsCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicComputeStatisticsFeature.cpp

${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsDeleteAllFeature.cpp

${CEE_CURRENT_LIST_DIR}RicTileWindowsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicLaunchUnitTestsFeature.cpp


# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
