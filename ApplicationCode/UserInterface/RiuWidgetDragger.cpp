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

#include "RiuWidgetDragger.h"
#include <QEvent>
#include <QWidget>
#include <QMouseEvent>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWidgetDragger::RiuWidgetDragger(QWidget* widgetToMove)
    : QObject(widgetToMove)
    , m_widgetToMove(widgetToMove)
    , m_startPos(0,0)
{
    m_widgetToMove->installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuWidgetDragger::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mMoveEv = static_cast<QMouseEvent*>(event);
        if (mMoveEv->buttons() & Qt::LeftButton)
        {
            m_widgetToMove->move( m_widgetToMove->mapToParent(mMoveEv->pos() - m_startPos));
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mEv = static_cast<QMouseEvent*>(event);
        m_startPos = mEv->pos();
    }

    return false;
}

