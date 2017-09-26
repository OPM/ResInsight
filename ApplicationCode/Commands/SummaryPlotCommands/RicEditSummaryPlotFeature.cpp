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

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"
#include "RicSummaryCurveCreatorFactoryImpl.h"

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
    m_curveCreatorFactory = RicSummaryCurveCreatorFactoryImpl::instance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::closeDialogAndResetTargetPlot()
{
    auto dialog = m_curveCreatorFactory->dialog();
    auto curveCreator = m_curveCreatorFactory->curveCreator();

    if (dialog && dialog->isVisible())
    {
        dialog->hide();
    }

    if (curveCreator)
    {
        curveCreator->updateFromSummaryPlot(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    auto dialog = m_curveCreatorFactory->dialog();
    auto curveCreator = m_curveCreatorFactory->curveCreator();

    if (!dialog->isVisible())
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    // Set target plot
    std::vector<RimSummaryPlot*> plots;
    caf::SelectionManager::instance()->objectsByType(&plots);
    if (plots.size() == 1)
    {
        curveCreator->updateFromSummaryPlot(plots.front());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}
