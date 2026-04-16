/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RiuQwtCurveSelectorFilter.h"

#include "RiuDockWidgetTools.h"

#include "qwt_plot.h"
#include "qwt_scale_map.h"

#include <QMouseEvent>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtCurveSelectorFilter::RiuQwtCurveSelectorFilter( QwtPlot* plot, ItemFinder finder )
    : QObject( plot->canvas() )
    , m_finder( std::move( finder ) )
{
    plot->canvas()->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuQwtCurveSelectorFilter::closestPointIndex( QwtPlot* plot, const QPoint& canvasPos, const std::vector<std::pair<double, double>>& points )
{
    auto xMap = plot->canvasMap( QwtAxis::XBottom );
    auto yMap = plot->canvasMap( QwtAxis::YLeft );

    const double xRange          = std::abs( xMap.s2() - xMap.s1() );
    const double yRange          = std::abs( yMap.s2() - yMap.s1() );
    const double minRangeEpsilon = 1e-14;
    if ( xRange < minRangeEpsilon || yRange < minRangeEpsilon ) return -1;

    const double clickX = xMap.invTransform( canvasPos.x() );
    const double clickY = yMap.invTransform( canvasPos.y() );

    double minDist    = std::numeric_limits<double>::max();
    int    closestIdx = -1;
    for ( int i = 0; i < static_cast<int>( points.size() ); ++i )
    {
        const double dx   = ( points[i].first - clickX ) / xRange;
        const double dy   = ( points[i].second - clickY ) / yRange;
        const double dist = dx * dx + dy * dy;
        if ( dist < minDist )
        {
            minDist    = dist;
            closestIdx = i;
        }
    }
    return ( closestIdx >= 0 && minDist < 0.03 * 0.03 ) ? closestIdx : -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuQwtCurveSelectorFilter::eventFilter( QObject* /*watched*/, QEvent* event )
{
    if ( event->type() == QEvent::MouseButtonPress )
    {
        auto* mouseEvent = static_cast<QMouseEvent*>( event );
        if ( mouseEvent->button() == Qt::LeftButton )
        {
            if ( const caf::PdmUiItem* item = m_finder( mouseEvent->pos() ) )
            {
                RiuDockWidgetTools::selectItemsInTreeView( RiuDockWidgetTools::plotMainWindowDataSourceTreeName(), { item } );
            }
        }
    }
    return false;
}
