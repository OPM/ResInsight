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
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimFault.h"
#include "RimFlowDiagSolution.h"
#include "RimFlowPlotCollection.h"
#include "RimFlowCharacteristicsPlot.h"
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
#include "RimScriptCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimEllipseFractureTemplate.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <vector>
#include <QMenu>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimContextCommandBuilder::commandsFromSelection()
{
    QStringList commandIds;

    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems(uiItems);

    if (uiItems.size() == 1)
    {
        caf::PdmUiItem* uiItem = uiItems[0];
        CVF_ASSERT(uiItem);

        if (dynamic_cast<RimEclipseCaseCollection*>(uiItem))
        {
            commandIds << "RicImportEclipseCaseFeature";
            commandIds << "RicImportInputEclipseCaseFeature";
            commandIds << "RicCreateGridCaseGroupFeature";
            commandIds << "RicEclipseCaseNewGroupFeature";
        }
        else if (dynamic_cast<RimGeoMechView*>(uiItem))
        {
            commandIds << "RicPasteGeoMechViewsFeature";
            commandIds << "Separator";

            commandIds << "RicNewViewFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
        }
        else if (dynamic_cast<RimEclipseView*>(uiItem))
        {
            commandIds << "RicPasteEclipseViewsFeature";
            commandIds << "Separator";
            commandIds << "RicNewViewFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
        }
        else if (dynamic_cast<RimCaseCollection*>(uiItem))
        {
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "Separator";
            commandIds << "RicNewStatisticsCaseFeature";
        }
        else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
        {
            commandIds << "RicNewViewFeature";
            commandIds << "RicComputeStatisticsFeature";
            commandIds << "Separator";
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "RicPasteEclipseViewsFeature";
            commandIds << "Separator";

            commandIds << "RicNewViewFeature";
            commandIds << "RicShowFlowCharacteristicsPlotFeature";
            commandIds << "RicEclipseCaseNewGroupFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "Separator";
        }
        else if (dynamic_cast<RimGeoMechCase*>(uiItem))
        {
            commandIds << "RicPasteGeoMechViewsFeature";
            commandIds << "Separator";
            commandIds << "RicNewViewFeature";
            commandIds << "Separator";
        }
        else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem))
        {
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "Separator";
            commandIds << "RicEclipseCaseNewGroupFeature";
        }
        else if (dynamic_cast<RimEclipseCellColors*>(uiItem))
        {
            commandIds << "RicSaveEclipseResultAsInputPropertyFeature";
        }
        else if (dynamic_cast<RimEclipseInputPropertyCollection*>(uiItem))
        {
            commandIds << "RicAddEclipseInputPropertyFeature";
        }
        else if (dynamic_cast<RimEclipseInputProperty*>(uiItem))
        {
            commandIds << "RicSaveEclipseInputPropertyFeature";
        }
        else if (dynamic_cast<RimCellRangeFilterCollection*>(uiItem))
        {
            commandIds << "RicRangeFilterNewFeature";
            commandIds << "RicRangeFilterNewSliceIFeature";
            commandIds << "RicRangeFilterNewSliceJFeature";
            commandIds << "RicRangeFilterNewSliceKFeature";
        }
        else if (dynamic_cast<RimCellRangeFilter*>(uiItem))
        {
            commandIds << "RicRangeFilterInsertFeature";
            commandIds << "RicRangeFilterNewSliceIFeature";
            commandIds << "RicRangeFilterNewSliceJFeature";
            commandIds << "RicRangeFilterNewSliceKFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterInsertFeature";
            commandIds << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterInsertFeature";
            commandIds << "RicApplyPropertyFilterAsCellResultFeature";
        }
        else if (dynamic_cast<RimWellPathCollection*>(uiItem))
        {
            commandIds << "RicWellPathsImportFileFeature";
            commandIds << "RicWellPathsImportSsihubFeature";
            commandIds << "RicWellLogsImportFileFeature";
            commandIds << "Separator";
        }
        else if (dynamic_cast<RimWellPath*>(uiItem))
        {
            commandIds << "RicNewWellLogFileCurveFeature";
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewWellPathIntersectionFeature";
        }
        else if (dynamic_cast<RimCalcScript*>(uiItem))
        {
            commandIds << "RicEditScriptFeature";
            commandIds << "Separator";
            commandIds << "RicNewScriptFeature";
            commandIds << "Separator";
            commandIds << "RicExecuteScriptFeature";
        }
        else if (dynamic_cast<RimScriptCollection*>(uiItem))
        {
            commandIds << "RicNewScriptFeature";
            commandIds << "Separator";
            commandIds << "RicAddScriptPathFeature";
            commandIds << "RicRefreshScriptsFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteScriptPathFeature";
        }
        else if (dynamic_cast<RimViewController*>(uiItem))
        {
            commandIds << "RicShowAllLinkedViewsFeature";
        }
        else if (dynamic_cast<RimViewLinker*>(uiItem))
        {
            commandIds << "RicShowAllLinkedViewsFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteAllLinkedViewsFeature";
        }
        else if (dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            commandIds << "RicPasteWellLogPlotFeature";
            commandIds << "Separator";
            commandIds << "RicNewWellLogPlotFeature";
        }
        else if (dynamic_cast<RimSummaryPlotCollection*>(uiItem))
        {
            commandIds << "RicPasteSummaryPlotFeature";
            commandIds << "RicPasteAsciiDataToSummaryPlotFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryPlotFeature";
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            commandIds << "RicPasteWellLogPlotFeature";
            commandIds << "RicPasteWellLogTrackFeature";
            commandIds << "Separator";
            commandIds << "RicNewWellLogPlotTrackFeature";
            commandIds << "RicAsciiExportWellLogPlotFeature";
        }
        else if (dynamic_cast<RimWellLogTrack*>(uiItem))
        {
            commandIds << "RicPasteWellLogTrackFeature";
            commandIds << "RicPasteWellLogCurveFeature";
            commandIds << "Separator";
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewWellLogFileCurveFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteWellLogPlotTrackFeature";
        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem))
        {
            commandIds << "RicPasteWellLogCurveFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFeature";
            commandIds << "RicPasteSummaryCurveFilterFeature";
            commandIds << "RicPasteSummaryPlotFeature";
            commandIds << "RicPasteAsciiDataToSummaryPlotFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryPlotFeature";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "RicAsciiExportSummaryPlotFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "Separator";
            commandIds << "RicViewZoomAllFeature";
        }
        else if (dynamic_cast<RimSummaryCurve*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
        }
        else if(dynamic_cast<RimSummaryCurveFilter*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFilterFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicCopyReferencesToClipboardFeature";
        }
        else if (dynamic_cast<RimSummaryCase*>(uiItem))
        {
            commandIds << "RicNewSummaryPlotFeature";
        }
        else if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            commandIds << "RicAddWellLogToPlotFeature";
        }
        else if (dynamic_cast<RimIntersectionCollection*>(uiItem))
        {
            commandIds << "RicAppendIntersectionFeature";
            commandIds << "RicAppendIntersectionBoxFeature";
        }
        else if (dynamic_cast<RimIntersection*>(uiItem))
        {
            commandIds << "RicAppendIntersectionFeature";
            commandIds << "RicAppendIntersectionBoxFeature";
        }
        else if (dynamic_cast<RimIntersectionBox*>(uiItem))
        {
            commandIds << "RicAppendIntersectionFeature";
            commandIds << "RicAppendIntersectionBoxFeature";
        }
        else if (dynamic_cast<RimEclipseWell*>(uiItem))
        {
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewSimWellIntersectionFeature";
            commandIds << "RicShowWellAllocationPlotFeature";
        }
        else if(dynamic_cast<RimFormationNames*>(uiItem))
        {
            commandIds << "RicImportFormationNamesFeature";
            commandIds << "RicReloadFormationNamesFeature";
        }
        else if(dynamic_cast<RimFormationNamesCollection*>(uiItem))
        {
            commandIds << "RicImportFormationNamesFeature";
            commandIds << "Separator";
            commandIds << "RicReloadFormationNamesFeature";
        }
        else if ( dynamic_cast<RimFault*>(uiItem) )
        {
            commandIds << "RicExportFaultsFeature";
        }
        else if (dynamic_cast<RimWellAllocationPlot*>(uiItem))
        {
            commandIds << "RicAddStoredWellAllocationPlotFeature";
        }
        else if (dynamic_cast<RimFlowCharacteristicsPlot*>(uiItem))
        {
            commandIds << "RicAddStoredFlowCharacteristicsPlotFeature";
        }
        else if (dynamic_cast<RimFlowDiagSolution*>(uiItem))
        {
            commandIds << "RicShowFlowCharacteristicsPlotFeature";
        }
        else if (dynamic_cast<RimFlowPlotCollection*>(uiItem))
        {
            commandIds << "RicShowFlowCharacteristicsPlotFeature";
        }
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
        else if (dynamic_cast<RimSimWellFracture*>(uiItem))
        {
            commandIds << "RicNewSimWellFractureFeature";
        }
        else if (dynamic_cast<RimFractureTemplateCollection*>(uiItem) ||
            dynamic_cast<RimStimPlanFractureTemplate*>(uiItem))
        {
            commandIds << "RicNewEllipseFractureTemplateFeature";
            commandIds << "RicNewStimPlanFractureTemplateFeature";
            commandIds << "Separator";
            commandIds << "RicConvertAllFractureTemplatesToMetricFeature";
            commandIds << "RicConvertAllFractureTemplatesToFieldFeature";
        }
        else if (dynamic_cast<RimEllipseFractureTemplate*>(uiItem))
        {
            commandIds << "RicNewEllipseFractureTemplateFeature";
            commandIds << "RicNewStimPlanFractureTemplateFeature";
            commandIds << "Separator";
            commandIds << "RicConvertFractureTemplateUnitFeature";
        }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES


        if (dynamic_cast<RimView*>(uiItem))
        {
            commandIds << "Separator";
            commandIds << "RicLinkVisibleViewsFeature";
            commandIds << "RicLinkViewFeature";
            commandIds << "RicShowLinkOptionsFeature";
            commandIds << "RicSetMasterViewFeature";
            commandIds << "RicUnLinkViewFeature";
        }
    }

    // Command supporting multiple selected objects
    if (uiItems.size() > 0)
    {
        // Work in progress -- Start
        // All commands should be aware of selection of multiple objects
        // Based on the selection, the command feature can decide if the command
        // can be executed, communicated by isCommandEnabled(). When a command feature
        // is aware of multiple selected items, move the command to this list
        // without using dyncamic_cast.

        commandIds << "RicPasteTimeHistoryCurveFeature";
        commandIds << "RicCopyReferencesToClipboardFeature";
        
        commandIds << "RicShowPlotDataFeature";
        commandIds << "RicShowTotalAllocationDataFeature";
        
        commandIds << "RicSummaryCurveSwitchAxisFeature";
        
        commandIds << "RicNewFishbonesSubsFeature";
        commandIds << "RicNewPerforationIntervalFeature";
        commandIds << "RicEditPerforationCollectionFeature";
        commandIds << "RicExportFishbonesLateralsFeature";
        commandIds << "RicExportFishbonesWellSegmentsFeature";
        commandIds << "RicWellPathImportPerforationIntervalsFeature";
        commandIds << "RicWellPathExportCompletionDataFeature";
        commandIds << "RicWellPathImportCompletionsFileFeature";
        commandIds << "RicFlyToObjectFeature";
        commandIds << "RicExportCarfin";

        // Fracture commands
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
        commandIds << "RicNewWellPathFractureFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

        // Work in progress -- End

        caf::PdmUiItem* uiItem = uiItems[0];
        if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            commandIds << "RicAddWellLogToPlotFeature";
        }
        else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
        {
            commandIds << "RicExecuteScriptForCasesFeature";
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            commandIds << "RicReloadCaseFeature";
            commandIds << "RicExecuteScriptForCasesFeature";
            commandIds << "RicCloseSourSimDataFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))
        {
            commandIds << "RicAsciiExportSummaryPlotFeature";
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            commandIds << "RicAsciiExportWellLogPlotFeature";
        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem) ||
                 dynamic_cast<RimWellLogTrack*>(uiItem) ||
                 dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            commandIds << "RicExportToLasFileFeature";
            commandIds << "RicChangeDataSourceFeature";
        }
        else if (dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            commandIds << "RicExportToLasFileFeature";
        }
        else if (dynamic_cast<RimFault*>(uiItem) )
        {
            commandIds << "RicExportFaultsFeature";
        }
        else if (dynamic_cast<RimEclipseWell*>(uiItem))
        {
            commandIds << "RicPlotProductionRateFeature";
            commandIds << "RicShowContributingWellsFeature";
            commandIds << "Separator";
            commandIds << "RicEclipseWellShowLabelFeature";
            commandIds << "RicEclipseWellShowHeadFeature";
            commandIds << "RicEclipseWellShowPipeFeature";
            commandIds << "RicEclipseWellShowSpheresFeature";
            commandIds << "RicEclipseWellShowWellCellsFeature";
            commandIds << "RicEclipseWellShowWellCellFenceFeature";
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
            commandIds << "Separator";
            commandIds << "RicNewSimWellFractureFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
        }
    }


    if (RicToggleItemsFeatureImpl::isToggleCommandsAvailable())
    {
        commandIds << "Separator";
        commandIds << "RicToggleItemsOnFeature";
        commandIds << "RicToggleItemsOffFeature";
        commandIds << "RicToggleItemsFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteItemFeature")->canFeatureBeExecuted() )
    {
        commandIds << "Separator";
        commandIds << "RicDeleteItemFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicDeleteSubItemsFeature")->canFeatureBeExecuted())
    {
        commandIds << "Separator";
        commandIds << "RicDeleteSubItemsFeature";
    }

    if (caf::CmdFeatureManager::instance()->getCommandFeature("RicWellPathDeleteFeature")->canFeatureBeExecuted())
    {
        // Special delete command for Well paths
        // Placed here to fit context menu location of general delete feature
        commandIds << "Separator";
        commandIds << "RicWellPathDeleteFeature";
    }

    if ( caf::CmdFeatureManager::instance()->getCommandFeature("RicCloseCaseFeature")->canFeatureBeExecuted() )
    {
        commandIds << "Separator";
        commandIds << "RicCloseCaseFeature";
    }

    return commandIds;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimContextCommandBuilder::appendCommandsToMenu(const QStringList& commandIds, QMenu* menu)
{
    CVF_ASSERT(menu);

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    for (int i = 0; i < commandIds.size(); i++)
    {
        if (commandIds[i] == "Separator")
        {
            menu->addSeparator();
        }
        else
        {
            caf::CmdFeature* feature = commandManager->getCommandFeature(commandIds[i].toStdString());
            CVF_ASSERT(feature);

            if (feature->canFeatureBeExecuted())
            {
                QAction* act = commandManager->action(commandIds[i]);
                CVF_ASSERT(act);

                for (QAction* existingAct : menu->actions())
                {
                    // If action exist, continue to make sure the action is positioned at the first
                    // location of a command ID
                    if (existingAct == act) continue;
                }

                menu->addAction(act);
            }
        }
    }
}

