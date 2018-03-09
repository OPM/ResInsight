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
#include "cvfVector3.h"

#include "Rim3dWellLogCurve.h"

#include "cafPdmPointer.h"

#include <vector>

namespace cvf
{
    class ModelBasicList;
    class Part;
}

namespace caf
{
    class DisplayCoordTransform;
}

class Riv3dWellLogCurveGeometryGenerator;
class RigWellPath;

class Riv3dWellLogPlanePartMgr : public cvf::Object
{
public:
    Riv3dWellLogPlanePartMgr(RigWellPath* wellPathGeometry);

    void append3dWellLogCurvesToModel(cvf::ModelBasicList* model, 
                                      const caf::DisplayCoordTransform* displayCoordTransform,
                                      std::vector<Rim3dWellLogCurve*>   rim3dWellLogCurves);
private:
    std::vector<cvf::Vec3f> createCurveVertices(const Rim3dWellLogCurve* rim3dWellLogCurve,
                                                const caf::DisplayCoordTransform* displayCoordTransform);
    std::vector<cvf::uint>  createPolylineIndices(size_t vertexCount);

private:
    cvf::ref<Riv3dWellLogCurveGeometryGenerator> m_3dWellLogCurveGeometryGenerator;
    cvf::ref<RigWellPath> m_wellPathGeometry;
};