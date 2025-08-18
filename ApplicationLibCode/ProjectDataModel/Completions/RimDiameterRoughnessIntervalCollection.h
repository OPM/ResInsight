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

class RimDiameterRoughnessInterval;

//==================================================================================================
///
/// Collection to manage diameter and roughness intervals for measured depth ranges
///
//==================================================================================================
class RimDiameterRoughnessIntervalCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDiameterRoughnessIntervalCollection();
    ~RimDiameterRoughnessIntervalCollection() override;

    // Collection management
    std::vector<RimDiameterRoughnessInterval*> intervals() const;
    void                                       addInterval( RimDiameterRoughnessInterval* interval );
    void insertInterval( RimDiameterRoughnessInterval* insertBefore, RimDiameterRoughnessInterval* interval );
    void removeInterval( RimDiameterRoughnessInterval* interval );
    void removeAllIntervals();

    // Interval creation
    RimDiameterRoughnessInterval* createInterval( double startMD, double endMD, double diameter, double roughness );
    RimDiameterRoughnessInterval* createDefaultInterval();

    // Lookup methods
    double                        getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;
    double                        getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;
    RimDiameterRoughnessInterval* findIntervalAtMD( double md ) const;

    // Validation
    bool                 hasValidIntervals() const;
    bool                 hasOverlappingIntervals() const;
    bool                 coversFullRange( double startMD, double endMD ) const;
    std::vector<QString> validateIntervals() const;

    // Sorting and organization
    void sortIntervalsByMD();
    void mergeAdjacentIntervals();

    // Utility
    bool   isEmpty() const;
    size_t count() const;
    void   updateConnectedEditors();

    // Accessor for UI integration
    caf::PdmChildArrayField<RimDiameterRoughnessInterval*>& intervalsField();

    // Visual feedback for overlapping intervals
    void updateOverlapVisualFeedback();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    caf::PdmChildArrayField<RimDiameterRoughnessInterval*> m_intervals;
};
