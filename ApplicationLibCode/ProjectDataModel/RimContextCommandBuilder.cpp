/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimContextCommandBuilder.h"

#include "RiaApplication.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "PlotTemplates/RimPlotTemplateFolderItem.h"
#include "Rim3dOverlayInfoConfig.h"
#include "Rim3dWellLogCurveCollection.h"
#include "Rim3dWellLogExtractionCurve.h"
#include "Rim3dWellLogFileCurve.h"
#include "Rim3dWellLogRftCurve.h"
#include "RimAnalysisPlot.h"
#include "RimAnalysisPlotCollection.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimBoxIntersection.h"
#include "RimCalcScript.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimCorrelationMatrixPlot.h"
#include "RimCorrelationPlot.h"
#include "RimCorrelationPlotCollection.h"
#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultAddress.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimElasticProperties.h"
#include "RimEllipseFractureTemplate.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleFractureStatisticsCollection.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultReactivationModel.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowDiagSolution.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimGridCaseSurface.h"
#include "RimGridCollection.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimModeledWellPath.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimObservedSummaryData.h"
#include "RimParameterResultCrossPlot.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimPlotDataFilterItem.h"
#include "RimPltPlotCollection.h"
#include "RimPressureDepthData.h"
#include "RimPressureTable.h"
#include "RimProject.h"
#include "RimRftCase.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimScriptCollection.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSeismicViewCollection.h"
#include "RimSimWellFracture.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCollection.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelTemplateCollection.h"
#include "RimStreamlineInViewCollection.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTable.h"
#include "RimSummaryTableCollection.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimVfpPlotCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimVirtualPerforationResults.h"
#include "RimWellAllocationPlot.h"
#include "RimWellIASettings.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilePath.h"
#include "RimWellPath.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#ifdef USE_QTCHARTS
#include "RimEnsembleFractureStatisticsPlotCollection.h"
#include "RimGridStatisticsPlotCollection.h"
#endif

#include "RiuMainWindow.h"

#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"
#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"
#include "cafTreeNode.h"
#include "cvfAssert.h"

#include <QDir>
#include <QIcon>
#include <QMenu>
#include <QString>
#include <QStringList>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::CmdFeatureMenuBuilder RimContextCommandBuilder::commandsFromSelection()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    caf::PdmUiItem* firstUiItem = nullptr;
    {
        if ( !uiItems.empty() )
        {
            firstUiItem = uiItems[0];
        }
    }

    if ( dynamic_cast<cafTreeNode*>( firstUiItem ) ) return {};

    caf::CmdFeatureMenuBuilder menuBuilder;

    if ( uiItems.size() == 1 )
    {
        if ( dynamic_cast<RimEclipseCaseCollection*>( firstUiItem ) )
        {
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportEclipseCaseFeature";
            menuBuilder << "RicImportEclipseCasesFeature";
            menuBuilder << "RicImportInputEclipseCaseFeature";
            menuBuilder << "RicCreateGridCaseGroupFromFilesFeature";
            menuBuilder.subMenuEnd();
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if ( dynamic_cast<RimGeoMechView*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";

            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";

            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "RicExportContourMapToTextFeature";
        }
        else if ( dynamic_cast<RimEclipseView*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteEclipseViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";
            menuBuilder << "RicCreateGridCrossPlotFeature";
            menuBuilder << "RicCreateSaturationPressurePlotsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "RicExportEclipseInputGridFeature";
            menuBuilder << "RicSaveEclipseResultAsInputPropertyFeature";
            menuBuilder << "RicExportContourMapToTextFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
            menuBuilder << "RicAddEclipseInputPropertyFeature";
        }
        else if ( dynamic_cast<RimEclipseContourMapViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewContourMapViewFeature";
        }
        else if ( dynamic_cast<RimGeoMechContourMapViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewContourMapViewFeature";
        }
        else if ( dynamic_cast<RimCaseCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewStatisticsCaseFeature";
        }
        else if ( dynamic_cast<RimGeoMechModels*>( firstUiItem ) )
        {
            menuBuilder << "RicImportGeoMechCaseFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimEclipseStatisticsCase*>( firstUiItem ) )
        {
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicComputeStatisticsFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimEclipseCase*>( firstUiItem ) )
        {
            menuBuilder << "RicRenameCaseFeature";
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "RicPasteEclipseViewsFeature";
            menuBuilder << "Separator";

            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
            menuBuilder << "RicEclipseCaseNewGroupFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimGridInfoCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicExportCompletionsForTemporaryLgrsFeature";
            menuBuilder << "RicDeleteTemporaryLgrsFeature";
        }
        else if ( dynamic_cast<RimGeoMechCase*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicImportElementPropertyFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicGeoMechCopyCaseFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimIdenticalGridCaseGroup*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if ( dynamic_cast<RimEclipseCellColors*>( firstUiItem ) )
        {
            menuBuilder << "RicSaveEclipseResultAsInputPropertyFeature";
            menuBuilder << "RicExportEclipseInputGridFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
            menuBuilder << "RicCreateGridCrossPlotFeature";
            menuBuilder << "RicAddEclipseInputPropertyFeature";
            menuBuilder << "RicShowGridCalculatorFeature";
        }
        else if ( dynamic_cast<RimEclipseInputProperty*>( firstUiItem ) )
        {
            menuBuilder << "RicSaveEclipseInputPropertyFeature";
        }
        else if ( dynamic_cast<RimEclipsePropertyFilterCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicEclipsePropertyFilterNewFeature";
        }
        else if ( dynamic_cast<RimEclipsePropertyFilter*>( firstUiItem ) )
        {
            menuBuilder << "RicEclipsePropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if ( dynamic_cast<RimGeoMechPropertyFilterCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicGeoMechPropertyFilterNewFeature";
        }
        else if ( dynamic_cast<RimGeoMechPropertyFilter*>( firstUiItem ) )
        {
            menuBuilder << "RicGeoMechPropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if ( dynamic_cast<RimWellPathCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewEditableWellPathFeature";
            menuBuilder << "RicPasteModeledWellPathFeature";
            menuBuilder << "RicCreateEnsembleWellLogFeature";
            menuBuilder << "RicCreateDepthAdjustedLasFilesFeature";
            menuBuilder.addSeparator();
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellPathsImportSsihubFeature";
            menuBuilder << "RicWellPathFormationsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";
            menuBuilder << "RicReloadWellPathFormationNamesFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicWellPathImportPerforationIntervalsFeature";
            menuBuilder << "RicImportWellMeasurementsFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder.subMenuStart( "Export Well Paths", QIcon( ":/Save.svg" ) );
            menuBuilder << "RicExportSelectedWellPathsFeature";
            menuBuilder << "RicExportVisibleWellPathsFeature";
            menuBuilder.subMenuEnd();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellMeasurementFilePath*>( firstUiItem ) )
        {
            menuBuilder << "RicReloadWellMeasurementsFeature";
            menuBuilder << "RicDeleteWellMeasurementFilePathFeature";
            menuBuilder << "RicImportWellMeasurementsFeature";
        }
        else if ( dynamic_cast<RimWellMeasurementCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportWellMeasurementsFeature";
        }
        else if ( dynamic_cast<RimWellPath*>( firstUiItem ) )
        {
            menuBuilder << "RicNewEditableWellPathFeature";
            menuBuilder << "RicNewWellPathLateralFeature";
            menuBuilder << "RicDuplicateWellPathFeature";

            menuBuilder.addSeparator();
            menuBuilder << "RicSetParentWellPathFeature";

            menuBuilder.addSeparator();
            menuBuilder << "RicNewWellPathIntersectionFeature";

            appendCreateCompletions( menuBuilder );
            menuBuilder.addSeparator();
            appendImportMenu( menuBuilder );
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
            menuBuilder.addSeparator();
            appendExportWellPaths( menuBuilder );
            menuBuilder.addSeparator();

            menuBuilder << "RicCreateEnsembleWellLogFeature";
            menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "RicNewWellLogExtractionCurveFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.subMenuStart( "3D Well Log Curves", QIcon( ":/WellLogCurve16x16.png" ) );
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();

            menuBuilder << "RicLinkWellPathFeature";
            menuBuilder << "RicDeleteWellPathFeature";

            menuBuilder.addSeparator();

            if ( auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( firstUiItem ) )
            {
                menuBuilder << "RicShowWellPlanFeature";

                if ( modeledWellPath->isTopLevelWellPath() )
                {
                    menuBuilder << "RicCreateMultipleWellPathLaterals";
                }
            }
        }
        else if ( dynamic_cast<RimWellPathCompletions*>( firstUiItem ) )
        {
            menuBuilder.subMenuStart( "Create Completions", QIcon( ":/CompletionsSymbol16x16.png" ) );
            menuBuilder << "RicNewPerforationIntervalFeature";
            menuBuilder << "RicNewFishbonesSubsFeature";
            menuBuilder << "RicNewWellPathFractureFeature";
            menuBuilder.subMenuEnd();
            menuBuilder << "RicCreateTemporaryLgrFeature";
            menuBuilder << "RicNewStimPlanModelFeature";
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimPerforationCollection*>( firstUiItem ) || dynamic_cast<RimPerforationInterval*>( firstUiItem ) )
        {
            menuBuilder << "RicNewPerforationIntervalFeature";
            if ( dynamic_cast<RimPerforationInterval*>( firstUiItem ) ) menuBuilder << "RicNewValveFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicEditPerforationCollectionFeature";
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimFishbonesCollection*>( firstUiItem ) || dynamic_cast<RimFishbones*>( firstUiItem ) )
        {
            menuBuilder << "RicNewFishbonesSubsFeature";
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellPathFractureCollection*>( firstUiItem ) || dynamic_cast<RimWellPathFracture*>( firstUiItem ) )
        {
            menuBuilder << "RicNewWellPathFractureFeature";
            if ( dynamic_cast<RimWellPathFracture*>( firstUiItem ) ) menuBuilder << "RicPlaceThermalFractureUsingTemplateDataFeature";
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellPathAttributeCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicDeleteWellPathAttributeFeature";
        }
        else if ( dynamic_cast<RimStimPlanModel*>( firstUiItem ) )
        {
            menuBuilder << "RicNewStimPlanModelFeature";
            menuBuilder << "RicNewStimPlanModelPlotFeature";
            menuBuilder << "RicExportStimPlanModelToFileFeature";
        }
        else if ( dynamic_cast<RimFaultReactivationModel*>( firstUiItem ) )
        {
            menuBuilder << "RicRunFaultReactModelingFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowFaultReactModelFeature";
            menuBuilder << "RicExportInpFileFeature";
        }
        else if ( dynamic_cast<RimPressureTable*>( firstUiItem ) )
        {
            menuBuilder << "RicNewPressureTableItemFeature";
        }
        else if ( dynamic_cast<RimStimPlanModelCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewStimPlanModelFeature";
        }
        else if ( dynamic_cast<Rim3dWellLogCurveCollection*>( firstUiItem ) || dynamic_cast<Rim3dWellLogExtractionCurve*>( firstUiItem ) ||
                  dynamic_cast<Rim3dWellLogFileCurve*>( firstUiItem ) || dynamic_cast<Rim3dWellLogRftCurve*>( firstUiItem ) )
        {
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
        }
        else if ( dynamic_cast<RimWellLogLasFile*>( firstUiItem ) )
        {
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";

            menuBuilder << "Separator";

            menuBuilder.subMenuStart( "Move LAS file to well path" );

            RimWellPath* parentWellPath     = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath>();
            QString      parentWellPathName = parentWellPath ? parentWellPath->name() : "";

            for ( RimWellPath* wellPath : allWellPaths() )
            {
                if ( wellPath->name() != parentWellPathName )
                {
                    menuBuilder.addCmdFeatureWithUserData( "RicMoveWellLogFilesFeature", wellPath->name(), wellPath->name() );
                }
            }
            menuBuilder.subMenuEnd();
        }
        else if ( dynamic_cast<RimCalcScript*>( firstUiItem ) )
        {
            menuBuilder << "RicEditScriptFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewOctaveScriptFeature";
#ifdef ENABLE_GRPC
            menuBuilder << "RicNewPythonScriptFeature";
#endif
            menuBuilder << "Separator";
            menuBuilder << "RicExecuteScriptFeature";
        }
        else if ( dynamic_cast<RimScriptCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewOctaveScriptFeature";
#ifdef ENABLE_GRPC
            menuBuilder << "RicNewPythonScriptFeature";
#endif
            menuBuilder << "Separator";
            menuBuilder << "RicAddScriptPathFeature";
            menuBuilder << "RicRefreshScriptsFeature";
            menuBuilder << "RicExecuteLastUsedScriptFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteScriptPathFeature";
        }
        else if ( dynamic_cast<RimViewController*>( firstUiItem ) )
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
        }
        else if ( dynamic_cast<RimViewLinker*>( firstUiItem ) || dynamic_cast<RimViewLinkerCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteAllLinkedViewsFeature";
        }
        else if ( dynamic_cast<RimWellLogPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
            menuBuilder << "RicNewMultiPhaseRftSegmentPlotFeature";
            menuBuilder << "RicNewRftSegmentWellLogPlotFeature";
        }
        else if ( dynamic_cast<RimRftPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewMultiPhaseRftSegmentPlotFeature";
            menuBuilder << "RicNewRftSegmentWellLogPlotFeature";
        }
        else if ( dynamic_cast<RimPltPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewPltPlotFeature";
        }
        else if ( dynamic_cast<RimVfpPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewVfpPlotFeature";
        }
        else if ( dynamic_cast<RimSummaryMultiPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSummaryMultiPlotFeature";
            menuBuilder << "RicOpenSummaryPlotEditorFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "RicPasteSummaryMultiPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
        }
        else if ( dynamic_cast<RimAnalysisPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewAnalysisPlotFeature";
        }
        else if ( dynamic_cast<RimAnalysisPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicNewAnalysisPlotFeature";
            menuBuilder << "RicNewPlotDataFilterFeature";
        }
        else if ( dynamic_cast<RimCorrelationPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewCorrelationPlotFeature";
            menuBuilder << "RicNewCorrelationMatrixPlotFeature";
            menuBuilder << "RicNewParameterResultCrossPlotFeature";
            menuBuilder << "RicNewCorrelationReportPlotFeature";
        }
        else if ( dynamic_cast<RimCorrelationPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicNewCorrelationPlotFeature";
        }
        else if ( dynamic_cast<RimCorrelationMatrixPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicNewCorrelationMatrixPlotFeature";
        }
        else if ( dynamic_cast<RimParameterResultCrossPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicNewParameterResultCrossPlotFeature";
        }
        else if ( dynamic_cast<RimPlotDataFilterCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewPlotDataFilterFeature";
        }
        else if ( dynamic_cast<RimPlotDataFilterItem*>( firstUiItem ) )
        {
            menuBuilder << "RicNewPlotDataFilterFeature";
        }
        else if ( dynamic_cast<RimSummaryTableCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSummaryTableFeature";
        }
        else if ( dynamic_cast<RimSummaryTable*>( firstUiItem ) )
        {
            menuBuilder << "RicDuplicateSummaryTableFeature";
        }
        else if ( dynamic_cast<RimWellRftPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateRftPlotsFeature";
        }
        else if ( dynamic_cast<RimWellLogPlot*>( firstUiItem ) && !dynamic_cast<RimWellPltPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotTrackFeature";
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
        }
        else if ( dynamic_cast<RimWellLogTrack*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "RicPasteWellLogCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogExtractionCurveFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "RicNewWellMeasurementCurveFeature";
            menuBuilder << "RicNewWellLogCalculatedCurveFeature";
            menuBuilder << "RicNewEnsembleWellLogCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteSubPlotFeature";
        }
        else if ( dynamic_cast<RimWellLogCurve*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteWellLogCurveFeature";
        }
        else if ( dynamic_cast<RimGridCrossPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateGridCrossPlotFeature";
        }
        else if ( dynamic_cast<RimSaturationPressurePlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateSaturationPressurePlotsFeature";
        }
#ifdef USE_QTCHARTS
        else if ( dynamic_cast<RimGridStatisticsPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateGridStatisticsPlotFeature";
        }
        else if ( dynamic_cast<RimEnsembleFractureStatisticsPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateEnsembleFractureStatisticsPlotFeature";
        }
#endif
        else if ( dynamic_cast<RimGridCrossPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteGridCrossPlotDataSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCreateGridCrossPlotDataSetFeature";
            menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
        }
        else if ( dynamic_cast<RimGridCrossPlotDataSet*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteGridCrossPlotDataSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
        }
        else if ( dynamic_cast<RimSummaryPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicDuplicateSummaryPlotFeature";
            menuBuilder << "RicSplitMultiPlotFeature";
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewPlotAxisPropertiesFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicAsciiExportSummaryPlotFeature";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicViewZoomAllFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteSubPlotFeature";
        }
        else if ( dynamic_cast<RimSummaryCurve*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteSummaryCurveFeature";

            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicDuplicateSummaryCurveFeature";
            menuBuilder << "RicCreateDeclineCurvesFeature";
            menuBuilder << "RicCreateRegressionAnalysisCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSetSourceSteppingSummaryCurveFeature";
            menuBuilder << "RicClearSourceSteppingSummaryCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicStackSelectedCurvesFeature";
            menuBuilder << "RicUnstackSelectedCurvesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryCurveCalculationFeature";
        }
        else if ( dynamic_cast<RimSummaryCurveCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveSetCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveSet*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSetSourceSteppingEnsembleCurveSetFeature";
            menuBuilder << "RicClearSourceSteppingEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewEnsembleCurveFilterFeature";
            menuBuilder << "RicCreateRegressionAnalysisCurveFeature";
        }
        else if ( dynamic_cast<RimCustomObjectiveFunctionCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewCustomObjectiveFunctionFeature";
        }
        else if ( dynamic_cast<RimCustomObjectiveFunction*>( firstUiItem ) )
        {
            menuBuilder << "RicNewCustomObjectiveFunctionWeightFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveFilterCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewEnsembleCurveFilterFeature";
        }
        else if ( dynamic_cast<RimSummaryCaseMainCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportRevealSummaryCaseFeature";
            menuBuilder << "RicImportStimPlanSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder << "RicNewDerivedEnsembleFeature";
            menuBuilder << "RicNewDerivedSummaryFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
        }
        else if ( dynamic_cast<RimSummaryCaseCollection*>( firstUiItem ) )
        {
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSummaryMultiPlotFeature";
            menuBuilder << "RicNewDerivedEnsembleFeature";
            menuBuilder << "RicOpenSummaryPlotEditorFeature";
            menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
            menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicConvertGroupToEnsembleFeature";
            menuBuilder.addSeparator();
        }
        else if ( dynamic_cast<RimSummaryCase*>( firstUiItem ) )
        {
            menuBuilder << "RicShowDataSourcesForRealization";
            menuBuilder.addSeparator();

            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSummaryMultiPlotFeature";
            menuBuilder << "RicOpenSummaryPlotEditorFeature";
            menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
            menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicImportGridModelFromSummaryCaseFeature";

            if ( !dynamic_cast<RimObservedSummaryData*>( firstUiItem ) )
            {
                menuBuilder << "RicShowSummaryCurveCalculatorFeature";
            }
        }
        else if ( dynamic_cast<RimWellLogFileChannel*>( firstUiItem ) )
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if ( dynamic_cast<RimIntersectionCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimExtrudedCurveIntersection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateSurfaceIntersectionBandFeature";
            menuBuilder << "RicCreateSurfaceIntersectionCurveFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicSeismicSectionFromIntersectionFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewIntersectionViewFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimBoxIntersection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimIntersectionResultsDefinitionCollection*>( firstUiItem ) ||
                  dynamic_cast<RimIntersectionResultDefinition*>( firstUiItem ) )
        {
            menuBuilder << "RicAppendSeparateIntersectionResultFeature";
        }
        else if ( dynamic_cast<RimSimWellInView*>( firstUiItem ) )
        {
            menuBuilder << "RicNewWellLogExtractionCurveFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewSimWellIntersectionFeature";

            menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicPlotProductionRateFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicShowCumulativePhasePlotFeature";
            menuBuilder.subMenuEnd();

            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if ( dynamic_cast<RimSimWellInViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if ( dynamic_cast<RimColorLegendCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicInsertColorLegendFeature";
            menuBuilder << "RicImportColorCategoriesFeature"; // import of color legend from LYR file
        }
        else if ( dynamic_cast<RimColorLegend*>( firstUiItem ) )
        {
            menuBuilder << "RicInsertColorLegendFeature";
            menuBuilder << "RicCopyStandardLegendFeature";
            menuBuilder << "RicInsertColorLegendItemFeature";
        }
        else if ( dynamic_cast<RimColorLegendItem*>( firstUiItem ) )
        {
            menuBuilder << "RicInsertColorLegendItemFeature";
        }

        else if ( dynamic_cast<RimFormationNames*>( firstUiItem ) )
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFormationNamesCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFaultInView*>( firstUiItem ) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if ( dynamic_cast<RimWellAllocationPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicAddStoredWellAllocationPlotFeature";
        }
        else if ( dynamic_cast<RimFlowCharacteristicsPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicAddStoredFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<RimFlowDiagSolution*>( firstUiItem ) )
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<RimFlowPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<Rim3dOverlayInfoConfig*>( firstUiItem ) )
        {
#ifdef USE_QTCHARTS
            menuBuilder << "RicCreateGridStatisticsPlotFeature";
#endif
            menuBuilder << "RicShowGridStatisticsFeature";
        }
        else if ( dynamic_cast<RimSimWellFracture*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSimWellFractureFeature";
        }
        else if ( dynamic_cast<RimValveTemplateCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewValveTemplateFeature";
            menuBuilder << "RicImportValveTemplatesFeature";
        }
        else if ( dynamic_cast<RimValveTemplate*>( firstUiItem ) )
        {
            menuBuilder << "RicDeleteValveTemplateFeature";
        }
        else if ( dynamic_cast<RimEnsembleFractureStatisticsCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportEnsembleFractureStatisticsFeature";
        }
        else if ( dynamic_cast<RimStimPlanModelTemplate*>( firstUiItem ) )
        {
            menuBuilder << "RicImportFaciesFeature";
            menuBuilder << "RicImportElasticPropertiesFeature";
        }
        else if ( dynamic_cast<RimStimPlanModelTemplateCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewStimPlanModelTemplateFeature";
        }
        else if ( dynamic_cast<RimFractureTemplateCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "RicNewThermalFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertAllFractureTemplatesToMetricFeature";
            menuBuilder << "RicConvertAllFractureTemplatesToFieldFeature";
        }
        else if ( dynamic_cast<RimStimPlanFractureTemplate*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "RicNewThermalFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if ( dynamic_cast<RimEllipseFractureTemplate*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "RicNewThermalFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if ( dynamic_cast<RimElasticProperties*>( firstUiItem ) )
        {
            menuBuilder << "RicNewElasticPropertyScalingFeature";
        }
        else if ( dynamic_cast<RimSurfaceCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportSurfacesFeature";
            menuBuilder << "RicNewGridSurfaceFeature";
            menuBuilder << "RicImportEnsembleSurfaceFeature";
            menuBuilder << "RicCreateEnsembleSurfaceFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSurfaceCollectionFeature";
        }
        else if ( dynamic_cast<RimSurface*>( firstUiItem ) )
        {
            if ( dynamic_cast<RimGridCaseSurface*>( firstUiItem ) )
            {
                menuBuilder << "RicExportKLayerToPtlFeature";
            }

            menuBuilder << "RicExportSurfaceToTsurfFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopySurfaceFeature";
            menuBuilder << "RicReloadSurfaceFeature";
        }
        else if ( dynamic_cast<RimCellFilterCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicPasteCellFiltersFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewPolygonFilterFeature";
            menuBuilder << "RicNewUserDefinedFilterFeature";
            menuBuilder << "RicNewUserDefinedIndexFilterFeature";
            menuBuilder << "RicNewCellIndexFilterFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewCellRangeFilterFeature";
            menuBuilder.subMenuStart( "Slice Filters" );
            menuBuilder << "RicNewRangeFilterSliceIFeature";
            menuBuilder << "RicNewRangeFilterSliceJFeature";
            menuBuilder << "RicNewRangeFilterSliceKFeature";
            menuBuilder.subMenuEnd();
        }
        else if ( dynamic_cast<RimSeismicSectionCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewInlineSeismicSectionFeature";
            menuBuilder << "RicNewXlineSeismicSectionFeature";
            menuBuilder << "RicNewZSliceSeismicSectionFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewPolylineSeismicSectionFeature";
            menuBuilder << "RicNewWellpathSeismicSectionFeature";
        }
        else if ( dynamic_cast<RimSeismicDataCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicImportSeismicFeature";
        }
        else if ( dynamic_cast<RimSeismicViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSeismicViewFeature";
        }
        else if ( dynamic_cast<RimAnnotationCollection*>( firstUiItem ) || dynamic_cast<RimAnnotationGroupCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateTextAnnotationFeature";
            menuBuilder << "RicCreateReachCircleAnnotationFeature";
            menuBuilder << "RicCreateUserDefinedPolylinesAnnotationFeature";
            menuBuilder << "RicImportPolylinesAnnotationFeature";
        }
        else if ( dynamic_cast<RimAnnotationInViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateTextAnnotationFeature";
        }
        else if ( dynamic_cast<RimPlotTemplateFolderItem*>( firstUiItem ) || dynamic_cast<RimPlotTemplateFileItem*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateNewPlotFromTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicRenamePlotTemplateFeature";
            menuBuilder << "RicDeletePlotTemplateFeature";
            menuBuilder << "RicOpenInTextEditorFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSetAsDefaultTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicReloadPlotTemplatesFeature";
        }
        else if ( dynamic_cast<RimSummaryMultiPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicNewDefaultSummaryPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSnapshotViewToPdfFeature";
            menuBuilder << "RicSaveMultiPlotTemplateFeature";
            menuBuilder << "RicPasteSummaryMultiPlotFeature";
        }
        else if ( dynamic_cast<RimMultiPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicSnapshotViewToPdfFeature";
        }
        else if ( dynamic_cast<RimStreamlineInViewCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicNewStreamlineFeature";
        }
        else if ( dynamic_cast<RimSummaryAddressCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicCreateMultiPlotFromSelectionFeature";
            menuBuilder << "RicCreatePlotFromTemplateByShortcutFeature";
            menuBuilder << "RicCreateCrossPlotFeature";
        }
        else if ( dynamic_cast<RimPlotAxisPropertiesInterface*>( firstUiItem ) )
        {
            if ( dynamic_cast<RimPlotAxisProperties*>( firstUiItem ) )
            {
                menuBuilder << "RicToggleYAxisLinkingFeature";
            }
            if ( dynamic_cast<RimSummaryTimeAxisProperties*>( firstUiItem ) )
            {
                menuBuilder << "RicToggleXAxisLinkingFeature";
            }
            menuBuilder << "RicNewPlotAxisPropertiesFeature";
        }
        else if ( dynamic_cast<RimRftCase*>( firstUiItem ) )
        {
            menuBuilder << "RicNewMultiPhaseRftSegmentPlotFeature";
            menuBuilder << "RicNewRftSegmentWellLogPlotFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewRftWellLogPlotFeature";
        }
        else if ( dynamic_cast<RimPressureDepthData*>( firstUiItem ) )
        {
            menuBuilder << "RicOpenInTextEditorFeature";
            menuBuilder << "RicReloadPressureDepthDataFeature";
        }
        else if ( dynamic_cast<RimEclipseResultAddress*>( firstUiItem ) )
        {
            menuBuilder << "RicAddGridCalculationFeature";
        }

        if ( dynamic_cast<Rim3dView*>( firstUiItem ) )
        {
            menuBuilder << "Separator";
            menuBuilder << "RicLinkVisibleViewsFeature";
            menuBuilder << "RicShowLinkOptionsFeature";
            menuBuilder << "RicSetMasterViewFeature";
            menuBuilder << "RicUnLinkViewFeature";
        }
    }

    if ( firstUiItem )
    {
        if ( auto pdmObject = dynamic_cast<caf::PdmUiObjectHandle*>( firstUiItem ) )
        {
            pdmObject->appendMenuItems( menuBuilder );
        }

        // Work in progress -- Start
        // All commands should be aware of selection of multiple objects
        // Based on the selection, the command feature can decide if the command
        // can be executed, communicated by isCommandEnabled(). When a command feature
        // is aware of multiple selected items, move the command to this list
        // without using dyncamic_cast.

        menuBuilder << "RicPasteTimeHistoryCurveFeature";
        menuBuilder << "RicPasteAsciiDataCurveFeature";
        menuBuilder << "RicPasteSummaryCaseFeature";
        menuBuilder.addSeparator();
        menuBuilder << "RicCopyReferencesToClipboardFeature";

        menuBuilder << "RicLinkViewFeature";

        menuBuilder << "RicShowPlotDataFeature";
        menuBuilder << "RicShowTotalAllocationDataFeature";

        menuBuilder << "RicNewSummaryMultiPlotFeature";
        menuBuilder << "RicOpenSummaryPlotEditorFeature";
        menuBuilder << "RicNewDerivedEnsembleFeature";
        menuBuilder << "RicSummaryCurveSwitchAxisFeature";
        menuBuilder << "RicNewDerivedSummaryFeature";
        menuBuilder.addSeparator();
        menuBuilder << "RicConvertGroupToEnsembleFeature";
        menuBuilder.addSeparator();
        menuBuilder.addSeparator();
        menuBuilder << "RicStackSelectedCurvesFeature";
        menuBuilder << "RicUnstackSelectedCurvesFeature";
        menuBuilder.addSeparator();

        menuBuilder << "RicFlyToObjectFeature";

        menuBuilder << "RicImportObservedDataFeature";
        menuBuilder << "RicImportObservedFmuDataFeature";
        menuBuilder << "RicImportPressureDepthDataFeature";
        menuBuilder << "RicRenameSummaryCaseFeature";
        menuBuilder << "RicReloadSummaryCaseFeature";
        menuBuilder << "RicReplaceSummaryCaseFeature";
        menuBuilder << "RicCreateSummaryCaseCollectionFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicCutReferencesToClipboardFeature";

        menuBuilder << "Separator";

        menuBuilder << "RicDeleteWellPathFeature";
        menuBuilder << "RicLinkWellPathFeature";

        if ( dynamic_cast<RimSummaryCase*>( firstUiItem ) || dynamic_cast<RimSummaryCaseCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
            menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
            menuBuilder << "RicCreateMultiPlotFromSelectionFeature";
            menuBuilder << "RicCreatePlotFromTemplateByShortcutFeature";
        }

        menuBuilder << "Separator";
        menuBuilder << "RicCloseSummaryCaseFeature";
        menuBuilder << "RicCloseSummaryCaseInCollectionFeature";
        menuBuilder << "RicDeleteSummaryCaseCollectionFeature";
        menuBuilder << "RicCloseObservedDataFeature";
        menuBuilder << "RicDeleteSubPlotFeature";

        menuBuilder << "RicNewMultiPlotFeature";
        menuBuilder << "RicAppendSummaryPlotsForObjectsFeature";
        menuBuilder << "RicAppendSummaryCurvesForObjectsFeature";

        // Work in progress -- End

        appendCreateCompletions( menuBuilder, menuBuilder.itemCount() > 0u );
        appendExportWellPaths( menuBuilder, menuBuilder.itemCount() > 0u );

        if ( menuBuilder.itemCount() > 0u )
        {
            menuBuilder.addSeparator();
        }

        if ( dynamic_cast<RimWellLogFileChannel*>( firstUiItem ) )
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if ( dynamic_cast<RimEclipseStatisticsCase*>( firstUiItem ) )
        {
            createExecuteScriptForCasesFeatureMenu( menuBuilder );
        }
        else if ( dynamic_cast<RimEclipseCase*>( firstUiItem ) )
        {
            menuBuilder << "RicShowGridCalculatorFeature";
            menuBuilder << "RicAddEclipseInputPropertyFeature";
            menuBuilder << "RicReloadCaseFeature";
            menuBuilder << "RicReplaceCaseFeature";
            createExecuteScriptForCasesFeatureMenu( menuBuilder );
            menuBuilder << "RicCloseSourSimDataFeature";
        }
        else if ( dynamic_cast<RimSummaryPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicAsciiExportSummaryPlotFeature";
        }
        else if ( dynamic_cast<RimSeismicData*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSeismicViewFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSeismicDifferenceFeature";
        }
        else if ( dynamic_cast<RimWellLogPlot*>( firstUiItem ) )
        {
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
            menuBuilder << "RicExportToLasFileFeature";
            menuBuilder << "RicChangeDataSourceFeature";
        }
        else if ( dynamic_cast<RimWellLogCurve*>( firstUiItem ) || dynamic_cast<RimWellLogTrack*>( firstUiItem ) )
        {
            menuBuilder << "RicNewWellLogCalculatedCurveFeature";
            menuBuilder << "RicExportToLasFileFeature";
            menuBuilder << "RicChangeDataSourceFeature";
        }
        else if ( dynamic_cast<RimWellLogPlotCollection*>( firstUiItem ) )
        {
            menuBuilder << "RicExportToLasFileFeature";
        }
        else if ( dynamic_cast<RimFaultInView*>( firstUiItem ) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if ( dynamic_cast<RimSimWellInView*>( firstUiItem ) )
        {
            menuBuilder << "RicShowContributingWellsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEclipseWellShowLabelFeature";
            menuBuilder << "RicEclipseWellShowHeadFeature";
            menuBuilder << "RicEclipseWellShowPipeFeature";
            menuBuilder << "RicEclipseWellShowSpheresFeature";
            menuBuilder << "RicEclipseWellShowWellCellsFeature";
            menuBuilder << "RicEclipseWellShowWellCellFenceFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSimWellFractureFeature";
            menuBuilder << "RicNewSimWellIntersectionFeature";
        }
        else if ( dynamic_cast<RimSummaryAddress*>( firstUiItem ) )
        {
            menuBuilder << "RicNewSummaryMultiPlotFromDataVectorFeature";
            menuBuilder << "RicAppendSummaryCurvesForSummaryAddressesFeature";
            menuBuilder << "RicAppendSummaryPlotsForSummaryAddressesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryTableFeature";
            menuBuilder << "RicCreateCrossPlotFeature";
        }
#ifdef USE_ODB_API
        else if ( dynamic_cast<RimWellIASettings*>( firstUiItem ) )
        {
            menuBuilder << "RicRunWellIntegrityAnalysisFeature";
        }
#endif
        menuBuilder.addSeparator();
        menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
    }

    {
        bool toggleCommandCandidate = true;

        if ( firstUiItem )
        {
            if ( dynamic_cast<RimEclipseCellColors*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
            else if ( dynamic_cast<RimCellEdgeColors*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
            else if ( dynamic_cast<RimEclipseFaultColors*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
            else if ( dynamic_cast<RimEclipseFaultColors*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
            else if ( dynamic_cast<RimVirtualPerforationResults*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
            else if ( dynamic_cast<RimGeoMechCellColors*>( firstUiItem ) )
            {
                toggleCommandCandidate = false;
            }
        }

        bool addSeparator = true;

        if ( toggleCommandCandidate && RicToggleItemsFeatureImpl::isToggleCommandsAvailable() )
        {
            menuBuilder << "Separator";
            menuBuilder << "RicToggleItemsOnFeature";
            menuBuilder << "RicToggleItemsOffFeature";
            menuBuilder << "RicToggleItemsFeature";

            addSeparator = false;
        }

        if ( addSeparator )
        {
            menuBuilder.addSeparator();
        }

        if ( dynamic_cast<RimRegularLegendConfig*>( firstUiItem ) == nullptr )
        {
            menuBuilder << "RicToggleItemsOnOthersOffFeature";
        }

        if ( RicToggleItemsFeatureImpl::isToggleCommandsAvailable() )
        {
            menuBuilder << "RicCollapseSiblingsFeature";
        }

        menuBuilder << "RicMoveItemsToTopFeature";
        menuBuilder << "RicSearchHelpFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteItemFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteItemFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteSubItemsFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteUncheckedSubItemsFeature";
        menuBuilder << "RicDeleteSubItemsFeature";
    }

    // Special delete commands for specific features
    // Placed here to fit context menu location of general delete feature

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "Ric3dWellLogCurveDeleteFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "Ric3dWellLogCurveDeleteFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicWellLogFileCloseFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicWellLogFileCloseFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicCloseCaseFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicCloseCaseFeature";
    }

    return menuBuilder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimContextCommandBuilder::allWellPaths()
{
    RimProject* proj = RimProject::current();
    return proj->allWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContextCommandBuilder::createExecuteScriptForCasesFeatureMenu( caf::CmdFeatureMenuBuilder& menuBuilder )
{
    // Execute script on selection of cases
    RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
    if ( ruiMainWindow )
    {
        std::vector<RimCase*> cases;
        ruiMainWindow->selectedCases( cases );

        if ( !cases.empty() )
        {
            menuBuilder.subMenuStart( "Execute script" );

            RiaApplication* app  = RiaApplication::instance();
            RimProject*     proj = app->project();
            if ( proj && proj->scriptCollection() )
            {
                RimScriptCollection* rootScriptCollection = proj->scriptCollection();

                // Root script collection holds a list of subdirectories of user defined script folders
                for ( size_t i = 0; i < rootScriptCollection->subDirectories.size(); i++ )
                {
                    RimScriptCollection* subDir = rootScriptCollection->subDirectories[i];

                    if ( subDir )
                    {
                        appendScriptItems( menuBuilder, subDir );
                    }
                }
            }

            menuBuilder.addSeparator();
            menuBuilder.subMenuEnd();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContextCommandBuilder::appendScriptItems( caf::CmdFeatureMenuBuilder& menuBuilder, RimScriptCollection* scriptCollection )
{
    QDir dir( scriptCollection->directory );
    menuBuilder.subMenuStart( dir.dirName() );

    for ( size_t i = 0; i < scriptCollection->calcScripts.size(); i++ )
    {
        RimCalcScript* calcScript = scriptCollection->calcScripts[i];
        QFileInfo      fi( calcScript->absoluteFileName() );

        QString menuText = fi.baseName();
        menuBuilder.addCmdFeatureWithUserData( "RicExecuteScriptForCasesFeature", menuText, QVariant( calcScript->absoluteFileName() ) );
    }

    for ( size_t i = 0; i < scriptCollection->subDirectories.size(); i++ )
    {
        RimScriptCollection* subDir = scriptCollection->subDirectories[i];

        appendScriptItems( menuBuilder, subDir );
    }

    menuBuilder.subMenuEnd();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendImportMenu( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu )
{
    QStringList candidates;
    candidates << "RicWellPathsImportFileFeature";
    candidates << "RicWellPathFormationsImportFileFeature";
    candidates << "RicWellLogsImportFileFeature";
    candidates << "RicImportWellLogCsvFileFeature";
    candidates << "RicReloadWellPathFormationNamesFeature";

    return appendSubMenuWithCommands( menuBuilder, candidates, "Import", QIcon(), addSeparatorBeforeMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendCreateCompletions( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu )
{
    QStringList candidates;
    candidates << "RicNewPerforationIntervalFeature";
    candidates << "RicEditPerforationCollectionFeature";
    candidates << "RicNewValveFeature";
    candidates << "RicNewFishbonesSubsFeature";
    candidates << "RicNewWellPathFractureFeature";
    candidates << "Separator";
    candidates << "RicCreateMultipleFracturesFeature";
    candidates << "RicNewWellPathAttributeFeature";
    candidates << "Separator";
    candidates << "RicCreateTemporaryLgrFeature";
    candidates << "RicNewStimPlanModelFeature";

    return appendSubMenuWithCommands( menuBuilder, candidates, "Create Completions", QIcon( ":/CompletionsSymbol16x16.png" ), addSeparatorBeforeMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendExportCompletions( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu )
{
    QStringList candidates;
    candidates << "RicExportCompletionsForVisibleWellPathsFeature";
    candidates << "RicWellPathExportCompletionDataFeature";
    candidates << "RicExportFishbonesLateralsFeature";
    candidates << "RicExportCompletionsWellSegmentsFeature";

    return appendSubMenuWithCommands( menuBuilder,
                                      candidates,
                                      "Export Completions",
                                      QIcon( ":/ExportCompletionsSymbol16x16.png" ),
                                      addSeparatorBeforeMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendExportWellPaths( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu )
{
    QStringList candidates;
    candidates << "RicExportSelectedWellPathsFeature";
    candidates << "RicExportVisibleWellPathsFeature";

    return appendSubMenuWithCommands( menuBuilder, candidates, "Export Well Paths", QIcon( ":/Save.svg" ), addSeparatorBeforeMenu );
}

//-------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendSubMenuWithCommands( caf::CmdFeatureMenuBuilder& menuBuilder,
                                                         const QStringList&          commandCandidates,
                                                         const QString&              menuLabel,
                                                         const QIcon&                menuIcon /*= QIcon()*/,
                                                         bool                        addSeparatorBeforeMenu /*=false*/ )
{
    int         actualCommandsAdded = 0;
    QStringList validCommands;
    for ( QString candidate : commandCandidates )
    {
        if ( candidate == "Separator" )
        {
            validCommands << candidate;
        }
        else
        {
            if ( caf::CmdFeatureManager::instance()->getCommandFeature( candidate.toStdString() )->canFeatureBeExecuted() &&
                 !menuBuilder.isCmdFeatureAdded( candidate ) )
            {
                validCommands << candidate;
                actualCommandsAdded++;
            }
        }
    }

    if ( actualCommandsAdded > 0 )
    {
        if ( addSeparatorBeforeMenu )
        {
            menuBuilder << "Separator";
        }
        menuBuilder.subMenuStart( menuLabel, menuIcon );

        for ( int i = 0; i < validCommands.size(); ++i )
        {
            bool firstOrLast = i == 0 || i == validCommands.size() - 1;
            if ( !firstOrLast || validCommands[i] != "Separator" )
            {
                menuBuilder << validCommands[i];
            }
        }

        menuBuilder.subMenuEnd();
    }
    return actualCommandsAdded;
}
