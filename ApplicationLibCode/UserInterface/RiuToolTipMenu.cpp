/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuToolTipMenu.h"

#include <QHelpEvent>
#include <QToolTip>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuToolTipMenu::RiuToolTipMenu( QWidget* parent )
    : QMenu( parent )
{
}

bool RiuToolTipMenu::event( QEvent* e )
{
    if ( e->type() == QEvent::ToolTip )
    {
        QHelpEvent* he  = dynamic_cast<QHelpEvent*>( e );
        QAction*    act = actionAt( he->pos() );
        if ( act )
        {
            // Remove ampersand as this is used to define shortcut key
            QString actionTextWithoutAmpersand = act->text().remove( "&" );

            if ( actionTextWithoutAmpersand != act->toolTip() )
            {
                QToolTip::showText( he->globalPos(), act->toolTip(), this );
            }

            return true;
        }
    }
    else if ( e->type() == QEvent::Paint && QToolTip::isVisible() )
    {
        QToolTip::hideText();
    }

    return QMenu::event( e );
}
