/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicCloseSummaryCaseCollectionFeature.h"

#include "RiaApplication.h"

#include "RicCloseSummaryCaseFeature.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <vector>


CAF_CMD_SOURCE_INIT(RicCloseSummaryCaseCollectionFeature, "RicCloseSummaryCaseCollectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseCollectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close Sub Items");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseCollectionFeature::isCommandEnabled()
{
    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    caf::SelectionManager::instance()->objectsByType(&summaryCaseCollections);
    return (summaryCaseCollections.size() > 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseCollectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    caf::SelectionManager::instance()->objectsByType(&summaryCaseCollections);

    for (RimSummaryCaseCollection* summaryCaseCollection : summaryCaseCollections)
    {
        std::vector<RimSummaryCase*> summaryCasesFromCollection;

        for (int i = 0; i < summaryCaseCollection->summaryCaseCount(); i++)
        {
            summaryCasesFromCollection.push_back(summaryCaseCollection->summaryCase(i));
        }
        
        RicCloseSummaryCaseFeature::deleteSummaryCases(summaryCasesFromCollection);
    }
}

