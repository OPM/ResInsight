
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddEclipseInputPropertyFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicComputeStatisticsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCaseGroupFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCaseGroupFromFilesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipseCaseNewGroupExec.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipseCaseNewGroupFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterInsertExec.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterInsertFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewExec.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCasesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportInputEclipseCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewStatisticsCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicApplyPropertyFilterAsCellResultFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCaseTimeStepFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewInViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEclipseHideFaultFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddEclipseInputPropertyFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicComputeStatisticsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCaseGroupFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCaseGroupFromFilesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipseCaseNewGroupExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipseCaseNewGroupFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterInsertExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterInsertFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCasesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportInputEclipseCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewStatisticsCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicApplyPropertyFilterAsCellResultFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportEclipseCaseTimeStepFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterNewInViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEclipseHideFaultFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Eclipse" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
