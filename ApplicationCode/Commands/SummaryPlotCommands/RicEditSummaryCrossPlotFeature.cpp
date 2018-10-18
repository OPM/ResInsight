/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicEditSummaryCrossPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include "RimSummaryPlot.h"


CAF_CMD_SOURCE_INIT(RicEditSummaryCrossPlotFeature, "RicEditSummaryCrossPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEditSummaryCrossPlotFeature::RicEditSummaryCrossPlotFeature()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCrossPlotFeature::closeDialogAndResetTargetPlot()
{
    auto dialog = RicEditSummaryCrossPlotFeature::curveCreatorDialog();

    if (dialog && dialog->isVisible())
    {
        dialog->hide();
    }

    dialog->updateFromSummaryPlot(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorDialog* RicEditSummaryCrossPlotFeature::curveCreatorDialog()
{
    static RicSummaryCurveCreatorDialog* singletonDialog = new RicSummaryCurveCreatorDialog(nullptr);

    return singletonDialog;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryCrossPlotFeature::isCommandEnabled()
{
    if (selectedSummaryPlot()) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCrossPlotFeature::onActionTriggered(bool isChecked)
{
    auto dialog = RicEditSummaryCrossPlotFeature::curveCreatorDialog();

    if (!dialog->isVisible())
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    // Set target plot
    if (selectedSummaryPlot())
    {
        //dialog->updateFromSummaryPlot(selectedSummaryPlot());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryXPlotLight16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicEditSummaryCrossPlotFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        selObj->firstAncestorOrThisOfType(sumPlot);
    }

    return sumPlot;
}
