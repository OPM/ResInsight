/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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
#include "RicMswSegment.h"

#include "RicMswCompletions.h"
#include "RicMswExportInfo.h"

#include <cafPdmBase.h>
#include <cafPdmObject.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSegment::RicMswSegment( const QString& label,
                              double         startMD,
                              double         endMD,
                              double         startTVD,
                              double         endTVD,
                              size_t         subIndex,
                              int            segmentNumber /*= -1*/ )
    : RicMswItem( label )
    , m_startMD( startMD )
    , m_endMD( endMD )
    , m_startTVD( startTVD )
    , m_endTVD( endTVD )
    , m_outputMD( 0.0 )
    , m_outputTVD( 0.0 )
    , m_equivalentDiameter( 0.15 )
    , m_holeDiameter( RicMswExportInfo::defaultDoubleValue() )
    , m_openHoleRoughnessFactor( 5.0e-5 )
    , m_skinFactor( RicMswExportInfo::defaultDoubleValue() )
    , m_subIndex( subIndex )
    , m_segmentNumber( segmentNumber )
    , m_effectiveDiameter( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::startTVD() const
{
    return m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::endTVD() const
{
    return m_endTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setOutputMD( double outputMD )
{
    m_outputMD = outputMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::outputMD() const
{
    return m_outputMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setOutputTVD( double outputTVD )
{
    m_outputTVD = outputTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::outputTVD() const
{
    return m_outputTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::equivalentDiameter() const
{
    return m_equivalentDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::holeDiameter() const
{
    return m_holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::openHoleRoughnessFactor() const
{
    return m_openHoleRoughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswSegment::subIndex() const
{
    return m_subIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswSegment::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswCompletion*> RicMswSegment::completions() const
{
    std::vector<const RicMswCompletion*> allCompletions;
    for ( const auto& completion : m_completions )
    {
        allCompletions.push_back( completion.get() );
    }
    return allCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswCompletion*> RicMswSegment::completions()
{
    std::vector<RicMswCompletion*> allCompletions;
    for ( auto& completion : m_completions )
    {
        allCompletions.push_back( completion.get() );
    }
    return allCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setLabel( const QString& label )
{
    m_label = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setEquivalentDiameter( double effectiveDiameter )
{
    m_equivalentDiameter = effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setHoleDiameter( double holeDiameter )
{
    m_holeDiameter = holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setOpenHoleRoughnessFactor( double roughnessFactor )
{
    m_openHoleRoughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSkinFactor( double skinFactor )
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSegmentNumber( int segmentNumber )
{
    m_segmentNumber = segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::effectiveDiameter() const
{
    return m_effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setEffectiveDiameter( double effectiveDiameter )
{
    m_effectiveDiameter = effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::addCompletion( std::unique_ptr<RicMswCompletion> completion )
{
    m_completions.push_back( std::move( completion ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RicMswCompletion> RicMswSegment::removeCompletion( RicMswCompletion* completion )
{
    std::unique_ptr<RicMswCompletion> removedCompletion;
    for ( auto it = m_completions.begin(); it != m_completions.end(); ++it )
    {
        if ( it->get() == completion )
        {
            removedCompletion = std::move( *it );
            m_completions.erase( it );
            break;
        }
    }
    return removedCompletion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::addIntersection( std::shared_ptr<RicMswSegmentCellIntersection> intersection )
{
    m_intersections.push_back( intersection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<RicMswSegmentCellIntersection>>& RicMswSegment::intersections() const
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswSegmentCellIntersection>>& RicMswSegment::intersections()
{
    return m_intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<size_t> RicMswSegment::globalCellsIntersected() const
{
    return m_intersectedGlobalCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setIntersectedGlobalCells( const std::set<size_t>& intersectedCells )
{
    m_intersectedGlobalCells = intersectedCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSourcePdmObject( const caf::PdmObject* object )
{
    m_sourcePdmObject = const_cast<caf::PdmObject*>( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmObject* RicMswSegment::sourcePdmObject() const
{
    return m_sourcePdmObject;
}
