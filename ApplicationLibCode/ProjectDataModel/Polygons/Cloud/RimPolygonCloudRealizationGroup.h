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

#include "Polygons/RimPolygonContainer.h"

#include "cafPdmField.h"

//==================================================================================================
///
/// Holds the RimCloudPolygon children fetched for one realization other than its owning
/// RimPolygonCloudAddress's own base realization. Created on demand -- as a genuine, visible child
/// of that address, in the global RimPolygonCollection tree -- when a 3D view with "Auto-Follow
/// View Realization" enabled resolves a realization other than the address's base one (see
/// RimPolygonInViewCollection). Named "Real <n>". This is what lets a user compare a polygon
/// against its "base" realization: the base is always shown via the address's own items(), and one
/// (or more) of these groups can appear alongside it, one per additional realization some view is
/// currently following.
///
/// Deleted (and its RimCloudPolygon children with it) once no view still shows it checked -- see
/// RimPolygonCloudAddress::evictRealizationGroup().
///
//==================================================================================================
class RimPolygonCloudRealizationGroup : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCloudRealizationGroup();
    explicit RimPolygonCloudRealizationGroup( int realization );

    int realization() const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

private:
    caf::PdmField<int> m_realization;
};
