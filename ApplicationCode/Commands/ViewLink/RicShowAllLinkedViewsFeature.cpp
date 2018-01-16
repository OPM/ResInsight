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

#include "RicShowAllLinkedViewsFeature.h"

#include "RimViewController.h"
#include "RimGridView.h"
#include "RimViewLinker.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowAllLinkedViewsFeature, "RicShowAllLinkedViewsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowAllLinkedViewsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowAllLinkedViewsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimViewLinker*> linkedViews;
    caf::SelectionManager::instance()->objectsByType(&linkedViews);

    std::vector<RimViewController*> managedViews;
    caf::SelectionManager::instance()->objectsByType(&managedViews);
    for (size_t i = 0; i < managedViews.size(); i++)
    {
        RimViewLinker* rimLinked = NULL;
        managedViews[i]->firstAncestorOrThisOfType(rimLinked);
        CVF_ASSERT(rimLinked);

        linkedViews.push_back(rimLinked);
    }

    for (size_t i = 0; i < linkedViews.size(); i++)
    {
        std::vector<RimGridView*> views;
        linkedViews[i]->allViews(views);

        for (size_t j = 0; j < views.size(); j++)
        {
            views[j]->forceShowWindowOn();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowAllLinkedViewsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Open All Linked Views");
}

