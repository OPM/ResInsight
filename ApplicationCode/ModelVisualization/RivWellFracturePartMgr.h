/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <vector>

namespace cvf
{
     class ModelBasicList;
     class DrawableGeo;
     class Part;
}

namespace caf
{
    class DisplayCoordTransform;
}


class RimFracture;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivWellFracturePartMgr : public cvf::Object
{
public:
    RivWellFracturePartMgr(RimFracture* well);
    ~RivWellFracturePartMgr();

    void appendGeometryPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform);
    void clearGeometryCache();

private:
    void updatePartGeometry(caf::DisplayCoordTransform* displayCoordTransform);

    static cvf::ref<cvf::DrawableGeo> createGeo(const std::vector<cvf::uint>& polygonIndices, const std::vector<cvf::Vec3f>& nodeCoords);

private:
    caf::PdmPointer<RimFracture> m_rimFracture;
    cvf::ref<cvf::Part>          m_part;
};
