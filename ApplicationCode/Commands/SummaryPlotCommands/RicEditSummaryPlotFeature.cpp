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

#include "RicEditSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaSummaryTools.h"

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "RimSummaryPlot.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicEditSummaryPlotFeature, "RicEditSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEditSummaryPlotFeature::RicEditSummaryPlotFeature()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::closeDialogAndResetTargetPlot()
{
    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    if (dialog && dialog->isVisible())
    {
        dialog->hide();
    }

    dialog->updateFromSummaryPlot(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreatorDialog* RicEditSummaryPlotFeature::curveCreatorDialog()
{
    static RicSummaryCurveCreatorDialog* singletonDialog = new RicSummaryCurveCreatorDialog(nullptr);

    return singletonDialog;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryPlotFeature::isCommandEnabled()
{
    if (selectedSummaryPlot()) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

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
        dialog->updateFromSummaryPlot(selectedSummaryPlot());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlotLight16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicEditSummaryPlotFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot(selObj);
    }

    return sumPlot;
}
