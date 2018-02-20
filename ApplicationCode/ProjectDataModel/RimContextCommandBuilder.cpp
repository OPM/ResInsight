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
#include "RimCalcScript.h"
#include "RimCaseCollection.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
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
#include "RimWellPathCollection.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimEllipseFractureTemplate.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

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
            menuBuilder << "RicImportEclipseCaseFeature";
            menuBuilder << "RicImportEclipseCasesFeature";
            menuBuilder << "RicImportInputEclipseCaseFeature";
            menuBuilder << "RicCreateGridCaseGroupFeature";
            menuBuilder << "RicCreateGridCaseGroupFromFilesFeature";
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
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "RicSaveEclipseInputVisibleCellsFeature";
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
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
            menuBuilder << "RicEclipseCaseNewGroupFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicCopyReferencesToClipboardFeature";
            menuBuilder << "Separator";
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

            menuBuilder.subMenuStart("Well Plots", QIcon(":/SummaryPlot16x16.png"));
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";
            menuBuilder << "RicNewWellLogCurveExtractionFeature";
            menuBuilder << "RicNewWellPathIntersectionFeature";
            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            menuBuilder.subMenuStart("Completions", QIcon(":/FishBoneGroup16x16.png"));
            // Fracture commands
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
            menuBuilder << "RicNewWellPathFractureFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
            menuBuilder << "RicNewFishbonesSubsFeature";
            menuBuilder << "RicNewPerforationIntervalFeature";
            menuBuilder << "RicEditPerforationCollectionFeature";
            menuBuilder.subMenuEnd();

            menuBuilder << "Separator";
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
            menuBuilder << "Separator";
            menuBuilder << "RicEditSummaryPlotFeature";
            menuBuilder << "RicNewSummaryPlotFeature";
            menuBuilder << "RicDuplicateSummaryPlotFeature";
            menuBuilder << "RicNewSummaryCurveFeature";
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
        else if (dynamic_cast<RimSummaryCase*>(uiItem))
        {
            if (!dynamic_cast<RimObservedData*>(uiItem))
            {
                menuBuilder << "RicShowSummaryCurveCalculatorFeature";
                menuBuilder << "RicNewSummaryPlotFeature";
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

            menuBuilder.subMenuStart("Well Plots", QIcon(":/SummaryPlot16x16.png"));
            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicPlotProductionRateFeature";
            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder.subMenuEnd();
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
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
        else if (dynamic_cast<RimSimWellFracture*>(uiItem))
        {
            menuBuilder << "RicNewSimWellFractureFeature";
        }
        else if (dynamic_cast<RimFractureTemplateCollection*>(uiItem))
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertAllFractureTemplatesToMetricFeature";
            menuBuilder << "RicConvertAllFractureTemplatesToFieldFeature";
        }
        else if (dynamic_cast<RimStimPlanFractureTemplate*>(uiItem))
        {
            //menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
        else if (dynamic_cast<RimEllipseFractureTemplate*>(uiItem))
        {
            menuBuilder << "RicPasteEllipseFractureFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewEllipseFractureTemplateFeature";
            menuBuilder << "RicNewStimPlanFractureTemplateFeature";
            menuBuilder << "Separator";
            menuBuilder << "RicConvertFractureTemplateUnitFeature";
        }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES


        if (dynamic_cast<Rim3dView*>(uiItem))
        {
            menuBuilder << "Separator";
            menuBuilder << "RicLinkVisibleViewsFeature";
            menuBuilder << "RicLinkViewFeature";
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
        
        menuBuilder << "RicShowPlotDataFeature";
        menuBuilder << "RicShowTotalAllocationDataFeature";
        
        menuBuilder << "RicSummaryCurveSwitchAxisFeature";
        
        if (!menuBuilder.isCmdFeatureAdded("RicNewFishbonesSubsFeature"))
        {
            menuBuilder << "RicNewFishbonesSubsFeature";
        }
        if (!menuBuilder.isCmdFeatureAdded("RicNewPerforationIntervalFeature"))
        {
            menuBuilder << "RicNewPerforationIntervalFeature";
        }

        menuBuilder << "RicEditPerforationCollectionFeature";
        menuBuilder << "RicExportFishbonesLateralsFeature";
        menuBuilder << "RicExportFishbonesWellSegmentsFeature";
        menuBuilder << "RicWellPathExportCompletionDataFeature";
        menuBuilder << "RicWellPathImportCompletionsFileFeature";
        menuBuilder << "RicFlyToObjectFeature";
        menuBuilder << "RicExportCarfin";

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
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
            menuBuilder << "Separator";
            menuBuilder << "RicNewSimWellFractureFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
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

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicWellPathDeleteFeature")->canFeatureBeExecuted())
    {
        // Special delete command for Well paths
        // Placed here to fit context menu location of general delete feature
        menuBuilder << "Separator";
        menuBuilder << "RicWellPathDeleteFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicWellLogFileCloseFeature")->canFeatureBeExecuted())
    {
        // Special delete command for Well paths
        // Placed here to fit context menu location of general delete feature
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
