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
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

#include "cvfAssert.h"


CAF_CMD_SOURCE_INIT(RicEditSummaryCurves, "RicEditSummaryCurves");

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
		m_dialog = new caf::PdmUiPropertyViewDialog(nullptr, m_curveCreator, "Summary plots", "");
	}
	if(!m_dialog->isVisible())
		m_dialog->show();

//    openSelector(summaryCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurves::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit summary curves in plot");
    //actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}
