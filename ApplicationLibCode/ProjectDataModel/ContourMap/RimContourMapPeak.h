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

#include "RimNamedObject.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfVector3.h"

class RimPolygon;

//==================================================================================================
/// A single computed "peak" (local maximum) in a contour map, as produced by RigContourMapPeakFinder.
///
/// Stores the result value, prominence and rank for inspection/sorting in the project tree, and
/// references a small marker RimPolygon (owned by the global polygon collection, so it can be
/// mirrored and rendered by the existing polygon-in-view machinery) used to visualize the peak's
/// position in 3d views. The marker polygon is deleted together with this object, unless it has been
/// detached with releaseMarkerPolygon().
//==================================================================================================
class RimContourMapPeak : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimContourMapPeak();
    ~RimContourMapPeak() override;

    void setValues( int rank, double value, double prominence, const cvf::Vec3d& domainPosition );
    void setMarkerPolygon( RimPolygon* markerPolygon );

    // Stop referencing the marker polygon without deleting it
    void releaseMarkerPolygon();

    int         rank() const;
    double      value() const;
    double      prominence() const;
    cvf::Vec3d  position() const;
    RimPolygon* markerPolygon() const;

private:
    void deleteMarkerPolygon();

private:
    caf::PdmField<int>            m_rank;
    caf::PdmField<double>         m_value;
    caf::PdmField<double>         m_prominence;
    caf::PdmField<cvf::Vec3d>     m_position;
    caf::PdmPtrField<RimPolygon*> m_markerPolygon;
};
