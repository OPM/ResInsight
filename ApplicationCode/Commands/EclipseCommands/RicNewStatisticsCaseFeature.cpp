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

#include "RicNewStatisticsCaseFeature.h"

#include "RimEclipseStatisticsCase.h"
#include "RimEclipseStatisticsCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimCaseCollection.h"
#include "RimProject.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewStatisticsCaseFeature, "RicNewStatisticsCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewStatisticsCaseFeature::isCommandEnabled()
{
    return selectedValidUIItem() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsCaseFeature::onActionTriggered(bool isChecked)
{
    caf::PdmUiItem* uiItem = selectedValidUIItem();
    if (uiItem)
    {
        RimEclipseStatisticsCase* newCase = addStatisticalCalculation(uiItem);
        if (newCase)
        {
            RiuMainWindow::instance()->selectAsCurrentItem(newCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Statistics Case");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiItem* RicNewStatisticsCaseFeature::selectedValidUIItem()
{
    std::vector<RimEclipseStatisticsCaseCollection*> statisticsCaseCollections;
    caf::SelectionManager::instance()->objectsByType(&statisticsCaseCollections);

    if (statisticsCaseCollections.size() > 0)
    {
        return statisticsCaseCollections[0];
    }

    std::vector<RimCaseCollection*> caseCollections;
    caf::SelectionManager::instance()->objectsByType(&caseCollections);

    if (caseCollections.size() > 0)
    {
        if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollections[0]))
        {
            return caseCollections[0];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RicNewStatisticsCaseFeature::addStatisticalCalculation(caf::PdmUiItem* uiItem)
{
    RimIdenticalGridCaseGroup* caseGroup = nullptr;

    if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem))
    {
        RimEclipseStatisticsCase* currentObject = dynamic_cast<RimEclipseStatisticsCase*>(uiItem);
        caseGroup = currentObject->parentStatisticsCaseCollection()->parentCaseGroup();
    }
    else if (dynamic_cast<RimCaseCollection*>(uiItem))
    {
        RimCaseCollection* statColl = dynamic_cast<RimCaseCollection*>(uiItem);
        caseGroup = statColl->parentCaseGroup();
    }

    if (caseGroup)
    {
        RimProject* proj = RiaApplication::instance()->project();
        RimEclipseStatisticsCase* createdObject = caseGroup->createAndAppendStatisticsCase();
        proj->assignCaseIdToCase(createdObject);

        caseGroup->updateConnectedEditors();
        return createdObject;
    }
    else
    {
        return nullptr;
    }
}
