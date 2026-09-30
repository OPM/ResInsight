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
/// Plain, non-user-editable organizational folder for the tree beneath a RimPolygonCloudSource
/// (e.g. "Field Outline", "Fluid Contact Outline" + per-name folders). Only
/// RimPolygonCloudSource::buildDirectoryTree() creates/populates these.
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
