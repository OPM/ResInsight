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

#pragma once

#include "cafPdmObjectCollection.h"

#include <QString>
#include <vector>

//==================================================================================================
///
/// Interval-specific templated collection base class
///
/// Extends RimPdmObjectCollection with interval-specific operations for objects that have
/// measured depth (MD) ranges. Provides sorting, overlap detection, and validation.
///
/// Template parameter:
/// - IntervalType: Must inherit from caf::PdmObject and provide interval methods
///
/// IntervalType must provide: startMD(), endMD(), containsMD(double), overlaps(const IntervalType*),
/// isValidInterval(), updateOverlapVisualFeedback(bool), operator<
///
/// Derived classes must call the base constructor with field name and display name.
///
//==================================================================================================
template <typename IntervalType>
class RimIntervalCollection : public caf::PdmObjectCollection<IntervalType>
{
public:
    // Interval-specific lookup
    IntervalType* findIntervalAtMD( double md ) const;

    // Validation
    bool                 hasValidIntervals() const;
    bool                 hasOverlappingIntervals() const;
    std::vector<QString> validateIntervals() const;

    // Organization
    void sortIntervalsByMD();
    void updateOverlapVisualFeedback();

    // Convenience accessors (delegate to base class)
    std::vector<IntervalType*>                    intervals() const;
    void                                          addInterval( IntervalType* interval );
    void                                          removeInterval( IntervalType* interval );
    void                                          removeAllIntervals();
    IntervalType*                                 createDefaultInterval();
    caf::PdmChildArrayField<IntervalType*>&       intervalsField();
    const caf::PdmChildArrayField<IntervalType*>& intervalsField() const;

protected:
    // Constructor (protected - only derived classes can instantiate)
    // Derived classes MUST initialize m_items field using CAF_PDM_InitFieldNoDefault
    RimIntervalCollection();
    ~RimIntervalCollection() override;

    // Override to automatically sort and update visual feedback when items change
    void onItemsChanged() override;
};

// Include template implementation
#include "RimIntervalCollection.inl"
