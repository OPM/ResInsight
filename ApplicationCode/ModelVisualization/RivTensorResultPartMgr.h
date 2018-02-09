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
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <array>
#include <vector>

namespace cvf
{
class Part;
class ModelBasicList;
} 

class RigFemResultAddress;
class RimGeoMechView;

class RivTensorResultPartMgr : public cvf::Object
{
public:
    RivTensorResultPartMgr(RimGeoMechView* reservoirView);
    ~RivTensorResultPartMgr();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex) const;

private:

    struct TensorVisualization
    {
        TensorVisualization(cvf::Vec3f vertex, cvf::Vec3f result, cvf::Color3f color, bool isPressure)
            : vertex(vertex), result(result), color(color), isPressure(isPressure) {};

        cvf::Vec3f   vertex;
        cvf::Vec3f   result;
        cvf::Color3f color;
        bool         isPressure;
    };
private:
    cvf::ref<cvf::Part> createPart(std::vector<TensorVisualization>& tensorVisualizations) const;

    static bool isTensorAddress(RigFemResultAddress address);
    static bool isValid(cvf::Vec3f resultVector);

private:
    caf::PdmPointer<RimGeoMechView> m_rimReservoirView;
};
