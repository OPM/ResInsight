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

#include "RiuMeasurementViewEventFilter.h"

#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "RiaApplication.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMeasurement.h"
#include "RimProject.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QKeyEvent>
#include <QTreeView>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMeasurementViewEventFilter::RiuMeasurementViewEventFilter(QObject* parent)
    : QObject(parent), m_inResetMeasurementMode(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuMeasurementViewEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::MouseButton::LeftButton)
        {
            auto* measurement = RiaApplication::instance()->project()->measurement();
            measurement->removeAllPoints();
            m_inResetMeasurementMode = true;

            //Swallow event
            return true;
        }
    }
    else if (m_inResetMeasurementMode && event->type() == QEvent::MouseButtonRelease)
    {
        // This code is necessary to prevent setting a new measurement point when double clicking
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::MouseButton::LeftButton)
        {
            m_inResetMeasurementMode = false;

            // Swallow event
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}
