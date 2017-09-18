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

#include "RicEditSummaryCurves.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

#include "cvfAssert.h"
#include "cafSelectionManager.h"


CAF_CMD_SOURCE_INIT(RicEditSummaryCurves, "RicEditSummaryCurves");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurves::closeDialogAndResetTargetPlot()
{
    if (m_dialogWithSplitter && m_dialogWithSplitter->isVisible())
    {
        m_dialogWithSplitter->hide();
    }

    if (m_curveCreator)
    {
        m_curveCreator->updateFromSummaryPlot(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryCurves::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurves::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

	if (m_curveCreator == nullptr)
	{
		m_curveCreator = new RicSummaryCurveCreator();
		m_dialogWithSplitter = new RicSummaryCurveCreatorDialog(nullptr, m_curveCreator);
	}

    if (!m_dialogWithSplitter->isVisible())
        m_dialogWithSplitter->show();

    // Set target plot
    std::vector<RimSummaryPlot*> plots;
    caf::SelectionManager::instance()->objectsByType(&plots);
    if (plots.size() == 1)
    {
        m_curveCreator->updateFromSummaryPlot(plots.front());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurves::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Summary Curves");
    //actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}
