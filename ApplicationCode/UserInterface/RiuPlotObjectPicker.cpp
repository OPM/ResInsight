/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuPlotObjectPicker.h"

#include "RiaApplication.h"
#include "RiuMainPlotWindow.h"

#include <QMouseEvent>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPlotObjectPicker::RiuPlotObjectPicker(QWidget* widget, caf::PdmObject* associatedObject) :
    QObject(widget),
    m_associatedObject(associatedObject)
{
    widget->installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuPlotObjectPicker::eventFilter(QObject* watchedObject, QEvent* event)
{
    RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
    if (mainPlotWindow && m_associatedObject.notNull())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::RightButton)
            {
                mainPlotWindow->selectAsCurrentItem(m_associatedObject);
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton)
            {
                mainPlotWindow->selectAsCurrentItem(m_associatedObject);
                return true;
            }
        }
    }

    // standard event processing
    return QObject::eventFilter(watchedObject, event);
}

