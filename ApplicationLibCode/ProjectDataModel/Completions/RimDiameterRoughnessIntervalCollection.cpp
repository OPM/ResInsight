/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimDiameterRoughnessIntervalCollection.h"

#include "RimDiameterRoughnessInterval.h"
#include "RimMswCompletionParameters.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimDiameterRoughnessIntervalCollection, "DiameterRoughnessIntervalCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessIntervalCollection::RimDiameterRoughnessIntervalCollection()
{
    CAF_PDM_InitObject( "Diameter Roughness Intervals", ":/WellPathComponent16x16.png" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Intervals", "Intervals" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessIntervalCollection::~RimDiameterRoughnessIntervalCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessInterval*
    RimDiameterRoughnessIntervalCollection::createInterval( double startMD, double endMD, double diameter, double roughness )
{
    auto* interval = new RimDiameterRoughnessInterval();
    interval->setStartMD( startMD );
    interval->setEndMD( endMD );
    interval->setDiameter( diameter );
    interval->setRoughnessFactor( roughness );

    addInterval( interval );
    return interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessIntervalCollection::getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto* interval = findIntervalAtMD( md );
    if ( interval )
    {
        return interval->diameter( unitSystem );
    }

    // Return default if no interval found
    return RimMswCompletionParameters::defaultLinerDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessIntervalCollection::getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto* interval = findIntervalAtMD( md );
    if ( interval )
    {
        return interval->roughnessFactor( unitSystem );
    }

    // Return default if no interval found
    return RimMswCompletionParameters::defaultRoughnessFactor( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessIntervalCollection::getDiameterAtMD( double                        md,
                                                                RiaDefines::EclipseUnitSystem unitSystem,
                                                                const QDateTime&              exportDate ) const
{
    for ( auto* interval : intervals() )
    {
        if ( interval && interval->containsMD( md ) && interval->isActiveOnDate( exportDate ) )
        {
            return interval->diameter( unitSystem );
        }
    }

    // Return default if no active interval found at this date
    return RimMswCompletionParameters::defaultLinerDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimDiameterRoughnessIntervalCollection::getRoughnessAtMD( double                        md,
                                                                 RiaDefines::EclipseUnitSystem unitSystem,
                                                                 const QDateTime&              exportDate ) const
{
    for ( auto* interval : intervals() )
    {
        if ( interval && interval->containsMD( md ) && interval->isActiveOnDate( exportDate ) )
        {
            return interval->roughnessFactor( unitSystem );
        }
    }

    // Return default if no active interval found at this date
    return RimMswCompletionParameters::defaultRoughnessFactor( unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessIntervalCollection::coversFullRange( double startMD, double endMD ) const
{
    if ( isEmpty() ) return false;

    auto intervalList = intervals();
    std::sort( intervalList.begin(),
               intervalList.end(),
               []( const RimDiameterRoughnessInterval* a, const RimDiameterRoughnessInterval* b ) { return a->startMD() < b->startMD(); } );

    double currentPos = startMD;
    for ( auto* interval : intervalList )
    {
        if ( interval->startMD() > currentPos + 1e-6 ) // Allow small gap tolerance
        {
            return false; // Gap found
        }
        currentPos = std::max( currentPos, interval->endMD() );
    }

    return currentPos >= endMD - 1e-6; // Allow small tolerance
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::mergeAdjacentIntervals()
{
    sortIntervalsByMD();

    auto                                       intervalList = intervals();
    std::vector<RimDiameterRoughnessInterval*> mergedIntervals;

    for ( auto* interval : intervalList )
    {
        if ( mergedIntervals.empty() )
        {
            mergedIntervals.push_back( interval );
            continue;
        }

        auto* lastInterval = mergedIntervals.back();

        // Check if intervals are adjacent and have same properties
        if ( std::abs( lastInterval->endMD() - interval->startMD() ) < 1e-6 &&
             std::abs( lastInterval->diameter() - interval->diameter() ) < 1e-6 &&
             std::abs( lastInterval->roughnessFactor() - interval->roughnessFactor() ) < 1e-6 )
        {
            // Merge intervals
            lastInterval->setEndMD( interval->endMD() );
            delete interval;
        }
        else
        {
            mergedIntervals.push_back( interval );
        }
    }

    // Rebuild collection with merged intervals
    this->m_items.clearWithoutDelete();
    for ( auto* interval : mergedIntervals )
    {
        this->m_items.push_back( interval );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                                      QMenu*                     menu,
                                                                      QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewDiameterRoughnessIntervalFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteDiameterRoughnessIntervalFeature";

    menuBuilder.appendToMenu( menu );
}
