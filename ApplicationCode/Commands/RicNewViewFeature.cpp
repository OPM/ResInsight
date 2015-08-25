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

#include "RicNewViewFeature.h"

#include "RimView.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafPdmUiTreeView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewViewFeature, "RicNewViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewViewFeature::isCommandEnabled()
{
    return selectedEclipseCase() != NULL
        || selectedEclipseView() != NULL
        || selectedGeoMechCase() != NULL
        || selectedGeoMechView() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::onActionTriggered(bool isChecked)
{
    RimView* newView = addReservoirView();

    RiuMainWindow::instance()->projectTreeView()->setExpanded(newView, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New View");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RicNewViewFeature::addReservoirView()
{
    // Establish type of selected object
    RimEclipseCase* eclipseCase = selectedEclipseCase();
    RimGeoMechCase* geomCase = selectedGeoMechCase();
    RimGeoMechView* geoMechView = selectedGeoMechView();
    RimEclipseView* reservoirView = selectedEclipseView();

    // Find case to insert into
    if (geoMechView) geomCase = geoMechView->geoMechCase();   
    if (reservoirView) eclipseCase = reservoirView->eclipseCase();

    RimView* insertedView = NULL;

    if (eclipseCase)
    {
        insertedView = eclipseCase->createAndAddReservoirView();
    }
    else if (geomCase)
    {
        insertedView = geomCase->createAndAddReservoirView();
    }

    // Must be run before buildViewItems, as wells are created in this function
    insertedView->loadDataAndUpdate();

    if (eclipseCase)
    {
        eclipseCase->updateConnectedEditors();
    }

    if (geomCase)
    {
        geomCase->updateConnectedEditors();
    }

    return insertedView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicNewViewFeature::selectedEclipseCase() const
{
    std::vector<RimEclipseCase*> selection;
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
RimGeoMechCase* RicNewViewFeature::selectedGeoMechCase() const
{
    std::vector<RimGeoMechCase*> selection;
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
RimEclipseView* RicNewViewFeature::selectedEclipseView() const
{
    std::vector<RimEclipseView*> selection;
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
RimGeoMechView* RicNewViewFeature::selectedGeoMechView() const
{
    std::vector<RimGeoMechView*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}
