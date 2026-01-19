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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimEclipseCase;
class RimMswSegment;
struct WelsegsRow;

//==================================================================================================
///
/// Collection to manage MSW segments from WELSEGS keyword data for 3D visualization
///
//==================================================================================================
class RimMswSegmentCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimMswSegmentCollection();

    bool                              hasSegments() const;
    std::vector<const RimMswSegment*> segments() const;
    void                              clearSegments();

    double referenceDiameter() const;

    void updateSegments( RimEclipseCase* eclipseCase );

private:
    void populateFromWelsegsData( std::vector<WelsegsRow> welsegsData, double wellTotalDepth );
    void appendSegment( RimMswSegment* segment );

private:
    caf::PdmChildArrayField<RimMswSegment*> m_segments;
};
