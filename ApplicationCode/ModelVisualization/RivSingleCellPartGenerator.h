/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfDrawableGeo.h"

namespace cvf
{
    class Part;
}

class RigEclipseCaseData;
class RimGeoMechCase;

//==================================================================================================
///
///
//==================================================================================================
class RivSingleCellPartGenerator 
{
public:
    RivSingleCellPartGenerator(RigEclipseCaseData* rigCaseData, size_t gridIndex, size_t cellIndex);
    RivSingleCellPartGenerator(RimGeoMechCase* rimGeoMechCase, size_t gridIndex, size_t cellIndex);

    cvf::ref<cvf::Part>         createPart(const cvf::Color3f color);

private:
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();

private:
    RigEclipseCaseData*    m_rigCaseData;
    RimGeoMechCase* m_geoMechCase;
    size_t          m_gridIndex;
    size_t          m_cellIndex;
};
