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
#include "RimCrossSection.h"
#include "RimCrossSectionCollection.h"
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
#include "RimGeoMechCase.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
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
    else if (uiItems.size() > 1)
    {
        caf::PdmUiItem* uiItem = uiItems[0];
        if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            commandIds << "RicAddWellLogToPlotFeature";
        }
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
            commandIds << "RicNewViewFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "RicPasteGeoMechViewsFeature";
            commandIds << "RicDeleteItemFeature";
            commandIds << "Separator";
        }
        else if (dynamic_cast<RimEclipseView*>(uiItem))
        {
            commandIds << "RicNewViewFeature";
            commandIds << "RicCopyReferencesToClipboardFeature";
            commandIds << "RicPasteEclipseViewsFeature";
            commandIds << "RicDeleteItemFeature";
            commandIds << "Separator";
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
            commandIds << "RicCloseCaseFeature";
            commandIds << "RicExecuteScriptForCasesFeature";
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            commandIds << "RicCopyReferencesToClipboardFeature";

            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "RicPasteEclipseViewsFeature";

            commandIds << "RicCloseCaseFeature";
            commandIds << "RicNewViewFeature";
            commandIds << "RicEclipseCaseNewGroupFeature";
            commandIds << "RicExecuteScriptForCasesFeature";
        }
        else if (dynamic_cast<RimGeoMechCase*>(uiItem))
        {
            commandIds << "RicPasteGeoMechViewsFeature";
            commandIds << "RicNewViewFeature";
            commandIds << "Separator";

            commandIds << "RicCloseCaseFeature";
        }
        else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem))
        {
            commandIds << "RicEclipseCaseNewGroupFeature";
            commandIds << "RicPasteEclipseCasesFeature";
            commandIds << "RicDeleteItemFeature";
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
            commandIds << "RicDeleteItemFeature";
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
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem))
        {
            commandIds << "RicEclipsePropertyFilterInsertFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterNewFeature";
        }
        else if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem))
        {
            commandIds << "RicGeoMechPropertyFilterInsertFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimWellPathCollection*>(uiItem))
        {
            commandIds << "RicWellPathsDeleteAllFeature";
            commandIds << "RicWellPathsImportFileFeature";
            commandIds << "RicWellPathsImportSsihubFeature";
            commandIds << "RicWellLogsImportFileFeature";
        }
        else if (dynamic_cast<RimWellPath*>(uiItem))
        {
            commandIds << "RicNewWellLogFileCurveFeature";
            commandIds << "RicNewWellLogCurveExtractionFeature";
            commandIds << "RicNewWellPathCrossSectionFeature";
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
            commandIds << "RicAddScriptPathFeature";
            commandIds << "RicDeleteScriptPathFeature";
        }
        else if (dynamic_cast<RimViewController*>(uiItem))
        {
            commandIds << "RicShowAllLinkedViewsFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
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
        }
        else if (dynamic_cast<RimWellLogPlot*>(uiItem))
        {
            commandIds << "RicNewWellLogPlotTrackFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
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
            commandIds << "RicExportToLasFileFeature";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimSummaryPlot*>(uiItem))
        {
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicViewZoomAllFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimSummaryCurve*>(uiItem))
        {
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if(dynamic_cast<RimSummaryCurveFilter*>(uiItem))
        {
            commandIds << "RicNewSummaryCurveFilterFeature";
            commandIds << "RicNewSummaryCurveFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimSummaryCase*>(uiItem))
        {
            commandIds << "RicNewSummaryPlotFeature";
        }
        else if (dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            commandIds << "RicAddWellLogToPlotFeature";
        }
        else if (dynamic_cast<RimCrossSectionCollection*>(uiItem))
        {
            commandIds << "RicAppendCrossSectionFeature";
        }
        else if (dynamic_cast<RimCrossSection*>(uiItem))
        {
            commandIds << "RicAppendCrossSectionFeature";
            commandIds << "Separator";
            commandIds << "RicDeleteItemFeature";
        }
        else if (dynamic_cast<RimEclipseWell*>(uiItem))
        {
            commandIds << "RicNewSimWellCrossSectionFeature";
        }

        if (dynamic_cast<RimView*>(uiItem))
        {
            commandIds << "RicLinkVisibleViewsFeature";
            commandIds << "RicLinkViewFeature";
            commandIds << "RicUnLinkViewFeature";
            commandIds << "RicShowLinkOptionsFeature";
            commandIds << "RicSetMasterViewFeature";
        }
    }

    if (RicToggleItemsFeatureImpl::isToggleCommandsAvailable())
    {
        commandIds << "Separator";
        commandIds << "RicToggleItemsOnFeature";
        commandIds << "RicToggleItemsOffFeature";
        commandIds << "RicToggleItemsFeature";
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

