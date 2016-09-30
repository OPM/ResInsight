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

#include "RicEditIntersectionBoxFeature.h"

#include "RiaApplication.h"

#include "RicBoxManipulatorEventHandler.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEditIntersectionBoxFeature, "RicEditIntersectionBoxFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEditIntersectionBoxFeature::RicEditIntersectionBoxFeature()
    : m_eventHandler(nullptr),
    m_intersectionBox(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::slotScheduleRedraw()
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (activeView && activeView->viewer())
    {
        activeView->viewer()->addStaticModelOnce(m_eventHandler->model());

        activeView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::slotUpdateGeometry(const cvf::Vec3d& origin, const cvf::Vec3d& size)
{
    if (m_intersectionBox)
    {
        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        if (activeView)
        {
            cvf::ref<caf::DisplayCoordTransform> transForm = activeView->displayCoordTransform();

            cvf::Vec3d domainOrigin = transForm->transformToDomainCoord(origin);
            cvf::Vec3d domainSize   = transForm->scaleToDomainSize(size);

            m_intersectionBox->setFromOriginAndSize(domainOrigin, domainSize);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::onActionTriggered(bool isChecked)
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
            viewer->removeStaticModel(m_eventHandler->model());
        }

        m_eventHandler->deleteLater();
        m_eventHandler = nullptr;

        m_intersectionBox = nullptr;
    }
    else if (viewer)
    {
        m_intersectionBox = dynamic_cast<RimIntersectionBox*>(viewer->lastPickedObject());
        if (m_intersectionBox)
        {
            m_eventHandler = new RicBoxManipulatorEventHandler(viewer);
            connect(m_eventHandler, SIGNAL(notifyRedraw()), this, SLOT(slotScheduleRedraw()));
            connect(m_eventHandler, SIGNAL(notifyUpdate(const cvf::Vec3d&, const cvf::Vec3d&)), this, SLOT(slotUpdateGeometry(const cvf::Vec3d&, const cvf::Vec3d&)));

            cvf::ref<caf::DisplayCoordTransform> transForm = activeView->displayCoordTransform();

            m_eventHandler->setOrigin(transForm->transformToDisplayCoord(m_intersectionBox->boxOrigin().translation()));
            m_eventHandler->setSize(transForm->scaleToDisplaySize(m_intersectionBox->boxSize()));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditIntersectionBoxFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("Edit Intersection Box");
    actionToSetup->setCheckable(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditIntersectionBoxFeature::isCommandChecked()
{
    return m_eventHandler != NULL;
}
