/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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
#include "RimWellPathComponentInterface.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
/// Represents a Multi-Segment Well (MSW) segment from WELSEGS keyword data
/// Used for 3D visualization of well segments along the well path
///
//==================================================================================================
class RimMswSegment : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimMswSegment();
    ~RimMswSegment() override;

    void setSegmentData( int segmentNumber, int branchNumber, double startMD, double endMD, double diameter );

    int    segmentNumber() const;
    int    branchNumber() const;
    double diameter() const;

    // RimWellPathComponentInterface overrides
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    void                              applyOffset( double offsetMD ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<int>    m_segmentNumber;
    caf::PdmField<int>    m_branchNumber;
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_diameter;
};
