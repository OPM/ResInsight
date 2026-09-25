/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cvfVector3.h"

#include <vector>

class RimContourMapTop;

//==================================================================================================
/// Owns the set of detected "tops" (local maxima) for a single contour map projection.
///
/// Each top is stored as a RimContourMapTop child, exposing its rank/value/prominence/position for
/// inspection in the project tree and property panel, and it owns (by reference) a small marker
/// polygon used for 3d visualization. See RimContourMapTop for details.
//==================================================================================================
class RimContourMapTopsCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimContourMapTopsCollection();

    RimContourMapTop* addTop( int rank, double value, double prominence, const cvf::Vec3d& domainPosition, double markerSize );
    void              clearTops();

    // Refresh the 3d views mirroring the polygon collection, so newly added/removed marker polygons
    // become visible. Call once after a batch of addTop()/clearTops() calls.
    void updateVisualization();

    std::vector<RimContourMapTop*> tops() const;

private:
    caf::PdmChildArrayField<RimContourMapTop*> m_tops;
};
