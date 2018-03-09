/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
#include "cvfObject.h"
#include "cvfDrawableGeo.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <vector>

namespace caf
{
    class DisplayCoordTransform;
}

class Riv3dWellLogCurveGeometryGenerator : public cvf::Object
{
public:
    Riv3dWellLogCurveGeometryGenerator() = default;

    cvf::ref<cvf::DrawableGeo> createDrawable(const std::vector<cvf::Vec3f>& vertices, const std::vector<cvf::uint>& indices ) const;
};
