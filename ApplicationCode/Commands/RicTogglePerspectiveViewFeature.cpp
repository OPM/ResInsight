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

#include "RicTogglePerspectiveViewFeature.h"

#include "RiuViewer.h"
#include "Rim3dView.h"
#include "RiuMainWindow.h"
#include "RiaApplication.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicTogglePerspectiveViewFeature, "RicTogglePerspectiveViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicTogglePerspectiveViewFeature::isCommandEnabled()
{
    this->action(); // Retrieve the action to update the looks
    return RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTogglePerspectiveViewFeature::onActionTriggered(bool isChecked)
{
    if(RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        bool isPerspective = RiaApplication::instance()->activeReservoirView()->isPerspectiveView();
        RiaApplication::instance()->activeReservoirView()->isPerspectiveView = !isPerspective;
        RiaApplication::instance()->activeReservoirView()->isPerspectiveView.uiCapability()->updateConnectedEditors();

        RiaApplication::instance()->activeReservoirView()->viewer()->enableParallelProjection(isPerspective);
        RiaApplication::instance()->activeReservoirView()->viewer()->navigationPolicyUpdate();

        this->action(); // Retrieve the action to update the looks
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTogglePerspectiveViewFeature::setupActionLook(QAction* actionToSetup)
{
    if(RiaApplication::instance()->activeReservoirView() &&  RiaApplication::instance()->activeReservoirView()->viewer())
    {
        if (RiaApplication::instance()->activeReservoirView()->isPerspectiveView())
        {
            actionToSetup->setText("Parallel View");
            actionToSetup->setIcon(QIcon(":/Parallel24x24.png"));
            return;
        }
    }

    actionToSetup->setText("Perspective View");
    actionToSetup->setIcon(QIcon(":/Perspective24x24.png"));
}
