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
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
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
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersectionBox.h"
#include "RimScriptCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include "cafCmdFeatureManager.h"
#include "cafCmdFeature.h"

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

    if (uiItems.size() == 0)
    {
        commandIds << "RicNewWellLogPlotFeature";
        commandIds << "RicNewSummaryPlotFeature";
    }
    else if (uiItems.size() == 1)
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
            commandIds << "RicCopyReferencesToClipboardFeature";
            //commandIds << "RicDeleteItemFeature";
            //commandIds << "Separator";
        }
        else if (dynamic_cast<RimEclipseView*>(uiItem))
        {
            commandIds << "RicPasteEclipseViewsFeature";
            commandIds << "Separator";
            commandIds << "RicNewViewFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            //commandIds << "RicDeleteItemFeature";
            //commandIds << "Separator";
        }
        else if (dynamic_cast<RimCaseCollection*>(uiItem))
        {
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "RicNewStatisticsCaseFeature";
        }
        else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
        {
            commandIds << "RicNewViewFeature";
            commandIds << "RicComputeStatisticsFeature";
            commandIds << "Separator";

            //commandIds << "RicCloseCaseFeature";
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "RicPasteEclipseViewsFeature";
            commandIds << "Separator";

            commandIds << "RicNewViewFeature";
            commandIds << "RicEclipseCaseNewGroupFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "Separator";

            //commandIds << "RicCloseCaseFeature";
        }
        else if (dynamic_cast<RimGeoMechCase*>(uiItem))
        {
            commandIds << "RicPasteGeoMechViewsFeature";
            commandIds << "RicNewViewFeature";
            commandIds << "Separator";

            //commandIds << "RicCloseCaseFeature";
        }
        else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem))
        {
            commandIds << "RicEclipseCaseNewGroupFeature";
            commandIds << "RicPasteEclipseCasesFeature";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimEclipseCellColors*>(uiItem))
        {
            commandIds << "RicSaveEclipseResultAsInputPropertyFeature";
        }
        else if (dynamic_cast<RimEclipseInputPropertyCollection*>(uiItem))
        {
            commandIds << "RicAddEclipseInputPropertyFeature";
            commandIds << "RicAddOpmInputPropertyFeature";
        }
        else if (dynamic_cast<RimEclipseInputProperty*>(uiItem))
        {
            //commandIds << "RicDeleteItemFeature";
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
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterInsertFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterInsertFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimWellPathCollection*>(uiItem))
        {
            commandIds << "RicWellPathsImportFileFeature";
            commandIds << "RicWellPathsImportSsihubFeature";
            commandIds << "RicWellLogsImportFileFeature";
            commandIds << "Separator";
            commandIds << "RicWellPathsDeleteAllFeature";
        }
        else if (dynamic_cast<RimWellPath*>(uiItem))
        {
            commandIds << "RicNewWellLogFileCurveFeature";
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewWellPathIntersectionFeature";
            commandIds << "Separator";
            commandIds << "RicWellPathDeleteFeature";
        }
        else if (dynamic_cast<RimCalcScript*>(uiItem))
        {
            commandIds << "RicEditScriptFeature";
            commandIds << "RicNewScriptFeature";
            commandIds << "Separator";
            commandIds << "RicExecuteScriptFeature";
        }
        else if (dynamic_cast<RimScriptCollection*>(uiItem))
        {
            commandIds << "RicNewScriptFeature";
            commandIds << "RicAddScriptPathFeature";
            commandIds << "RicRefreshScriptsFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteScriptPathFeature";
        }
        else if (dynamic_cast<RimViewController*>(uiItem))
        {
            commandIds << "RicShowAllLinkedViewsFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimViewLinker*>(uiItem))
        {
            commandIds << "RicShowAllLinkedViewsFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteAllLinkedViewsFeature";
        }
        else if (dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            commandIds << "RicNewWellLogPlotFeature";
        }
        else if (dynamic_cast<RimSummaryPlotCollection*>(uiItem))
        {
            commandIds << "RicNewSummaryPlotFeature";
            commandIds << "RicPasteSummaryPlotFeature";
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            commandIds << "RicNewWellLogPlotTrackFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimWellLogTrack*>(uiItem))
        {
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewWellLogFileCurveFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteWellLogPlotTrackFeature";
        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem))
        {
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFeature";
            commandIds << "RicPasteSummaryCurveFilterFeature";
            commandIds << "RicPasteSummaryPlotFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryPlotFeature";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            //commandIds << "RicDeleteItemFeature";
            commandIds << "Separator";
            commandIds << "RicViewZoomAllFeature";
        }
        else if (dynamic_cast<RimSummaryCurve*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            //commandIds << "RicDeleteItemFeature";
            commandIds << "Separator";
            commandIds << "RicSummaryCurveSwitchAxisFeature";
        }
        else if(dynamic_cast<RimSummaryCurveFilter*>(uiItem))
        {
            commandIds << "RicPasteSummaryCurveFilterFeature";
            commandIds << "Separator";
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            //commandIds << "RicDeleteItemFeature";
            commandIds << "Separator";
            commandIds << "RicSummaryCurveSwitchAxisFeature";
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
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimIntersectionBox*>(uiItem))
        {
            commandIds << "RicAppendIntersectionFeature";
            commandIds << "RicAppendIntersectionBoxFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimEclipseWell*>(uiItem))
        {
            commandIds << "RicNewSimWellIntersectionFeature";
        }
        else if(dynamic_cast<RimFormationNames*>(uiItem))
        {
            commandIds << "RicImportFormationNamesFeature";
            commandIds << "RicReloadFormationNamesFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
        else if(dynamic_cast<RimFormationNamesCollection*>(uiItem))
        {
            commandIds << "RicImportFormationNamesFeature";
            commandIds << "RicReloadFormationNamesFeature";
            //commandIds << "Separator";
            //commandIds << "RicDeleteItemFeature";
        }
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
            commandIds << "RicExecuteScriptForCasesFeature";
        }
        else if (dynamic_cast<RimSummaryCurve*>(uiItem) ||
                 dynamic_cast<RimSummaryCurveFilter*>(uiItem) )
        {
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "RicSummaryCurveSwitchAxisFeature";

        }
        else if (dynamic_cast<RimWellLogCurve*>(uiItem) ||
                 dynamic_cast<RimWellLogTrack*>(uiItem) ||
                 dynamic_cast<RimWellLogPlot*>(uiItem) ||
                 dynamic_cast<RimWellLogPlotCollection*>(uiItem))
        {
            commandIds << "RicExportToLasFileFeature";
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
        caf::CmdFeature* feature = commandManager->getCommandFeature(commandIds[i].toStdString());
        CVF_ASSERT(feature);

        if (feature->canFeatureBeExecuted())
        {
            QAction* act = commandManager->action(commandIds[i]);
            CVF_ASSERT(act);

            menu->addAction(act);
        }
    }
}

