/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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
#include "cvfMatrix4.h"

#include "cafPdmPointer.h"

#include <vector>

namespace cvf
{
class Part;
class ModelBasicList;
class ScalarMapper;
} // namespace cvf

class RimWellPath;
class RimVirtualPerforationResults;

namespace caf
{
class DisplayCoordTransform;
}

struct CompletionVizData
{
    CompletionVizData(cvf::Vec3d anchor, cvf::Vec3d direction, double connectionFactor)
        : m_anchor(anchor)
        , m_direction(direction)
        , m_connectionFactor(connectionFactor)
    {
    }

    cvf::Vec3d m_anchor;
    cvf::Vec3d m_direction;
    double     m_connectionFactor;
};


//--------------------------------------------------------------------------------------------------
///
/// Based on RivWellSpheresPartMgr
///
//--------------------------------------------------------------------------------------------------
class RivVirtualConnFactorPartMgr : public cvf::Object
{
public:
    RivVirtualConnFactorPartMgr(RimWellPath* well, RimVirtualPerforationResults* virtualPerforationResult);
    ~RivVirtualConnFactorPartMgr();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);

private:
    static cvf::ref<cvf::Part> createPart(std::vector<CompletionVizData>& centerColorPairs, 
                                          double radius, 
                                          cvf::ScalarMapper* scalarMapper);

    static void createStarGeometry(std::vector<cvf::Vec3f>* vertices, std::vector<cvf::uint>* indices, double radius, double thickness);

    static cvf::Mat4f rotationMatrixBetweenVectors(const cvf::Vec3d& v1, const cvf::Vec3d& v2);

private:
    caf::PdmPointer<RimWellPath>                  m_rimWell;
    caf::PdmPointer<RimVirtualPerforationResults> m_virtualPerforationResult;
};
