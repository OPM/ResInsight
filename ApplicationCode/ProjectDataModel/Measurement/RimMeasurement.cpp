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

#include "Rim3dView.h"
#include "RimGridView.h"

#include "MeasurementCommands/RicMeasurementPickEventHandler.h"

#include "RiuMeasurementEventFilter.h"
#include "RiuViewerCommands.h"

#include "RiuMainWindow.h"
#include "cvfGeometryTools.h"

CAF_PDM_SOURCE_INIT( RimMeasurement, "RimMeasurement" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::RimMeasurement()
    : m_measurementMode( MEASURE_DISABLED )
{
    CAF_PDM_InitObject( "Measurement", ":/TextAnnotation16x16.png", "", "" );
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

    RiuMainWindow::instance()->refreshViewActions();
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
    auto lengths = calculateLengths();

    QString text;

    if ( m_pointsInDomainCoords.size() > 2 )
    {
        text = QString( "Segment Length: %1\nSegment Horizontal Length: %2\n" )
                   .arg( lengths.lastSegmentLength )
                   .arg( lengths.lastSegmentHorisontalLength );

        text += QString( "Total Length: %1\nTotal Horizontal Length: %2\n" )
                    .arg( lengths.totalLength )
                    .arg( lengths.totalHorizontalLength );

        text += QString( "\nHorizontal Area : %1" ).arg( lengths.area );
    }
    else
    {
        text = QString( "Length: %1\nHorizontal Length: %2\n" )
                   .arg( lengths.lastSegmentLength )
                   .arg( lengths.lastSegmentHorisontalLength );
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement::Lengths RimMeasurement::calculateLengths() const
{
    Lengths lengths;

    for ( size_t p = 1; p < m_pointsInDomainCoords.size(); p++ )
    {
        const auto& p0 = m_pointsInDomainCoords[p - 1];
        const auto& p1 = m_pointsInDomainCoords[p];

        lengths.lastSegmentLength = ( p1 - p0 ).length();

        const auto& p1_horiz = cvf::Vec3d( p1.x(), p1.y(), p0.z() );

        lengths.lastSegmentHorisontalLength = ( p1_horiz - p0 ).length();

        lengths.totalLength += lengths.lastSegmentLength;
        lengths.totalHorizontalLength += lengths.lastSegmentHorisontalLength;
    }

    {
        std::vector<Vec3d> pointsProjectedInZPlane;
        for ( const auto& p : m_pointsInDomainCoords )
        {
            auto pointInZ = p;
            pointInZ.z()  = 0.0;
            pointsProjectedInZPlane.push_back( pointInZ );
        }

        Vec3d area = cvf::GeometryTools::polygonAreaNormal3D( pointsProjectedInZPlane );

        lengths.area = cvf::Math::abs( area.z() );
    }

    return lengths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeasurement::updateView() const
{
    Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();
    rimView->createMeasurementDisplayModelAndRedraw();
}
