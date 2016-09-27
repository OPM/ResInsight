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

#include "RicEditIntersectionBoxEventFeature.h"

#include "RiaApplication.h"

#include "RicBoxManipulatorEventHandler.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimView.h"


#include "RiuViewer.h"


#include <QAction>

CAF_CMD_SOURCE_INIT(RicEditIntersectionBoxEventFeature, "RicEditIntersectionBoxEventFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEditIntersectionBoxEventFeature::RicEditIntersectionBoxEventFeature()
    : m_eventHandler(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxEventFeature::updateGeometry()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView && activeView->viewer())
    {
        activeView->viewer()->addStaticModelOnce(m_eventHandler->model());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxEventFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxEventFeature::onActionTriggered(bool isChecked)
{
    RiuViewer* viewer = nullptr;

    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView && activeView->viewer())
    {
        viewer = activeView->viewer();
    }

    if (isCommandChecked() && m_eventHandler)
    {
        if (viewer)
        {
            viewer->removeEventFilter(m_eventHandler);
            viewer->removeStaticModel(m_eventHandler->model());
        }

        m_eventHandler->deleteLater();
        m_eventHandler = nullptr;
    }
    else if (viewer)
    {
        RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(viewer->lastPickedObject());
        if (intersectionBox)
        {
            m_eventHandler = new RicBoxManipulatorEventHandler(viewer);

            cvf::Mat4d myMat = cvf::Mat4d::fromRotation(cvf::Vec3d::X_AXIS, 0.0);
            myMat.setTranslation(intersectionBox->boxOrigin().translation());
            m_eventHandler->setOrigin(myMat);

            m_eventHandler->setSize(intersectionBox->boxSize());
            m_eventHandler->setScaleZ(activeView->scaleZ());

            RimCase* rimCase = activeView->ownerCase();
            m_eventHandler->setDisplayModelOffset(rimCase->displayModelOffset());

            viewer->installEventFilter(m_eventHandler);
            connect(m_eventHandler, SIGNAL(geometryUpdated()), this, SLOT(updateGeometry()));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxEventFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("Edit Intersection Box (event handler)");
    actionToSetup->setCheckable(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxEventFeature::isCommandChecked()
{
    return m_eventHandler != NULL;
}
