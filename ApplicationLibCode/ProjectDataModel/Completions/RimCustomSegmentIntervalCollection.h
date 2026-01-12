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

#include "RimCustomSegmentInterval.h"
#include "RimIntervalCollection.h"

//==================================================================================================
///
/// Collection to manage custom segment intervals for measured depth ranges
///
//==================================================================================================
class RimCustomSegmentIntervalCollection : public RimIntervalCollection<RimCustomSegmentInterval>
{
    CAF_PDM_HEADER_INIT;

public:
    RimCustomSegmentIntervalCollection();
    ~RimCustomSegmentIntervalCollection() override;

    // Domain-specific interval creation
    RimCustomSegmentInterval* createInterval( double startMD, double endMD );

    // Validation
    std::map<QString, QString> validate( const QString& configName = "" ) const override;

protected:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
};
