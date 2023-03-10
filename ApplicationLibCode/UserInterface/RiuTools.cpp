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

#include "RiuTools.h"

#include "QMenu"
#include "QObject"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::WindowFlags RiuTools::defaultDialogFlags()
{
    Qt::WindowFlags f = Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

    return f;
}

//--------------------------------------------------------------------------------------------------
/// When a cafCmdFeature is used to create an action, the enable state is controlled by cafCmdFeature::isCommandEnabled(). If an action is
/// used in menus with no selection/context available, the enable state can be forced on before the menu is displayed.
//--------------------------------------------------------------------------------------------------
void RiuTools::enableAllActionsOnShow( QObject* object, QMenu* menu )
{
    if ( object && menu )
    {
        object->connect( menu, &QMenu::aboutToShow, [menu]() {
            for ( auto act : menu->actions() )
                act->setEnabled( true );
        } );
    }
}
