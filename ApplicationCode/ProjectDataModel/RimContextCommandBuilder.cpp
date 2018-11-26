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

#include "Rim3dOverlayInfoConfig.h"
#include "Rim3dWellLogCurveCollection.h"
#include "Rim3dWellLogExtractionCurve.h"
#include "Rim3dWellLogFileCurve.h"
#include "Rim3dWellLogRftCurve.h"
#include "RimAnnotationCollection.h"
#include "RimCalcScript.h"
#include "RimCaseCollection.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimContourMapViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimFaultInView.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowDiagSolution.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimGridCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"
#include "RimObservedData.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimScriptCollection.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
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
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "RimEllipseFractureTemplate.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimModeledWellPath.h"

#include "RiuMainWindow.h"

#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <vector>
#include <QMenu>
#include <QDir>
#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::CmdFeatureMenuBuilder RimContextCommandBuilder::commandsFromSelection()
{
    //QStringList commandIds;
    caf::CmdFeatureMenuBuilder menuBuilder;

    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems(uiItems);

    if (uiItems.size() == 1)
    {
        caf::PdmUiItem* uiItem = uiItems[0];
        CVF_ASSERT(uiItem);

        if (dynamic_cast<RimEclipseCaseCollection*>(uiItem))
        {
            menuBuilder.subMenuStart("Import");
            menuBuilder << "RicImportEclipseCaseFeature";
            menuBuilder << "RicImportEclipseCasesFeature";
            menuBuilder << "RicImportInputEclipseCaseFeature";
            menuBuilder << "RicCreateGridCaseGroupFeature";
            menuBuilder << "RicCreateGridCaseGroupFromFilesFeature";
            menuBuilder.subMenuEnd();
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if (dynamic_cast<RimGeoMechView*>(uiItem))
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";

            menuBuilder << "RicNewViewFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
        }
        else if (dynamic_cast<RimEclipseView*>(uiItem))
        {
            menuBuilder << "RicPasteEclipseViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicNewContourMapViewFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
        }
        else if (dynamic_cast<RimContourMapViewCollection*>(uiItem))
        {
            menuBuilder << "RicNewContourMapViewFeature";
        }
        else if (dynamic_cast<RimCaseCollection*>(uiItem))
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewStatisticsCaseFeature";
        }
        else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
        {
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "RicComputeStatisticsFeature";
            menuBuilder << "Separator";
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
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
        else if (dynamic_cast<RimGridInfoCollection*>(uiItem))
        {
            menuBuilder << "RicExportCompletionsForTemporaryLgrsFeature";
            menuBuilder << "RicDeleteTemporaryLgrsFeature";
        }
        else if (dynamic_cast<RimGeoMechCase*>(uiItem))
        {
            menuBuilder << "RicPasteGeoMechViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewViewFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicImportElementPropertyFeature";
            menuBuilder << "Separator";
        }
        else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem))
        {
            menuBuilder << "RicPasteEclipseCasesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEclipseCaseNewGroupFeature";
        }
        else if (dynamic_cast<RimEclipseCellColors*>(uiItem))
        {
            menuBuilder << "RicSaveEclipseResultAsInputPropertyFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
        }
        else if (dynamic_cast<RimEclipseInputPropertyCollection*>(uiItem))
        {
            menuBuilder << "RicAddEclipseInputPropertyFeature";
        }
        else if (dynamic_cast<RimEclipseInputProperty*>(uiItem))
        {
            menuBuilder << "RicSaveEclipseInputPropertyFeature";
        }
        else if (dynamic_cast<RimCellRangeFilterCollection*>(uiItem))
        {
            menuBuilder << "RicRangeFilterNewFeature";
            menuBuilder << "RicRangeFilterNewSliceIFeature";
            menuBuilder << "RicRangeFilterNewSliceJFeature";
            menuBuilder << "RicRangeFilterNewSliceKFeature";
        }
        else if (dynamic_cast<RimCellRangeFilter*>(uiItem))
        {
            menuBuilder << "RicRangeFilterInsertFeature";
            menuBuilder << "RicRangeFilterNewSliceIFeature";
            menuBuilder << "RicRangeFilterNewSliceJFeature";
            menuBuilder << "RicRangeFilterNewSliceKFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(uiItem))
        {
            menuBuilder << "RicEclipsePropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem))
        {
            menuBuilder << "RicEclipsePropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(uiItem))
        {
            menuBuilder << "RicGeoMechPropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem))
        {
            menuBuilder << "RicGeoMechPropertyFilterInsertFeature";
            menuBuilder << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if (dynamic_cast<RimWellPathCollection*>(uiItem))
        {
            menuBuilder.subMenuStart("Import");
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellPathsImportSsihubFeature";
            menuBuilder << "RicWellPathFormationsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";
            menuBuilder << "RicReloadWellPathFormationNamesFeature";
            menuBuilder << "RicWellPathImportPerforationIntervalsFeature";
            menuBuilder.subMenuEnd();
         
            menuBuilder.addSeparator();
         
            menuBuilder << "RicNewEditableWellPathFeature";
        }
        else if (dynamic_cast<RimWellPath*>(uiItem))
        {
            menuBuilder.subMenuStart("Import");
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellPathFormationsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";
            menuBuilder << "RicReloadWellPathFormationNamesFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            menuBuilder.subMenuStart("Well Plots", QIcon(":/WellLogTrack16x16.png"));
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            menuBuilder.subMenuStart("3D Well Log Curves", QIcon(":/WellLogCurve16x16.png"));
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
            menuBuilder.subMenuEnd();

            menuBuilder << "RicNewEditableWellPathFeature";
            menuBuilder << "RicNewWellPathIntersectionFeature";

            menuBuilder.addSeparator();
            menuBuilder.subMenuStart("Completions", QIcon(":/CompletionsSymbol16x16.png"));
            menuBuilder << "RicNewWellPathFractureFeature";
            menuBuilder << "RicNewFishbonesSubsFeature";
            menuBuilder << "RicNewPerforationIntervalFeature";
            menuBuilder << "RicNewValveFeature";
            menuBuilder << "RicEditPerforationCollectionFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.subMenuStart("Export Completions", QIcon(":/ExportCompletionsSymbol16x16.png"));
            menuBuilder << "RicExportCompletionsForVisibleWellPathsFeature";
            menuBuilder << "RicWellPathExportCompletionDataFeature";
            menuBuilder.subMenuEnd();

            if ( dynamic_cast<RimModeledWellPath*>(uiItem) )
            {
                menuBuilder << "RicShowWellPlanFeature";
            }
            menuBuilder << "RicCreateMultipleFracturesFeature"; 
            menuBuilder << "RicNewWellPathAttributeFeature";

            menuBuilder << "Separator";

        }
        else if (dynamic_cast<RimWellPathAttributeCollection*>(uiItem))
        {
            menuBuilder << "RicDeleteWellPathAttributeFeature";
        }
        else if (dynamic_cast<Rim3dWellLogCurveCollection*>(uiItem) ||
                 dynamic_cast<Rim3dWellLogExtractionCurve*>(uiItem) ||
                 dynamic_cast<Rim3dWellLogFileCurve*>(uiItem) ||
                 dynamic_cast<Rim3dWellLogRftCurve*>(uiItem))
        {
            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";
            menuBuilder << "RicAdd3dWellLogRftCurveFeature";
        }
        else if (dynamic_cast<RimWellLogFile*>(uiItem))
        {
            menuBuilder << "RicWellPathsImportFileFeature";
            menuBuilder << "RicWellLogsImportFileFeature";

            menuBuilder << "Separator";

            menuBuilder.subMenuStart("Move LAS file to well path");

            RimWellPath* parentWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>();
            QString parentWellPathName = parentWellPath ? parentWellPath->name() : "";

            for (RimWellPath* wellPath : allWellPaths())
            {
                if (wellPath->name() != parentWellPathName)
                {
                    menuBuilder.addCmdFeatureWithUserData("RicMoveWellLogFilesFeature", wellPath->name(), wellPath->name());
                }
            }
            menuBuilder.subMenuEnd();
        }
        else if (dynamic_cast<RimWellRftPlot*>(uiItem))
        {
            menuBuilder << "RicDeleteRftPlotFeature";
        }
        else if (dynamic_cast<RimWellPltPlot*>(uiItem))
        {
            menuBuilder << "RicDeletePltPlotFeature";
        }
        else if (dynamic_cast<RimCalcScript*>(uiItem))
        {
            menuBuilder << "RicEditScriptFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewScriptFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicExecuteScriptFeature";
        }
        else if (dynamic_cast<RimScriptCollection*>(uiItem))
        {
            menuBuilder << "RicNewScriptFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicAddScriptPathFeature";
            menuBuilder << "RicRefreshScriptsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteScriptPathFeature";
        }
        else if (dynamic_cast<RimViewController*>(uiItem))
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
        }
        else if (dynamic_cast<RimViewLinker*>(uiItem))
        {
            menuBuilder << "RicShowAllLinkedViewsFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteAllLinkedViewsFeature";
        }
        else if (dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";
        }
        else if (dynamic_cast<RimRftPlotCollection*>(uiItem))
        {
            menuBuilder << "RicNewRftPlotFeature";
        }
        else if (dynamic_cast<RimPltPlotCollection*>(uiItem))
        {
            menuBuilder << "RicNewPltPlotFeature";
        }
        else if (dynamic_cast<RimSummaryPlotCollection*>(uiItem))
        {
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
        }
        else if (dynamic_cast<RimSummaryCrossPlotCollection*>(uiItem))
        {
            menuBuilder << "RicPasteSummaryCrossPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            menuBuilder << "RicPasteWellLogPlotFeature";
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogPlotTrackFeature";
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
        }
        else if (dynamic_cast<RimWellLogTrack*>(uiItem))
        {
            menuBuilder << "RicPasteWellLogTrackFeature";
            menuBuilder << "RicPasteWellLogCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicDeleteWellLogPlotTrackFeature";
        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem))
        {
            menuBuilder << "RicPasteWellLogCurveFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))  // This is also the definition for RimSummaryCrossPlot
        {
            RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>(uiItem);

            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";
            menuBuilder << "RicPasteSummaryPlotFeature";
            menuBuilder << "RicPasteAsciiDataToSummaryPlotFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicDuplicateSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "RicDuplicateSummaryCrossPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicShowSummaryCurveCalculatorFeature";
            menuBuilder << "Separator";

            // Export is not supported for cross plot
            if (!summaryCrossPlot) menuBuilder << "RicAsciiExportSummaryPlotFeature";
            
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicViewZoomAllFeature";
        }
        else if (dynamic_cast<RimSummaryCurve*>(uiItem))
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicDuplicateSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
            menuBuilder << "RicDuplicateSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryCurveCalculationFeature";
        }
        else if (dynamic_cast<RimSummaryCurveCollection*>(uiItem))
        {
            menuBuilder << "RicPasteSummaryCurveFeature";
            menuBuilder << "RicPasteSummaryCrossPlotCurveFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
            menuBuilder << "RicNewSummaryCrossPlotCurveFeature";
        }
        else if (dynamic_cast<RimEnsembleCurveSetCollection*>(uiItem))
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
            menuBuilder << "RicPasteEnsembleCurveSetFeature";
        }
        else if (dynamic_cast<RimEnsembleCurveSet*>(uiItem))
        {
            menuBuilder << "RicNewSummaryEnsembleCurveSetFeature";
        }
        else if (dynamic_cast<RimEnsembleCurveFilterCollection*>(uiItem))
        {
            menuBuilder << "RicNewEnsembleCurveFilterFeature";
        }
        else if (dynamic_cast<RimSummaryCaseMainCollection*>(uiItem))
        {
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder << "RicNewDerivedEnsembleFeature";
        }
        else if (dynamic_cast<RimSummaryCaseCollection*>(uiItem))
        {
            menuBuilder.subMenuStart("Import");
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewDerivedEnsembleFeature";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicConvertGroupToEnsembleFeature";
            menuBuilder.addSeparator();
        }
        else if (dynamic_cast<RimSummaryCase*>(uiItem))
        {
            menuBuilder.subMenuStart("Import");
            menuBuilder << "RicImportSummaryCaseFeature";
            menuBuilder << "RicImportSummaryCasesFeature";
            menuBuilder << "RicImportSummaryGroupFeature";
            menuBuilder << "RicImportEnsembleFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCrossPlotFeature";
            menuBuilder.addSeparator();

            if (!dynamic_cast<RimObservedData*>(uiItem))
            {
                menuBuilder << "RicShowSummaryCurveCalculatorFeature";
                //menuBuilder << "RicNewSummaryPlotFeature";
            }
        }
        else if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if (dynamic_cast<RimIntersectionCollection*>(uiItem))
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if (dynamic_cast<RimIntersection*>(uiItem))
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
        else if (dynamic_cast<RimIntersectionBox*>(uiItem))
        {
            menuBuilder << "RicPasteIntersectionsFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicAppendIntersectionFeature";
            menuBuilder << "RicAppendIntersectionBoxFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
        }
        else if (dynamic_cast<RimSimWellInView*>(uiItem))
        {
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder << "RicNewWellLogRftCurveFeature";
            menuBuilder << "RicNewSimWellIntersectionFeature";

            menuBuilder.subMenuStart("Well Plots", QIcon(":/WellLogTrack16x16.png"));
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicPlotProductionRateFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder.subMenuEnd();

            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if (dynamic_cast<RimSimWellInViewCollection*>(uiItem))
        {
            menuBuilder << "RicExportCompletionsForVisibleSimWellsFeature";
        }
        else if(dynamic_cast<RimFormationNames*>(uiItem))
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if(dynamic_cast<RimFormationNamesCollection*>(uiItem))
        {
            menuBuilder << "RicImportFormationNamesFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFaultInView*>(uiItem) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if (dynamic_cast<RimWellAllocationPlot*>(uiItem))
        {
            menuBuilder << "RicAddStoredWellAllocationPlotFeature";
        }
        else if (dynamic_cast<RimFlowCharacteristicsPlot*>(uiItem))
        {
            menuBuilder << "RicAddStoredFlowCharacteristicsPlotFeature";
        }
        else if (dynamic_cast<RimFlowDiagSolution*>(uiItem))
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if (dynamic_cast<RimFlowPlotCollection*>(uiItem))
        {
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if (dynamic_cast<Rim3dOverlayInfoConfig*>(uiItem))
        {
            menuBuilder << "RicShowGridStatisticsFeature";
        }
        else if (dynamic_cast<RimSimWellFracture*>(uiItem))
        {
            menuBuilder << "RicNewSimWellFractureFeature";
        }
        else if (dynamic_cast<RimFractureTemplateCollection*>(uiItem))
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
        else if (dynamic_cast<RimStimPlanFractureTemplate*>(uiItem))
        {
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if (dynamic_cast<RimEllipseFractureTemplate*>(uiItem))
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder << "RicPasteStimPlanFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if (dynamic_cast<RimAnnotationCollection*>(uiItem))
        {
            menuBuilder << "RicImportPolylinesAnnotationFeature";
            menuBuilder << "RicNewTextAnnotationFeature";
            menuBuilder << "RicNewReachCircleAnnotationFeature";
            menuBuilder << "RicNewPolygonAnnotationFeature";
        }

        if (dynamic_cast<Rim3dView*>(uiItem))
        {
            menuBuilder << "Separator";
            menuBuilder << "RicLinkVisibleViewsFeature";
            menuBuilder << "RicShowLinkOptionsFeature";
            menuBuilder << "RicSetMasterViewFeature";
            menuBuilder << "RicUnLinkViewFeature";
        }
    }

    if (uiItems.size() > 0)
    {
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
        
        menuBuilder << "RicNewDerivedEnsembleFeature";
        menuBuilder << "RicNewSummaryPlotFeature";
        menuBuilder << "RicNewSummaryCrossPlotFeature";
        menuBuilder << "RicSummaryCurveSwitchAxisFeature";
        menuBuilder.addSeparator();
        menuBuilder << "RicConvertGroupToEnsembleFeature";
        menuBuilder.addSeparator();

        if (!menuBuilder.isCmdFeatureAdded("RicNewFishbonesSubsFeature"))
        {
            menuBuilder << "RicNewFishbonesSubsFeature";
        }
        if (!menuBuilder.isCmdFeatureAdded("RicNewPerforationIntervalFeature"))
        {
            menuBuilder << "RicNewPerforationIntervalFeature";
        }
        if (!menuBuilder.isCmdFeatureAdded("RicNewValveFeature"))
        {
            menuBuilder << "RicNewValveFeature";
        }

        menuBuilder << "RicEditPerforationCollectionFeature";
        menuBuilder << "RicExportFishbonesLateralsFeature";
        menuBuilder << "RicExportFishbonesWellSegmentsFeature";
        menuBuilder << "RicExportFracturesWellSegmentsFeature";
        {
            QStringList candidates;

            if (!menuBuilder.isCmdFeatureAdded("RicExportCompletionsForVisibleWellPathsFeature"))
            {
                candidates << "RicExportCompletionsForVisibleWellPathsFeature";
            }
            if (!menuBuilder.isCmdFeatureAdded("RicWellPathExportCompletionDataFeature"))
            {
                candidates << "RicWellPathExportCompletionDataFeature";
            }

            if (!candidates.isEmpty())
            {
                menuBuilder.subMenuStart("Export Completions", QIcon(":/ExportCompletionsSymbol16x16.png"));

                for (const auto& text : candidates)
                {
                    menuBuilder << text;
                }

                menuBuilder.subMenuEnd();
            }
        }

        {
            QStringList candidates;

            if (!menuBuilder.isCmdFeatureAdded("RicExportSelectedWellPathsFeature"))
            {
                candidates << "RicExportSelectedWellPathsFeature";

            }
            if (!menuBuilder.isCmdFeatureAdded("RicExportVisibleWellPathsFeature"))
            {
                candidates << "RicExportVisibleWellPathsFeature";
            }

            if (!candidates.isEmpty())
            {
                menuBuilder.subMenuStart("Export Well Paths", QIcon(":/Save.png"));

                for (const auto& text : candidates)
                {
                    menuBuilder << text;
                }

                menuBuilder.subMenuEnd();
            }
        }

        menuBuilder << "RicCreateMultipleFracturesFeature";
        menuBuilder << "RicWellPathImportCompletionsFileFeature";
        menuBuilder << "RicFlyToObjectFeature";

        menuBuilder << "RicImportObservedDataFeature";
        menuBuilder << "RicReloadSummaryCaseFeature";
        menuBuilder << "RicCreateSummaryCaseCollectionFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicCutReferencesToClipboardFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicCloseSummaryCaseFeature";
        menuBuilder << "RicCloseSummaryCaseInCollectionFeature";
        menuBuilder << "RicDeleteSummaryCaseCollectionFeature";
        menuBuilder << "RicCloseObservedDataFeature";

        menuBuilder << "RicCreateTemporaryLgrFeature";

        // Work in progress -- End

        caf::PdmUiItem* uiItem = uiItems[0];
        if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            menuBuilder << "RicAddWellLogToPlotFeature";
        }
        else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
        {
            createExecuteScriptForCasesFeatureMenu(menuBuilder);
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            menuBuilder << "RicReloadCaseFeature";
            createExecuteScriptForCasesFeatureMenu(menuBuilder);
            menuBuilder << "RicCloseSourSimDataFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))
        {
            RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>(uiItem);
            if (!summaryCrossPlot)
            {
                menuBuilder << "RicAsciiExportSummaryPlotFeature";
            }
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            menuBuilder << "RicAsciiExportWellLogPlotFeature";
        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem) ||
                 dynamic_cast<RimWellLogTrack*>(uiItem) ||
                 dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            menuBuilder << "RicExportToLasFileFeature";
            menuBuilder << "RicChangeDataSourceFeature";
        }
        else if (dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            menuBuilder << "RicExportToLasFileFeature";
        }
        else if (dynamic_cast<RimFaultInView*>(uiItem) )
        {
            menuBuilder << "RicExportFaultsFeature";
        }
        else if (dynamic_cast<RimSimWellInView*>(uiItem))
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

        if (RicToggleItemsFeatureImpl::isToggleCommandsAvailable())
        {
            menuBuilder << "Separator";
            menuBuilder << "RicToggleItemsOnFeature";
            menuBuilder << "RicToggleItemsOffFeature";
            menuBuilder << "RicToggleItemsFeature";
            addSeparator = false;
        }

        if (addSeparator)
        {
            menuBuilder.addSeparator();
        }
        menuBuilder << "RicToggleItemsOnOthersOffFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteItemFeature")->canFeatureBeExecuted() )
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteItemFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteSubItemsFeature")->canFeatureBeExecuted())
    {
        menuBuilder << "Separator";
        menuBuilder << "RicDeleteSubItemsFeature";
    }

    // Special delete commands for specific features
    // Placed here to fit context menu location of general delete feature
    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicWellPathDeleteFeature")->canFeatureBeExecuted())
    {
        menuBuilder << "Separator";
        menuBuilder << "RicWellPathDeleteFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("Ric3dWellLogCurveDeleteFeature")->canFeatureBeExecuted())
    {
        menuBuilder << "Separator";
        menuBuilder << "Ric3dWellLogCurveDeleteFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicWellLogFileCloseFeature")->canFeatureBeExecuted())
    {
        menuBuilder << "Separator";
        menuBuilder << "RicWellLogFileCloseFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature("RicCloseCaseFeature")->canFeatureBeExecuted() )
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
void RimContextCommandBuilder::createExecuteScriptForCasesFeatureMenu(caf::CmdFeatureMenuBuilder& menuBuilder)
{
    // Execute script on selection of cases
    RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
    if (ruiMainWindow)
    {
        std::vector<RimCase*> cases;
        ruiMainWindow->selectedCases(cases);

        if (cases.size() > 0)
        {
            menuBuilder.subMenuStart("Execute script");

            RiaApplication* app = RiaApplication::instance();
            RimProject* proj = app->project();
            if (proj && proj->scriptCollection())
            {
                RimScriptCollection* rootScriptCollection = proj->scriptCollection();

                // Root script collection holds a list of subdirectories of user defined script folders
                for (size_t i = 0; i < rootScriptCollection->subDirectories.size(); i++)
                {
                    RimScriptCollection* subDir = rootScriptCollection->subDirectories[i];

                    if (subDir)
                    {
                        appendScriptItems(menuBuilder, subDir);
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
void RimContextCommandBuilder::appendScriptItems(caf::CmdFeatureMenuBuilder& menuBuilder, RimScriptCollection* scriptCollection)
{
    QDir dir(scriptCollection->directory);
    menuBuilder.subMenuStart(dir.dirName());

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    CVF_ASSERT(commandManager);

    RicExecuteScriptForCasesFeature* executeScriptFeature = dynamic_cast<RicExecuteScriptForCasesFeature*>(commandManager->getCommandFeature("RicExecuteScriptForCasesFeature"));
    CVF_ASSERT(executeScriptFeature);

    for (size_t i = 0; i < scriptCollection->calcScripts.size(); i++)
    {
        RimCalcScript* calcScript = scriptCollection->calcScripts[i];
        QFileInfo fi(calcScript->absolutePath());

        QString menuText = fi.baseName();
        menuBuilder.addCmdFeatureWithUserData("RicExecuteScriptForCasesFeature", menuText, QVariant(calcScript->absolutePath()));
    }

    for (size_t i = 0; i < scriptCollection->subDirectories.size(); i++)
    {
        RimScriptCollection* subDir = scriptCollection->subDirectories[i];

        appendScriptItems(menuBuilder, subDir);
    }

    menuBuilder.subMenuEnd();
}
