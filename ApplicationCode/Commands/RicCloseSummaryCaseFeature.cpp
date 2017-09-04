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

#include "RicCloseSummaryCaseFeature.h"
#include "RimSummaryCaseCollection.h"
#include "cafSelectionManager.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RiaApplication.h"
#include "RimSummaryCase.h"
#include "cvfAssert.h"
#include <QAction>


CAF_CMD_SOURCE_INIT(RicCloseSummaryCaseFeature, "RicCloseSummaryCaseFeature");


void RicCloseSummaryCaseFeature::setupActionLook(QAction* actionToSetup)
{
	actionToSetup->setText("Close Summary Plot");
	actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseFeature::isCommandEnabled()
{
	return selectedSummaryCase() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RicCloseSummaryCaseFeature::selectedSummaryCase() const
{
	std::vector<RimSummaryCase*> selection;
	caf::SelectionManager::instance()->objectsByType(&selection);

	if (selection.size() > 0)
	{
		return selection[0];
	}

	return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseFeature::onActionTriggered(bool isChecked)
{
	std::vector<RimSummaryCase*> selection;
	caf::SelectionManager::instance()->objectsByType(&selection);
	assert(selection.size() > 0);

	for (RimSummaryCase* summaryCase : selection)
	{
		RimSummaryCaseCollection* summaryCaseCollection = NULL;
		summaryCase->firstAncestorOrThisOfType(summaryCaseCollection);
		CVF_ASSERT(summaryCaseCollection);

		summaryCaseCollection->deleteCase(summaryCase);
		delete summaryCase;
		summaryCaseCollection->updateConnectedEditors();
	}
}