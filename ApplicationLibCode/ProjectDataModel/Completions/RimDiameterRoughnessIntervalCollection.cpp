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

#include "RiaLogging.h"
#include "RimDiameterRoughnessInterval.h"
#include "RimMswCompletionParameters.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimDiameterRoughnessIntervalCollection, "DiameterRoughnessIntervalCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDiameterRoughnessIntervalCollection::RimDiameterRoughnessIntervalCollection()
{
    CAF_PDM_InitObject( "Diameter Roughness Intervals", ":/WellPathComponent16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_intervals, "Intervals", "Intervals" );
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
std::vector<RimDiameterRoughnessInterval*> RimDiameterRoughnessIntervalCollection::intervals() const
{
    return m_intervals.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::addInterval( RimDiameterRoughnessInterval* interval )
{
    if ( interval )
    {
        m_intervals.push_back( interval );
        sortIntervalsByMD();
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::removeInterval( RimDiameterRoughnessInterval* interval )
{
    if ( interval )
    {
        m_intervals.removeChild( interval );
        delete interval;
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::removeAllIntervals()
{
    m_intervals.deleteChildren();
    updateConnectedEditors();
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
RimDiameterRoughnessInterval* RimDiameterRoughnessIntervalCollection::createDefaultInterval()
{
    auto* interval = new RimDiameterRoughnessInterval();
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
RimDiameterRoughnessInterval* RimDiameterRoughnessIntervalCollection::findIntervalAtMD( double md ) const
{
    for ( auto* interval : intervals() )
    {
        if ( interval && interval->containsMD( md ) )
        {
            return interval;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessIntervalCollection::hasValidIntervals() const
{
    if ( isEmpty() ) return false;

    for ( auto* interval : intervals() )
    {
        if ( !interval || !interval->isValidInterval() )
        {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessIntervalCollection::hasOverlappingIntervals() const
{
    auto intervalList = intervals();

    for ( size_t i = 0; i < intervalList.size(); ++i )
    {
        for ( size_t j = i + 1; j < intervalList.size(); ++j )
        {
            if ( intervalList[i]->overlaps( intervalList[j] ) )
            {
                return true;
            }
        }
    }
    return false;
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
std::vector<QString> RimDiameterRoughnessIntervalCollection::validateIntervals() const
{
    std::vector<QString> issues;

    if ( isEmpty() )
    {
        issues.push_back( "No intervals defined" );
        return issues;
    }

    auto intervalList = intervals();

    // Check for invalid intervals
    for ( auto* interval : intervalList )
    {
        if ( !interval->isValidInterval() )
        {
            issues.push_back( QString( "Invalid interval: MD %.1f-%.1f" ).arg( interval->startMD() ).arg( interval->endMD() ) );
        }
    }

    // Check for overlaps
    for ( size_t i = 0; i < intervalList.size(); ++i )
    {
        for ( size_t j = i + 1; j < intervalList.size(); ++j )
        {
            if ( intervalList[i]->overlaps( intervalList[j] ) )
            {
                issues.push_back( QString( "Overlapping intervals: MD %.1f-%.1f and MD %.1f-%.1f" )
                                      .arg( intervalList[i]->startMD() )
                                      .arg( intervalList[i]->endMD() )
                                      .arg( intervalList[j]->startMD() )
                                      .arg( intervalList[j]->endMD() ) );
            }
        }
    }

    return issues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::sortIntervalsByMD()
{
    auto intervalList = intervals();
    std::sort( intervalList.begin(),
               intervalList.end(),
               []( const RimDiameterRoughnessInterval* a, const RimDiameterRoughnessInterval* b ) { return *a < *b; } );

    // Rebuild the collection in sorted order
    m_intervals.clearWithoutDelete();
    for ( auto* interval : intervalList )
    {
        m_intervals.push_back( interval );
    }
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
    m_intervals.clearWithoutDelete();
    for ( auto* interval : mergedIntervals )
    {
        m_intervals.push_back( interval );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDiameterRoughnessIntervalCollection::isEmpty() const
{
    return m_intervals.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimDiameterRoughnessIntervalCollection::count() const
{
    return m_intervals.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::updateConnectedEditors()
{
    m_intervals.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayField<RimDiameterRoughnessInterval*>& RimDiameterRoughnessIntervalCollection::intervalsField()
{
    return m_intervals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::updateOverlapVisualFeedback()
{
    auto intervalList = intervals();

    // First, reset all intervals to no overlap
    for ( auto* interval : intervalList )
    {
        interval->updateOverlapVisualFeedback( false );
    }

    // Then check for overlaps and update visual feedback
    for ( size_t i = 0; i < intervalList.size(); ++i )
    {
        bool hasOverlap = false;

        for ( size_t j = 0; j < intervalList.size(); ++j )
        {
            if ( i != j && intervalList[i]->overlaps( intervalList[j] ) )
            {
                hasOverlap = true;
                break;
            }
        }

        if ( hasOverlap )
        {
            intervalList[i]->updateOverlapVisualFeedback( true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                               const QVariant&            oldValue,
                                                               const QVariant&            newValue )
{
    if ( changedField == &m_intervals )
    {
        sortIntervalsByMD();
        updateOverlapVisualFeedback();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_intervals );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                                    QString                    uiConfigName,
                                                                    caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_intervals )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                             std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDiameterRoughnessIntervalCollection::insertInterval( RimDiameterRoughnessInterval* insertBefore, RimDiameterRoughnessInterval* interval )
{
    if ( !interval ) return;

    size_t index = m_intervals.indexOf( insertBefore );
    if ( index < m_intervals.size() )
        m_intervals.insert( index, interval );
    else
        m_intervals.push_back( interval );

    sortIntervalsByMD();
    updateConnectedEditors();
}
