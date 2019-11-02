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
#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimCalcScript.h"
#include "RimCaseCollection.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimFaultInView.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowDiagSolution.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimGridCollection.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimGridPlotWindowCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"
#include "RimModeledWellPath.h"
#include "RimObservedSummaryData.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimScriptCollection.h"
#include "RimSimWellFracture.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "RiuMainWindow.h"

#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"
#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"
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
    // QStringList commandIds;
    caf::CmdFeatureMenuBuilder menuBuilder;

    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    if ( uiItems.size() == 1 )
    {
        caf::PdmUiItem* uiItem = uiItems[0];
        CVF_ASSERT( uiItem );

        if ( dynamic_cast<RimEclipseCaseCollection*>( uiItem ) )
        {
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportEclipseCaseFeature";
            menuBuilder << "RicImportEclipseCasesFeature";
            menuBuilder << "RicImportInputEclipseCaseFeature";
            menuBuilder << "RicCreateGridCaseGroupFeature";
            menuBuilder << "RicCreateGridCaseGroupFromFilesFeature";
            menuBuilder.subMenuEnd();
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if ( dynamic_cast<RimGeoMechView*>( uiItem ) )
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";

            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";

            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "RicExportContourMapToTextFeature";
        }
        else if ( dynamic_cast<RimEclipseView*>( uiItem ) )
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
            menuBuilder << "RicExportContourMapToTextFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
        }
        else if ( dynamic_cast<RimEclipseContourMapViewCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewContourMapViewFeature";
        }
        else if ( dynamic_cast<RimGeoMechContourMapViewCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewContourMapViewFeature";
        }
        else if ( dynamic_cast<RimCaseCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewStatisticsCaseFeature";
        }
        else if ( dynamic_cast<RimEclipseStatisticsCase*>( uiItem ) )
        {
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicComputeStatisticsFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimEclipseCase*>( uiItem ) )
        {
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
        else if ( dynamic_cast<RimGridInfoCollection*>( uiItem ) )
        {
            menuBuilder << "RicExportCompletionsForTemporaryLgrsFeature";
            menuBuilder << "RicDeleteTemporaryLgrsFeature";
        }
        else if ( dynamic_cast<RimGeoMechCase*>( uiItem ) )
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicImportElementPropertyFeature";
            menuBuilder << "Separator";
        }
        else if ( dynamic_cast<RimIdenticalGridCaseGroup*>( uiItem ) )
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if ( dynamic_cast<RimEclipseCellColors*>( uiItem ) )
        {
            menuBuilder << "RicSaveEclipseResultAsInputPropertyFeature";
            menuBuilder << "RicExportEclipseInputGridFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
            menuBuilder << "RicCreateGridCrossPlotFeature";
        }
        else if ( dynamic_cast<RimEclipseInputPropertyCollection*>( uiItem ) )
        {
            menuBuilder << "RicAddEclipseInputPropertyFeature";
        }
        else if ( dynamic_cast<RimEclipseInputProperty*>( uiItem ) )
        {
            menuBuilder << "RicSaveEclipseInputPropertyFeature";
        }
        else if ( dynamic_cast<RimCellRangeFilterCollection*>( uiItem ) )
        {
            menuBuilder << "RicRangeFilterNewFeature";
            menuBuilder << "RicRangeFilterNewSliceIFeature";
            menuBuilder << "RicRangeFilterNewSliceJFeature";
            menuBuilder << "RicRangeFilterNewSliceKFeature";
        }
        else if ( dynamic_cast<RimCellRangeFilter*>( uiItem ) )
        {
            menuBuilder << "RicRangeFilterInsertFeature";
            menuBuilder << "RicRangeFilterNewSliceIFeature";
            menuBuilder << "RicRangeFilterNewSliceJFeature";
            menuBuilder << "RicRangeFilterNewSliceKFeature";
        }
        else if ( dynamic_cast<RimEclipsePropertyFilterCollection*>( uiItem ) )
        {
            menuBuilder << "RicEclipsePropertyFilterNewFeature";
        }
        else if ( dynamic_cast<RimEclipsePropertyFilter*>( uiItem ) )
        {
            menuBuilder << "RicEclipsePropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if ( dynamic_cast<RimGeoMechPropertyFilterCollection*>( uiItem ) )
        {
            menuBuilder << "RicGeoMechPropertyFilterNewFeature";
        }
        else if ( dynamic_cast<RimGeoMechPropertyFilter*>( uiItem ) )
        {
            menuBuilder << "RicGeoMechPropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if ( dynamic_cast<RimWellPathCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewEditableWellPathFeature";
            menuBuilder.addSeparator();
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellPathsImportSsihubFeature";
            menuBuilder << "RicWellPathFormationsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";
            menuBuilder << "RicReloadWellPathFormationNamesFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicWellPathImportPerforationIntervalsFeature";
            menuBuilder << "RicWellPathImportCompletionsFileFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder.subMenuStart( "Export Well Paths", QIcon( ":/Save.png" ) );
            menuBuilder << "RicExportSelectedWellPathsFeature";
            menuBuilder << "RicExportVisibleWellPathsFeature";
            menuBuilder.subMenuEnd();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellPath*>( uiItem ) )
        {
            menuBuilder << "RicNewEditableWellPathFeature";
            menuBuilder << "RicNewWellPathIntersectionFeature";
            appendCreateCompletions( menuBuilder );
            menuBuilder.addSeparator();
            appendImportMenu( menuBuilder );
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
            menuBuilder.addSeparator();
            appendExportWellPaths( menuBuilder );
            menuBuilder.addSeparator();

            menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.subMenuStart( "3D Well Log Curves", QIcon( ":/WellLogCurve16x16.png" ) );
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();

            menuBuilder.addSeparator();

            if ( dynamic_cast<RimModeledWellPath*>( uiItem ) )
            {
                menuBuilder << "RicShowWellPlanFeature";
            }
        }
        else if ( dynamic_cast<RimWellPathCompletions*>( uiItem ) )
        {
            menuBuilder.subMenuStart( "Create Completions", QIcon( ":/CompletionsSymbol16x16.png" ) );
            menuBuilder << "RicNewPerforationIntervalFeature";
            menuBuilder << "RicNewFishbonesSubsFeature";
            menuBuilder << "RicNewWellPathFractureFeature";
            menuBuilder.subMenuEnd();
            menuBuilder << "RicCreateTemporaryLgrFeature";
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimPerforationCollection*>( uiItem ) || dynamic_cast<RimPerforationInterval*>( uiItem ) )
        {
            menuBuilder << "RicNewPerforationIntervalFeature";
            if ( dynamic_cast<RimPerforationInterval*>( uiItem ) ) menuBuilder << "RicNewValveFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicEditPerforationCollectionFeature";
            menuBuilder.addSeparator();
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimFishbonesCollection*>( uiItem ) || dynamic_cast<RimFishbonesMultipleSubs*>( uiItem ) ||
                  dynamic_cast<RimFishboneWellPathCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewFishbonesSubsFeature";
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellPathFractureCollection*>( uiItem ) || dynamic_cast<RimWellPathFracture*>( uiItem ) )
        {
            menuBuilder << "RicNewWellPathFractureFeature";
            appendExportCompletions( menuBuilder );
        }
        else if ( dynamic_cast<RimWellPathAttributeCollection*>( uiItem ) )
        {
            menuBuilder << "RicDeleteWellPathAttributeFeature";
        }
        else if ( dynamic_cast<Rim3dWellLogCurveCollection*>( uiItem ) ||
                  dynamic_cast<Rim3dWellLogExtractionCurve*>( uiItem ) ||
                  dynamic_cast<Rim3dWellLogFileCurve*>( uiItem ) || dynamic_cast<Rim3dWellLogRftCurve*>( uiItem ) )
        {
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
        }
        else if ( dynamic_cast<RimWellLogFile*>( uiItem ) )
        {
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";

            menuBuilder << "Separator";

            menuBuilder.subMenuStart( "Move LAS file to well path" );

            RimWellPath* parentWellPath     = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>();
            QString      parentWellPathName = parentWellPath ? parentWellPath->name() : "";

            for ( RimWellPath* wellPath : allWellPaths() )
            {
                if ( wellPath->name() != parentWellPathName )
                {
                    menuBuilder.addCmdFeatureWithUserData( "RicMoveWellLogFilesFeature",
                                                           wellPath->name(),
                                                           wellPath->name() );
                }
            }
            menuBuilder.subMenuEnd();
        }
        else if ( dynamic_cast<RimWellRftPlot*>( uiItem ) )
        {
            menuBuilder << "RicDeleteRftPlotFeature";
        }
        else if ( dynamic_cast<RimWellPltPlot*>( uiItem ) )
        {
            menuBuilder << "RicDeletePltPlotFeature";
        }
        else if ( dynamic_cast<RimCalcScript*>( uiItem ) )
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
        else if ( dynamic_cast<RimScriptCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewOctaveScriptFeature";
#ifdef ENABLE_GRPC
            menuBuilder << "RicNewPythonScriptFeature";
#endif
            menuBuilder << "Separator";
            menuBuilder << "RicAddScriptPathFeature";
            menuBuilder << "RicRefreshScriptsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteScriptPathFeature";
        }
        else if ( dynamic_cast<RimViewController*>( uiItem ) )
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
        }
        else if ( dynamic_cast<RimViewLinker*>( uiItem ) || dynamic_cast<RimViewLinkerCollection*>( uiItem ) )
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteAllLinkedViewsFeature";
        }
        else if ( dynamic_cast<RimWellLogPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
        }
        else if ( dynamic_cast<RimRftPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewRftPlotFeature";
        }
        else if ( dynamic_cast<RimPltPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewPltPlotFeature";
        }
        else if ( dynamic_cast<RimSummaryPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewDefaultSummaryPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
        }
        else if ( dynamic_cast<RimSummaryCrossPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteSummaryCrossPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
        }
        else if ( dynamic_cast<RimWellLogPlot*>( uiItem ) )
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotTrackFeature";
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
        }
        else if ( dynamic_cast<RimWellLogTrack*>( uiItem ) )
        {
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "RicPasteWellLogCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteSubPlotFeature";
        }
        else if ( dynamic_cast<RimWellLogCurve*>( uiItem ) )
        {
            menuBuilder << "RicPasteWellLogCurveFeature";
        }
        else if ( dynamic_cast<RimGridCrossPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicCreateGridCrossPlotFeature";
        }
        else if ( dynamic_cast<RimSaturationPressurePlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicCreateSaturationPressurePlotsFeature";
        }
        else if ( dynamic_cast<RimGridCrossPlot*>( uiItem ) )
        {
            menuBuilder << "RicPasteGridCrossPlotDataSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCreateGridCrossPlotDataSetFeature";
            menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
        }
        else if ( dynamic_cast<RimGridCrossPlotDataSet*>( uiItem ) )
        {
            menuBuilder << "RicPasteGridCrossPlotDataSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
        }
        else if ( dynamic_cast<RimSummaryPlot*>( uiItem ) ) // This is also the definition for RimSummaryCrossPlot
        {
            RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>( uiItem );

            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewDefaultSummaryPlotFeature";
            menuBuilder << "RicDuplicateSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "RicDuplicateSummaryCrossPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSavePlotTemplateFeature";

            // Export is not supported for cross plot
            if ( !summaryCrossPlot ) menuBuilder << "RicAsciiExportSummaryPlotFeature";

            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicViewZoomAllFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteSubPlotFeature";
        }
        else if ( dynamic_cast<RimSummaryCurve*>( uiItem ) )
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";

            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicDuplicateSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "RicDuplicateSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSetSourceSteppingSummaryCurveFeature";
            menuBuilder << "RicClearSourceSteppingSummaryCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryCurveCalculationFeature";
        }
        else if ( dynamic_cast<RimSummaryCurveCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveSetCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveSet*>( uiItem ) )
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicSetSourceSteppingEnsembleCurveSetFeature";
            menuBuilder << "RicClearSourceSteppingEnsembleCurveSetFeature";
        }
        else if ( dynamic_cast<RimEnsembleCurveFilterCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewEnsembleCurveFilterFeature";
        }
        else if ( dynamic_cast<RimSummaryCaseMainCollection*>( uiItem ) )
        {
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder << "RicNewDerivedEnsembleFeature";
        }
        else if ( dynamic_cast<RimSummaryCaseCollection*>( uiItem ) )
        {
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewDerivedEnsembleFeature";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewDefaultSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicConvertGroupToEnsembleFeature";
            menuBuilder.addSeparator();
        }
        else if ( dynamic_cast<RimSummaryCase*>( uiItem ) )
        {
            menuBuilder.subMenuStart( "Import" );
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewDefaultSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
            menuBuilder.addSeparator();

            if ( !dynamic_cast<RimObservedSummaryData*>( uiItem ) )
            {
                menuBuilder << "RicShowSummaryCurveCalculatorFeature";
                // menuBuilder << "RicNewSummaryPlotFeature";
            }
        }
        else if ( dynamic_cast<RimWellLogFileChannel*>( uiItem ) )
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if ( dynamic_cast<RimIntersectionCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimIntersection*>( uiItem ) )
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewIntersectionViewFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimIntersectionBox*>( uiItem ) )
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if ( dynamic_cast<RimSimWellInView*>( uiItem ) )
        {
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewSimWellIntersectionFeature";

            menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicPlotProductionRateFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder.subMenuEnd();

            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if ( dynamic_cast<RimSimWellInViewCollection*>( uiItem ) )
        {
            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if ( dynamic_cast<RimFormationNames*>( uiItem ) )
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFormationNamesCollection*>( uiItem ) )
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFaultInView*>( uiItem ) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if ( dynamic_cast<RimWellAllocationPlot*>( uiItem ) )
        {
            menuBuilder << "RicAddStoredWellAllocationPlotFeature";
        }
        else if ( dynamic_cast<RimFlowCharacteristicsPlot*>( uiItem ) )
        {
            menuBuilder << "RicAddStoredFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<RimFlowDiagSolution*>( uiItem ) )
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<RimFlowPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if ( dynamic_cast<Rim3dOverlayInfoConfig*>( uiItem ) )
        {
            menuBuilder << "RicShowGridStatisticsFeature";
        }
        else if ( dynamic_cast<RimSimWellFracture*>( uiItem ) )
        {
            menuBuilder << "RicNewSimWellFractureFeature";
        }
        else if ( dynamic_cast<RimValveTemplateCollection*>( uiItem ) )
        {
            menuBuilder << "RicNewValveTemplateFeature";
        }
        else if ( dynamic_cast<RimValveTemplate*>( uiItem ) )
        {
            menuBuilder << "RicDeleteValveTemplateFeature";
        }
        else if ( dynamic_cast<RimFractureTemplateCollection*>( uiItem ) )
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertAllFractureTemplatesToMetricFeature";
            menuBuilder << "RicConvertAllFractureTemplatesToFieldFeature";
        }
        else if ( dynamic_cast<RimStimPlanFractureTemplate*>( uiItem ) )
        {
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if ( dynamic_cast<RimEllipseFractureTemplate*>( uiItem ) )
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if ( dynamic_cast<RimAnnotationCollection*>( uiItem ) ||
                  dynamic_cast<RimAnnotationGroupCollection*>( uiItem ) )
        {
            menuBuilder << "RicCreateTextAnnotationFeature";
            menuBuilder << "RicCreateReachCircleAnnotationFeature";
            menuBuilder << "RicCreateUserDefinedPolylinesAnnotationFeature";
            menuBuilder << "RicImportPolylinesAnnotationFeature";
        }
        else if ( dynamic_cast<RimAnnotationInViewCollection*>( uiItem ) )
        {
            menuBuilder << "RicCreateTextAnnotationFeature";
        }
        else if ( dynamic_cast<RimPlotTemplateFolderItem*>( uiItem ) || dynamic_cast<RimPlotTemplateFileItem*>( uiItem ) )
        {
            menuBuilder << "RicReloadPlotTemplatesFeature";
        }
        if ( dynamic_cast<Rim3dView*>( uiItem ) )
        {
            menuBuilder << "Separator";
            menuBuilder << "RicLinkVisibleViewsFeature";
            menuBuilder << "RicShowLinkOptionsFeature";
            menuBuilder << "RicSetMasterViewFeature";
            menuBuilder << "RicUnLinkViewFeature";
        }
    }

    if ( uiItems.size() > 0 )
    {
        // Work in progress -- Start
        // All commands should be aware of selection of multiple objects
        // Based on the selection, the command feature can decide if the command
        // can be executed, communicated by isCommandEnabled(). When a command feature
        // is aware of multiple selected items, move the command to this list
        // without using dyncamic_cast.

        caf::PdmUiItem* uiItem = uiItems[0];

        menuBuilder << "RicPasteTimeHistoryCurveFeature";
        menuBuilder << "RicPasteAsciiDataCurveFeature";
        menuBuilder << "RicPasteSummaryCaseFeature";
        menuBuilder.addSeparator();
        menuBuilder << "RicCopyReferencesToClipboardFeature";

        menuBuilder << "RicLinkViewFeature";

        menuBuilder << "RicShowPlotDataFeature";
        menuBuilder << "RicShowTotalAllocationDataFeature";

        menuBuilder << "RicNewDerivedEnsembleFeature";
        menuBuilder << "RicNewSummaryPlotFeature";
        menuBuilder << "RicNewDefaultSummaryPlotFeature";
        menuBuilder << "RicNewSummaryCrossPlotFeature";
        menuBuilder << "RicSummaryCurveSwitchAxisFeature";
        menuBuilder.addSeparator();
        menuBuilder << "RicConvertGroupToEnsembleFeature";
        menuBuilder.addSeparator();

        menuBuilder << "RicFlyToObjectFeature";

        menuBuilder << "RicImportObservedDataFeature";
        menuBuilder << "RicImportObservedFmuDataFeature";
        menuBuilder << "RicReloadSummaryCaseFeature";
        menuBuilder << "RicCreateSummaryCaseCollectionFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicCutReferencesToClipboardFeature";

        menuBuilder << "Separator";
        if ( dynamic_cast<RimSummaryCase*>( uiItem ) || dynamic_cast<RimSummaryCaseCollection*>( uiItem ) )
        {
            menuBuilder << "RicCreatePlotFromSelectionFeature";
            menuBuilder << "RicCreatePlotFromTemplateByShortcutFeature";
        }

        menuBuilder << "Separator";
        menuBuilder << "RicCloseSummaryCaseFeature";
        menuBuilder << "RicCloseSummaryCaseInCollectionFeature";
        menuBuilder << "RicDeleteSummaryCaseCollectionFeature";
        menuBuilder << "RicCloseObservedDataFeature";

        menuBuilder << "RicNewGridPlotWindowFeature";

        // Work in progress -- End
        appendCreateCompletions( menuBuilder, menuBuilder.itemCount() > 0u );
        bool addedExportWellPaths = appendExportWellPaths( menuBuilder, menuBuilder.itemCount() > 0u ) > 0;
        appendExportCompletions( menuBuilder, menuBuilder.itemCount() > 0u && !addedExportWellPaths );

        if ( menuBuilder.itemCount() > 0u )
        {
            menuBuilder.addSeparator();
        }

        if ( dynamic_cast<RimWellLogFileChannel*>( uiItem ) )
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if ( dynamic_cast<RimEclipseStatisticsCase*>( uiItem ) )
        {
            createExecuteScriptForCasesFeatureMenu( menuBuilder );
        }
        else if ( dynamic_cast<RimEclipseCase*>( uiItem ) )
        {
            menuBuilder << "RicReloadCaseFeature";
            createExecuteScriptForCasesFeatureMenu( menuBuilder );
            menuBuilder << "RicCloseSourSimDataFeature";
        }
        else if ( dynamic_cast<RimSummaryPlot*>( uiItem ) )
        {
            RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>( uiItem );
            if ( !summaryCrossPlot )
            {
                menuBuilder << "RicAsciiExportSummaryPlotFeature";
            }
        }
        else if ( dynamic_cast<RimWellLogPlot*>( uiItem ) )
        {
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
            menuBuilder << "RicExportToLasFileFeature";
            menuBuilder << "RicChangeDataSourceFeature";
        }
        else if ( dynamic_cast<RimWellLogCurve*>( uiItem ) || dynamic_cast<RimWellLogTrack*>( uiItem ) )
        {
            menuBuilder << "RicExportToLasFileFeature";
            menuBuilder << "RicChangeDataSourceFeature";
        }
        else if ( dynamic_cast<RimWellLogPlotCollection*>( uiItem ) )
        {
            menuBuilder << "RicExportToLasFileFeature";
        }
        else if ( dynamic_cast<RimFaultInView*>( uiItem ) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if ( dynamic_cast<RimSimWellInView*>( uiItem ) )
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
        }
        menuBuilder.addSeparator();
        menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
    }

    {
        bool addSeparator = true;

        if ( RicToggleItemsFeatureImpl::isToggleCommandsAvailable() )
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
        menuBuilder << "RicToggleItemsOnOthersOffFeature";

        if ( RicToggleItemsFeatureImpl::isToggleCommandsAvailable() )
        {
            menuBuilder << "RicCollapseSiblingsFeature";
        }
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteItemFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteItemFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicDeleteSubItemsFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteSubItemsFeature";
    }

    // Special delete commands for specific features
    // Placed here to fit context menu location of general delete feature
    if ( caf::CmdFeatureManager::instance()->getCommandFeature( "RicWellPathDeleteFeature" )->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicWellPathDeleteFeature";
    }

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
    RimProject* proj = RiaApplication::instance()->project();
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

        if ( cases.size() > 0 )
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
void RimContextCommandBuilder::appendScriptItems( caf::CmdFeatureMenuBuilder& menuBuilder,
                                                  RimScriptCollection*        scriptCollection )
{
    QDir dir( scriptCollection->directory );
    menuBuilder.subMenuStart( dir.dirName() );

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    CVF_ASSERT( commandManager );

    RicExecuteScriptForCasesFeature* executeScriptFeature = dynamic_cast<RicExecuteScriptForCasesFeature*>(
        commandManager->getCommandFeature( "RicExecuteScriptForCasesFeature" ) );
    CVF_ASSERT( executeScriptFeature );

    for ( size_t i = 0; i < scriptCollection->calcScripts.size(); i++ )
    {
        RimCalcScript* calcScript = scriptCollection->calcScripts[i];
        QFileInfo      fi( calcScript->absoluteFileName() );

        QString menuText = fi.baseName();
        menuBuilder.addCmdFeatureWithUserData( "RicExecuteScriptForCasesFeature",
                                               menuText,
                                               QVariant( calcScript->absoluteFileName() ) );
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
    candidates << "RicReloadWellPathFormationNamesFeature";
    candidates << "Separator";
    candidates << "RicWellPathImportCompletionsFileFeature";

    return appendSubMenuWithCommands( menuBuilder, candidates, "Import", QIcon(), addSeparatorBeforeMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendCreateCompletions( caf::CmdFeatureMenuBuilder& menuBuilder,
                                                       bool                        addSeparatorBeforeMenu )
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

    return appendSubMenuWithCommands( menuBuilder,
                                      candidates,
                                      "Create Completions",
                                      QIcon( ":/CompletionsSymbol16x16.png" ),
                                      addSeparatorBeforeMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimContextCommandBuilder::appendExportCompletions( caf::CmdFeatureMenuBuilder& menuBuilder,
                                                       bool                        addSeparatorBeforeMenu )
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

    return appendSubMenuWithCommands( menuBuilder,
                                      candidates,
                                      "Export Well Paths",
                                      QIcon( ":/Save.png" ),
                                      addSeparatorBeforeMenu );
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
