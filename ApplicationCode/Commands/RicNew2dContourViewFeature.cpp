/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNew2dContourViewFeature.h"

#include "Rim2dEclipseView.h"
#include "Rim2dEclipseViewCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseCase.h"
#include "Rim3dView.h"

#include "Riu3DMainWindowTools.h"
#include "RiaLogging.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNew2dContourViewFeature, "RicNew2dContourViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNew2dContourViewFeature::isCommandEnabled()
{
    bool selectedView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>() != nullptr;
    bool selectedCase = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>() != nullptr;
    bool selectedMapCollection = caf::SelectionManager::instance()->selectedItemOfType<Rim2dEclipseViewCollection>();
    return selectedView || selectedCase || selectedMapCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNew2dContourViewFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* reservoirView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    RimEclipseCase* eclipseCase = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseCase>();
    Rim2dEclipseView* contourMap = nullptr;

    // Find case to insert into
    if (reservoirView)
    {
        contourMap = eclipseCase->create2dContourMapFrom3dView(reservoirView);
    }
    else if (eclipseCase)
    {
        contourMap = eclipseCase->create2dContourMap();
    }

    if (contourMap)
    {
        // Must be run before buildViewItems, as wells are created in this function
        contourMap->loadDataAndUpdate();

        if (eclipseCase)
        {
            eclipseCase->updateConnectedEditors();
        }
        caf::SelectionManager::instance()->setSelectedItem(contourMap);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNew2dContourViewFeature::setupActionLook(QAction* actionToSetup)
{
    Rim2dEclipseView* contourMap = caf::SelectionManager::instance()->selectedItemOfType<Rim2dEclipseView>();
    RimEclipseView* eclipseView  = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    if (contourMap)
    {
        actionToSetup->setText("Duplicate Contour Map");
    }
    else if (eclipseView)
    {
        actionToSetup->setText("New Contour Map From 3d View");
    }
    else
    {
        actionToSetup->setText("New Contour Map");
    }
    actionToSetup->setIcon(QIcon(":/2DMap16x16.png"));
}    
