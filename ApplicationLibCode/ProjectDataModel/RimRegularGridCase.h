/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimEclipseResultCase.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cvfBoundingBox.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RimRegularGridCase : public RimEclipseResultCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimRegularGridCase();

    void setBoundingBox( const cvf::BoundingBox& boundingBox );

    bool openEclipseGridFile() override;

    cvf::ref<RifReaderInterface> createModel( QString modelName );

private:
    caf::PdmField<cvf::Vec3d> m_minimum;
    caf::PdmField<cvf::Vec3d> m_maximum;
};
