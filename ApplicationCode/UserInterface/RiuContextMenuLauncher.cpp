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

#include "RiuContextMenuLauncher.h"

#include "RimContextCommandBuilder.h"

#include "cvfAssert.h"

#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QWidget>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuContextMenuLauncher::RiuContextMenuLauncher(QWidget* widget, const QStringList& commandIds) : 
    QObject(widget),
    m_commandIds(commandIds)
{
    widget->installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuContextMenuLauncher::eventFilter(QObject* watchedObject, QEvent* event)
{
    if (event->type() == QEvent::ContextMenu)
    {
        QMenu menu;
        RimContextCommandBuilder::appendCommandsToMenu(m_commandIds, &menu);

        if (menu.actions().size() > 0)
        {
            QContextMenuEvent* cme = static_cast<QContextMenuEvent*>(event);
            CVF_ASSERT(cme);

            menu.exec(cme->globalPos());
        }
        
        return true;
    }

    // standard event processing
    return QObject::eventFilter(watchedObject, event);
}

