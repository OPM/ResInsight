/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaViewRedrawScheduler.h"
#include "Rim3dView.h"

#include <QTimer>
#include <QCoreApplication>

#include <set>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaViewRedrawScheduler* RiaViewRedrawScheduler::instance()
{
    static RiaViewRedrawScheduler theInstance; 
    
    return &theInstance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaViewRedrawScheduler::clearViewsScheduledForUpdate()
{
    if (m_resViewUpdateTimer)
    {
        while (m_resViewUpdateTimer->isActive())
        {
            QCoreApplication::processEvents();
        }
    }
    m_resViewsToUpdate.clear();
}


//--------------------------------------------------------------------------------------------------
/// Schedule a creation of the Display model and redraw of the reservoir view
/// The redraw will happen as soon as the event loop is entered
//--------------------------------------------------------------------------------------------------
void RiaViewRedrawScheduler::scheduleDisplayModelUpdateAndRedraw(Rim3dView* resViewToUpdate)
{
    m_resViewsToUpdate.push_back(resViewToUpdate);

    if (!m_resViewUpdateTimer) 
    {
        m_resViewUpdateTimer = new QTimer(this);
        connect(m_resViewUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateScheduledDisplayModels()));
    }

    if (!m_resViewUpdateTimer->isActive())
    {
        m_resViewUpdateTimer->setSingleShot(true);
        m_resViewUpdateTimer->start(0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaViewRedrawScheduler::slotUpdateScheduledDisplayModels()
{
    // Compress to remove duplicates
    // and update dependent views after independent views

    std::set<Rim3dView*> independent3DViewsToUpdate;
    std::set<Rim3dView*> dependent3DViewsToUpdate;

    for (size_t i = 0; i < m_resViewsToUpdate.size(); ++i)
    {
        if (!m_resViewsToUpdate[i]) continue;

        if (m_resViewsToUpdate[i]->viewController())
            dependent3DViewsToUpdate.insert(m_resViewsToUpdate[i]);
        else
            independent3DViewsToUpdate.insert(m_resViewsToUpdate[i]);
    }

    for (std::set<Rim3dView*>::iterator it = independent3DViewsToUpdate.begin(); it != independent3DViewsToUpdate.end(); ++it )
    {
        if (*it)
        {
            (*it)->createDisplayModelAndRedraw();
        }
    }

    for (std::set<Rim3dView*>::iterator it = dependent3DViewsToUpdate.begin(); it != dependent3DViewsToUpdate.end(); ++it)
    {
        if (*it)
        {
            (*it)->createDisplayModelAndRedraw();
        }
    }

    m_resViewsToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaViewRedrawScheduler::~RiaViewRedrawScheduler()
{
    delete m_resViewUpdateTimer;
}
