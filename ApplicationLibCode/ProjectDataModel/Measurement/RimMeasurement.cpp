/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimMeasurement.h"

#include "RiaApplication.h"

#include "RigPolygonTools.h"

#include "Rim3dView.h"
#include "RimGridView.h"

#include "MeasurementCommands/RicMeasurementPickEventHandler.h"

#include "RiuMainWindow.h"
#include "RiuMeasurementEventFilter.h"
#include "RiuViewerCommands.h"

#include "cvfGeometryTools.h"

CAF_PDM_SOURCE_INIT( RimMeasurement, "RimMeasurement" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::RimMeasurement()
    : m_measurementMode( MEASURE_DISABLED )
{
    CAF_PDM_InitObject( "Measurement", ":/TextAnnotation16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::~RimMeasurement()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::setMeasurementMode( MeasurementMode measurementMode )
{
    m_measurementMode = measurementMode;

    if ( m_measurementMode != MEASURE_DISABLED )
    {
        RicMeasurementPickEventHandler::instance()->registerAsPickEventHandler();
        RicMeasurementPickEventHandler::instance()->enablePolyLineMode( m_measurementMode == MEASURE_POLYLINE );
        m_eventFilter = new RiuMeasurementEventFilter( this );
        m_eventFilter->registerFilter();
    }
    else
    {
        RicMeasurementPickEventHandler::instance()->unregisterAsPickEventHandler();
        removeAllPoints();

        if ( m_eventFilter )
        {
            m_eventFilter->unregisterFilter();
            m_eventFilter->deleteLater();
            m_eventFilter = nullptr;
        }
    }

    if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->refreshViewActions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::MeasurementMode RimMeasurement::measurementMode() const
{
    return m_measurementMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::addPointInDomainCoords( const Vec3d& domainCoord )
{
    auto activeView = dynamic_cast<Rim3dView*>( RiaApplication::instance()->activeMainOrComparisonGridView() );

    if ( m_sourceView.p() != activeView )
    {
        removeAllPoints();
    }

    m_pointsInDomainCoords.push_back( domainCoord );
    m_sourceView = activeView;

    updateView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimMeasurement::pointsInDomainCoords() const
{
    return m_pointsInDomainCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::removeAllPoints()
{
    m_pointsInDomainCoords.clear();
    updateView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMeasurement::label() const
{
    bool includeLastSegmentInfo = true;
    return RigPolygonTools::geometryDataAsText( m_pointsInDomainCoords, includeLastSegmentInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::updateView() const
{
    Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();
    rimView->createMeasurementDisplayModelAndRedraw();
}
