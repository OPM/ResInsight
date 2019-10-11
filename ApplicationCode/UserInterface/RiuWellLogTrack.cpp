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

#include "RiuWellLogTrack.h"

#include "RimWellLogTrack.h"

#include <QWheelEvent>

#define RIU_SCROLLWHEEL_ZOOMFACTOR 1.1
#define RIU_SCROLLWHEEL_PANFACTOR 0.1

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::RiuWellLogTrack( RimWellLogTrack* plotTrackDefinition, QWidget* parent /*= nullptr */ )
    : RiuQwtPlotWidget( plotTrackDefinition, parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack::~RiuWellLogTrack() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWellLogTrack::eventFilter( QObject* watched, QEvent* event )
{
    QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>( event );
    if ( wheelEvent && watched == canvas() )
    {
        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( plotDefinition() );
        CAF_ASSERT( track );

        RimWellLogPlot* wellLogPlot = nullptr;
        track->firstAncestorOrThisOfType( wellLogPlot );

        if ( wellLogPlot )
        {
            if ( wheelEvent->modifiers() & Qt::ControlModifier )
            {
                QwtScaleMap scaleMap   = canvasMap( QwtPlot::yLeft );
                double      zoomCenter = scaleMap.invTransform( wheelEvent->pos().y() );

                if ( wheelEvent->delta() > 0 )
                {
                    wellLogPlot->setDepthAxisRangeByFactorAndCenter( RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter );
                }
                else
                {
                    wellLogPlot->setDepthAxisRangeByFactorAndCenter( 1.0 / RIU_SCROLLWHEEL_ZOOMFACTOR, zoomCenter );
                }
            }
            else
            {
                wellLogPlot->setDepthAxisRangeByPanDepth( wheelEvent->delta() < 0 ? RIU_SCROLLWHEEL_PANFACTOR
                                                                                  : -RIU_SCROLLWHEEL_PANFACTOR );
            }

            event->accept();
            return true;
        }
    }
    return RiuQwtPlotWidget::eventFilter( watched, event );
}
