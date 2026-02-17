/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimMswSegment.h"

#include "cafPdmUiDoubleValueEditor.h"

CAF_PDM_SOURCE_INIT( RimMswSegment, "RimMswSegment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswSegment::RimMswSegment()
{
    CAF_PDM_InitObject( "MSW Segment" );

    CAF_PDM_InitFieldNoDefault( &m_segmentNumber, "SegmentNumber", "Segment Number" );
    m_segmentNumber.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_branchNumber, "BranchNumber", "Branch Number" );
    m_branchNumber.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_joinSegment, "JoinSegment", "Join Segment" );
    m_joinSegment.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_length, "Length", "Length" );
    m_length.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_depth, "Depth", "Depth" );
    m_depth.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_nextSegmentLength, "NextSegmentLength", "Next Segment Length" );
    m_nextSegmentLength.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_diameter, "Diameter", "Diameter" );
    m_diameter.uiCapability()->setUiReadOnly( true );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswSegment::~RimMswSegment()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegment::setSegmentData( int    segmentNumber,
                                    int    branchNumber,
                                    int    joinSegment,
                                    double length,
                                    double depth,
                                    double nextSegmentLength,
                                    double diameter )
{
    m_segmentNumber     = segmentNumber;
    m_branchNumber      = branchNumber;
    m_joinSegment       = joinSegment;
    m_length            = length;
    m_depth             = depth;
    m_nextSegmentLength = nextSegmentLength;
    m_diameter          = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMswSegment::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMswSegment::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMswSegment::joinSegment() const
{
    return m_joinSegment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::length() const
{
    return m_length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::depth() const
{
    return m_depth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::nextSegmentLength() const
{
    return m_nextSegmentLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::diameter() const
{
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimMswSegment::isEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimMswSegment::componentType() const
{
    return RiaDefines::WellPathComponentType::MSW_SEGMENT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMswSegment::componentLabel() const
{
    return QString( "Seg %1\n%2 - %3" ).arg( segmentNumber() ).arg( startMD(), 0, 'f', 1 ).arg( endMD(), 0, 'f', 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMswSegment::componentTypeLabel() const
{
    return "MSW Segment";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimMswSegment::defaultComponentColor() const
{
    // Alternating colors based on segment number
    if ( segmentNumber() % 2 == 0 )
    {
        return cvf::Color3f( 0.3f, 0.5f, 0.8f ); // Blue
    }
    return cvf::Color3f( 0.3f, 0.7f, 0.9f ); // Cyan
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::startMD() const
{
    return m_length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMswSegment::endMD() const
{
    return m_nextSegmentLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegment::applyOffset( double offsetMD )
{
    // This does not make sense to apply offset to MSW segments, as they are not connected to each other and the start/end MD is only used
    // for display purposes. So we do nothing here.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMswSegment::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_segmentNumber );
    uiOrdering.add( &m_branchNumber );
    uiOrdering.add( &m_length );
    uiOrdering.add( &m_depth );
    uiOrdering.add( &m_nextSegmentLength );
    uiOrdering.add( &m_joinSegment );
    uiOrdering.add( &m_diameter );

    uiOrdering.skipRemainingFields( true );
}
