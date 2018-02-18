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

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "Rim3dView.h"

#include "RiuMainWindow.h"
#include "RiaLogging.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewViewFeature, "RicNewViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::addReservoirView(RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase)
{
    Rim3dView* newView = createReservoirView(eclipseCase, geomCase);

    if (newView)
    {
        RiuMainWindow::instance()->setExpanded(newView);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewViewFeature::isCommandEnabled()
{
    return selectedEclipseCase() != nullptr
        || selectedEclipseView() != nullptr
        || selectedGeoMechCase() != nullptr
        || selectedGeoMechView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::onActionTriggered(bool isChecked)
{
    // Establish type of selected object
    RimEclipseCase* eclipseCase = selectedEclipseCase();
    RimGeoMechCase* geomCase = selectedGeoMechCase();
    RimGeoMechView* geoMechView = selectedGeoMechView();
    RimEclipseView* reservoirView = selectedEclipseView();
    

    // Find case to insert into
    if (geoMechView) geomCase = geoMechView->geoMechCase();   
    if (reservoirView) eclipseCase = reservoirView->eclipseCase();

    addReservoirView(eclipseCase, geomCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New View");
    actionToSetup->setIcon(QIcon(":/3DView16x16.png"));
}    

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dView* RicNewViewFeature::createReservoirView(RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase)
{
    Rim3dView* insertedView = nullptr;

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
RimEclipseCase* RicNewViewFeature::selectedEclipseCase()
{
    std::vector<RimEclipseCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RicNewViewFeature::selectedGeoMechCase()
{
    std::vector<RimGeoMechCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicNewViewFeature::selectedEclipseView()
{
    std::vector<RimEclipseView*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RicNewViewFeature::selectedGeoMechView()
{
    std::vector<RimGeoMechView*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return nullptr;
}
