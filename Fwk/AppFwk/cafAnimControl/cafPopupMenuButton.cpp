//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "cafPopupMenuButton.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QVBoxLayout>

using namespace caf;
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

PopupMenuButton::PopupMenuButton( QWidget*            parentWidget,
                                  Qt::Orientation     orientation /*= Qt::Horizontal*/,
                                  ToolButtonPopupMode popupMode /*=InstantPopup*/ )
    : QToolButton( parentWidget )
{
    if ( orientation == Qt::Horizontal )
    {
        m_layout = new QHBoxLayout( this );
    }
    else
    {
        m_layout = new QVBoxLayout( this );
    }
    m_layout->setContentsMargins( QMargins( 2, 2, 2, 2 ) );

    QMenu* menu = new QMenu( this );
    menu->setLayout( m_layout );
    setMenu( menu );

    setCheckable( true );
    setPopupMode( popupMode );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PopupMenuButton::addWidget( QWidget* widget, int stretch, Qt::Alignment alignment )
{
    m_layout->addWidget( widget, stretch, alignment );
}
