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

#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <vector>

class RimCustomSegmentInterval;

//==================================================================================================
///
/// Collection to manage custom segment intervals for measured depth ranges
///
//==================================================================================================
class RimCustomSegmentIntervalCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCustomSegmentIntervalCollection();
    ~RimCustomSegmentIntervalCollection() override;

    // Collection management
    std::vector<RimCustomSegmentInterval*> intervals() const;
    void                                   addInterval( RimCustomSegmentInterval* interval );
    void                                   insertInterval( RimCustomSegmentInterval* insertBefore, RimCustomSegmentInterval* interval );
    void                                   removeInterval( RimCustomSegmentInterval* interval );
    void                                   removeAllIntervals();

    // Interval creation
    RimCustomSegmentInterval* createInterval( double startMD, double endMD );
    RimCustomSegmentInterval* createDefaultInterval();

    // Lookup methods
    RimCustomSegmentInterval* findIntervalAtMD( double md ) const;

    // Validation
    std::map<QString, QString> validate( const QString& configName = "" ) const override;
    bool                       hasValidIntervals() const;
    bool                       hasOverlappingIntervals() const;

    // Sorting and organization
    void sortIntervalsByMD();

    // Utility
    bool   isEmpty() const;
    size_t count() const;
    void   updateConnectedEditors();

    // Accessor for UI integration
    caf::PdmChildArrayField<RimCustomSegmentInterval*>& intervalsField();

    // Visual feedback for overlapping intervals
    void updateOverlapVisualFeedback();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    caf::PdmChildArrayField<RimCustomSegmentInterval*> m_intervals;
};
