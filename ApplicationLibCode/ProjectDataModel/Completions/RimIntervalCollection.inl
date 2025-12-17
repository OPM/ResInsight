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

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename IntervalType>
RimIntervalCollection<IntervalType>::RimIntervalCollection()
{
    // m_items field must be initialized by derived class using CAF_PDM_InitFieldNoDefault
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename IntervalType>
RimIntervalCollection<IntervalType>::~RimIntervalCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename IntervalType>
IntervalType* RimIntervalCollection<IntervalType>::findIntervalAtMD( double md ) const
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
template <typename IntervalType>
bool RimIntervalCollection<IntervalType>::hasValidIntervals() const
{
    if ( this->isEmpty() ) return false;

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
template <typename IntervalType>
bool RimIntervalCollection<IntervalType>::hasOverlappingIntervals() const
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
template <typename IntervalType>
std::vector<QString> RimIntervalCollection<IntervalType>::validateIntervals() const
{
    std::vector<QString> issues;

    if ( this->isEmpty() )
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
            issues.push_back(
                QString( "Invalid interval: MD %.1f-%.1f (Start MD must be less than End MD)" ).arg( interval->startMD() ).arg( interval->endMD() ) );
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
template <typename IntervalType>
void RimIntervalCollection<IntervalType>::sortIntervalsByMD()
{
    auto intervalList = intervals();
    std::sort( intervalList.begin(), intervalList.end(), []( const IntervalType* a, const IntervalType* b ) { return *a < *b; } );

    // Rebuild the collection in sorted order
    this->m_items.clearWithoutDelete();
    for ( auto* interval : intervalList )
    {
        this->m_items.push_back( interval );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename IntervalType>
void RimIntervalCollection<IntervalType>::updateOverlapVisualFeedback()
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
template <typename IntervalType>
void RimIntervalCollection<IntervalType>::onItemsChanged()
{
    sortIntervalsByMD();
    updateOverlapVisualFeedback();
}

//--------------------------------------------------------------------------------------------------
/// Convenience accessors (delegate to base class)
//--------------------------------------------------------------------------------------------------
template <typename IntervalType>
std::vector<IntervalType*> RimIntervalCollection<IntervalType>::intervals() const
{
    return this->items();
}

template <typename IntervalType>
void RimIntervalCollection<IntervalType>::addInterval( IntervalType* interval )
{
    this->addItem( interval );
}

template <typename IntervalType>
void RimIntervalCollection<IntervalType>::removeInterval( IntervalType* interval )
{
    this->deleteItem( interval );
}

template <typename IntervalType>
void RimIntervalCollection<IntervalType>::removeAllIntervals()
{
    this->deleteAllItems();
}

template <typename IntervalType>
IntervalType* RimIntervalCollection<IntervalType>::createDefaultInterval()
{
    return this->createDefaultItem();
}

template <typename IntervalType>
caf::PdmChildArrayField<IntervalType*>& RimIntervalCollection<IntervalType>::intervalsField()
{
    return this->itemsField();
}

template <typename IntervalType>
const caf::PdmChildArrayField<IntervalType*>& RimIntervalCollection<IntervalType>::intervalsField() const
{
    return this->itemsField();
}
