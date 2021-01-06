/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiuVfpQwtPlot.h"

#include "RiuQwtPlotTools.h"

#include "RimVfpPlot.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuVfpQwtPlot::RiuVfpQwtPlot( RimVfpPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuQwtPlotWidget( plot, parent )
{
    RiuQwtPlotTools::setCommonPlotBehaviour( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuVfpQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    emit plotSelected( false );

    menuBuilder << "RicShowPlotDataFeature";

    menuBuilder.appendToMenu( &menu );

    if ( menu.actions().size() > 0 )
    {
        menu.exec( event->globalPos() );
    }
}
