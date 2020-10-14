
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellLogTools.h
${CMAKE_CURRENT_LIST_DIR}/RicCloseCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCloseSummaryCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCloseSummaryCaseInCollectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCloseObservedDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateSummaryCaseCollectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.h
${CMAKE_CURRENT_LIST_DIR}/RicNewViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewContourMapViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterExecImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterInsertExec.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterInsertFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewExec.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceIFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceJFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceKFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportFaciesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportFormationNamesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReloadFormationNamesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReloadWellPathFormationNamesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSliceRangeFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHideIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHideIntersectionBoxFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportElementPropertyFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSelectColorResult.h

${CMAKE_CURRENT_LIST_DIR}/RicTogglePerspectiveViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryCasesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedDataInMenuFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedFmuDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedFmuDataInMenuFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportGeneralDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFeatureImpl.h

${CMAKE_CURRENT_LIST_DIR}/RicSelectOrCreateViewFeatureImpl.h

${CMAKE_CURRENT_LIST_DIR}/RicPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/Ric3dViewPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicContourMapPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicVec3dPickEventHandler.h

# General delete of any object in a child array field
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemExec.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemExecData.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteSubItemsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteSummaryCaseCollectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellMeasurementFilePathFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReloadWellMeasurementsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellMeasurementImportTools.h
${CMAKE_CURRENT_LIST_DIR}/RicImportElasticPropertiesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicElasticPropertiesImportTools.h
${CMAKE_CURRENT_LIST_DIR}/RicFaciesPropertiesImportTools.h

${CMAKE_CURRENT_LIST_DIR}/RicCloseSourSimDataFeature.h

${CMAKE_CURRENT_LIST_DIR}/RicReloadCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReplaceCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReloadSummaryCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReplaceSummaryCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicFlyToObjectFeature.h

${CMAKE_CURRENT_LIST_DIR}/RicGridStatisticsDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicShowGridStatisticsFeature.h

${CMAKE_CURRENT_LIST_DIR}/RicRecursiveFileSearchDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCaseRestartDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryGroupFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicConvertGroupToEnsembleFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicResampleDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateTemporaryLgrFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteTemporaryLgrsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportContourMapToTextFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportContourMapToTextUi.h
${CMAKE_CURRENT_LIST_DIR}/RicNewMultiPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureModelPlotToFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicStackSelectedCurvesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicUnstackSelectedCurvesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicThemeColorEditorFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewCustomObjectiveFunctionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewCustomObjectiveFunctionWeightFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteCustomObjectiveFunctionFeature.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellLogTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCloseCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCloseSummaryCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCloseSummaryCaseInCollectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCloseObservedDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateSummaryCaseCollectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechCopyCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterInsertExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewInViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicGeoMechPropertyFilterNewExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewContourMapViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterExecImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterInsertExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterInsertFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceIFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceJFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRangeFilterNewSliceKFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportFaciesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportFormationNamesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReloadFormationNamesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReloadWellPathFormationNamesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSliceRangeFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHideIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHideIntersectionBoxFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportElementPropertyFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSelectColorResult.cpp

${CMAKE_CURRENT_LIST_DIR}/RicTogglePerspectiveViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportGeoMechCaseTimeStepFilterFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryCasesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedDataInMenuFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedFmuDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportObservedFmuDataInMenuFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportGeneralDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFeatureImpl.cpp

${CMAKE_CURRENT_LIST_DIR}/RicSelectOrCreateViewFeatureImpl.cpp

${CMAKE_CURRENT_LIST_DIR}/Ric3dViewPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicContourMapPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicVec3dPickEventHandler.cpp

# General delete of any object in a child array field
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemExecData.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteItemFeature.cpp

${CMAKE_CURRENT_LIST_DIR}/RicDeleteSubItemsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteSummaryCaseCollectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellMeasurementFilePathFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReloadWellMeasurementsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellMeasurementImportTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportElasticPropertiesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicElasticPropertiesImportTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFaciesPropertiesImportTools.cpp

${CMAKE_CURRENT_LIST_DIR}/RicCloseSourSimDataFeature.cpp

${CMAKE_CURRENT_LIST_DIR}/RicReloadCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReplaceCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReloadSummaryCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReplaceSummaryCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFlyToObjectFeature.cpp

${CMAKE_CURRENT_LIST_DIR}/RicGridStatisticsDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowGridStatisticsFeature.cpp

${CMAKE_CURRENT_LIST_DIR}/RicRecursiveFileSearchDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCaseRestartDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportSummaryGroupFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicConvertGroupToEnsembleFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicResampleDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateTemporaryLgrFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteTemporaryLgrsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportContourMapToTextFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportContourMapToTextUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewMultiPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureModelPlotToFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicStackSelectedCurvesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicUnstackSelectedCurvesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicThemeColorEditorFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewCustomObjectiveFunctionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewCustomObjectiveFunctionWeightFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteCustomObjectiveFunctionFeature.cpp
)


list(APPEND COMMAND_CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND COMMAND_CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND COMMAND_QT_MOC_HEADERS
${CMAKE_CURRENT_LIST_DIR}/RicGridStatisticsDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicRecursiveFileSearchDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCaseRestartDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicResampleDialog.h
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
