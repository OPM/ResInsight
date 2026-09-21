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

//==================================================================================================
///
/// A plain, non-user-editable organizational folder used to build the browsable tree beneath a
/// RimPolygonCloudSource (e.g. "Field Outline", "Structure Depth Fault Lines", "Fluid Contact
/// Outline", and per-name folders under fluid contact outline). Only RimPolygonCloudSource::
/// buildDirectoryTree() creates/populates these -- the user cannot add/remove sub-collections
/// here via the UI (canAddSubCollection() is false).
///
//==================================================================================================
class RimPolygonCloudFolder : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCloudFolder();
    explicit RimPolygonCloudFolder( const QString& folderName );

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;
};
