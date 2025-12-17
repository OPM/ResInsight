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
#include "RimDiameterRoughnessInterval.h"
#include "RimIntervalCollection.h"

//==================================================================================================
///
/// Collection to manage diameter and roughness intervals for measured depth ranges
///
//==================================================================================================
class RimDiameterRoughnessIntervalCollection : public RimIntervalCollection<RimDiameterRoughnessInterval>
{
    CAF_PDM_HEADER_INIT;

public:
    RimDiameterRoughnessIntervalCollection();
    ~RimDiameterRoughnessIntervalCollection() override;

    // Domain-specific interval creation
    RimDiameterRoughnessInterval* createInterval( double startMD, double endMD, double diameter, double roughness );

    // Domain-specific lookup methods
    double getDiameterAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;
    double getRoughnessAtMD( double md, RiaDefines::EclipseUnitSystem unitSystem ) const;

    // Domain-specific validation
    bool coversFullRange( double startMD, double endMD ) const;

    // Domain-specific organization
    void mergeAdjacentIntervals();

protected:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
};
