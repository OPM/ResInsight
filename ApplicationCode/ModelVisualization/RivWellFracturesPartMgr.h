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
}
namespace caf
{
    class DisplayCoordTransform;
}


class RimEclipseWell;
class RimFracture;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RivWellFracturesPartMgr : public cvf::Object
{
public:
    RivWellFracturesPartMgr(RimEclipseWell* well);
    ~RivWellFracturesPartMgr();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);

    static void appendFracturePartsToModel(std::vector<RimFracture*> fractures, cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform);

private:
    static cvf::ref<cvf::DrawableGeo> createGeo(const std::vector<cvf::uint>& polygonIndices, const std::vector<cvf::Vec3f>& nodeCoords);

private:
    caf::PdmPointer<RimEclipseWell> m_rimWell;
};
