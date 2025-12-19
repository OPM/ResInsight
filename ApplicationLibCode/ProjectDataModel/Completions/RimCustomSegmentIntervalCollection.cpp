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

#include "RimCustomSegmentIntervalCollection.h"

#include "RiaLogging.h"
#include "RimCustomSegmentInterval.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTableViewEditor.h"

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimCustomSegmentIntervalCollection, "CustomSegmentIntervalCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentIntervalCollection::RimCustomSegmentIntervalCollection()
{
    CAF_PDM_InitObject( "Custom Segment Intervals", ":/WellPathComponent16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_intervals, "Intervals", "Intervals" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentIntervalCollection::~RimCustomSegmentIntervalCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCustomSegmentInterval*> RimCustomSegmentIntervalCollection::intervals() const
{
    return m_intervals.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::addInterval( RimCustomSegmentInterval* interval )
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
void RimCustomSegmentIntervalCollection::removeInterval( RimCustomSegmentInterval* interval )
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
void RimCustomSegmentIntervalCollection::removeAllIntervals()
{
    m_intervals.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentInterval* RimCustomSegmentIntervalCollection::createInterval( double startMD, double endMD )
{
    auto* interval = new RimCustomSegmentInterval();
    interval->setStartMD( startMD );
    interval->setEndMD( endMD );

    addInterval( interval );
    return interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentInterval* RimCustomSegmentIntervalCollection::createDefaultInterval()
{
    auto* interval = new RimCustomSegmentInterval();
    addInterval( interval );
    return interval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomSegmentInterval* RimCustomSegmentIntervalCollection::findIntervalAtMD( double md ) const
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
bool RimCustomSegmentIntervalCollection::hasValidIntervals() const
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
bool RimCustomSegmentIntervalCollection::hasOverlappingIntervals() const
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
std::map<QString, QString> RimCustomSegmentIntervalCollection::validate( const QString& configName ) const
{
    // First validate all fields using base implementation
    auto errors = PdmObject::validate( configName );

    QStringList validationIssues;

    // Validate each child interval
    for ( const auto* interval : intervals() )
    {
        if ( interval )
        {
            // Get validation errors from the interval itself
            auto intervalErrors = interval->validate( configName );
            for ( const auto& [field, errorMsg] : intervalErrors )
            {
                validationIssues.append( QString( "Interval (%1): %2." ).arg( interval->generateDisplayLabel() ).arg( errorMsg ) );
            }
        }
    }

    // Check for overlaps between intervals
    for ( size_t i = 0; i < intervals().size(); ++i )
    {
        for ( size_t j = i + 1; j < intervals().size(); ++j )
        {
            RimCustomSegmentInterval* intervalI = intervals()[i];
            RimCustomSegmentInterval* intervalJ = intervals()[j];
            if ( intervalI && intervalJ && intervalI->overlaps( intervalJ ) )
            {
                validationIssues.append( QString( "Interval (%1) overlaps with another interval (%2)." )
                                             .arg( intervalI->generateDisplayLabel() )
                                             .arg( intervalJ->generateDisplayLabel() ) );
            }
        }
    }

    // If there are any validation issues, combine them into a single error message
    if ( !validationIssues.isEmpty() )
    {
        errors["Intervals"] = validationIssues.join( "\n" );
    }

    return errors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::sortIntervalsByMD()
{
    auto intervalList = intervals();
    std::sort( intervalList.begin(),
               intervalList.end(),
               []( const RimCustomSegmentInterval* a, const RimCustomSegmentInterval* b ) { return *a < *b; } );

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
bool RimCustomSegmentIntervalCollection::isEmpty() const
{
    return m_intervals.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimCustomSegmentIntervalCollection::count() const
{
    return m_intervals.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::updateConnectedEditors()
{
    m_intervals.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayField<RimCustomSegmentInterval*>& RimCustomSegmentIntervalCollection::intervalsField()
{
    return m_intervals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::updateOverlapVisualFeedback()
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
void RimCustomSegmentIntervalCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
void RimCustomSegmentIntervalCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_intervals );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
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
void RimCustomSegmentIntervalCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                         std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                                  QMenu*                     menu,
                                                                  QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicNewCustomSegmentIntervalFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteCustomSegmentIntervalFeature";

    menuBuilder.appendToMenu( menu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomSegmentIntervalCollection::insertInterval( RimCustomSegmentInterval* insertBefore, RimCustomSegmentInterval* interval )
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
