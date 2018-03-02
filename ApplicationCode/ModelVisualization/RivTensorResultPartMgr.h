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
#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"
#include "cafTensor3.h"

#include "RimTensorResults.h"

#include <array>
#include <vector>

namespace cvf
{
class ModelBasicList;
class Part;
class ScalarMapper;
class ScalarMapperDiscreteLinear;
} // namespace cvf

class RigFemResultAddress;
class RimGeoMechView;
class RigFemPartNodes;
class RigFemPart;

class RivTensorResultPartMgr : public cvf::Object
{
public:
    RivTensorResultPartMgr(RimGeoMechView* reservoirView);
    ~RivTensorResultPartMgr();

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex) const;

private:
    struct TensorVisualization
    {
        TensorVisualization(cvf::Vec3f vertex, cvf::Vec3f result, cvf::Vec3f faceNormal, bool isPressure, size_t princial, float principalValue)
            : vertex(vertex)
            , result(result)
            , faceNormal(faceNormal)
            , isPressure(isPressure)
            , princial(princial)
            , principalValue(principalValue) {};

        cvf::Vec3f vertex;
        cvf::Vec3f result;
        cvf::Vec3f faceNormal;
        bool       isPressure;
        size_t     princial;
        float      principalValue;
    };

private:
    static void calculateElementTensors(const RigFemPart&              part,
                                        const std::vector<caf::Ten3f>& vertexTensors,
                                        std::vector<caf::Ten3f>*       elmTensors);

    static void calculatePrincipalsAndDirections(const std::vector<caf::Ten3f>&          tensors,
                                                 std::array<std::vector<float>, 3>*      principals,
                                                 std::vector<std::array<cvf::Vec3f, 3>>* principalDirections);

    static cvf::Vec3f calculateFaceNormal(const RigFemPartNodes&     nodes,
                                          const std::vector<size_t>& quadVerticesToNodeIdxMapping,
                                          int                        quadVertex);

    cvf::ref<cvf::Part> createPart(const std::vector<TensorVisualization>& tensorVisualizations) const;

    static void createOneColorPerPrincipalScalarMapper(const RimTensorResults::TensorColors& colorSet, cvf::ScalarMapperDiscreteLinear* scalarMapper);
    static void createOneColorPerPrincipalTextureCoords(cvf::Vec2fArray*                        textureCoords,
                                                        const std::vector<TensorVisualization>& tensorVisualizations,
                                                        const cvf::ScalarMapper*                mapper);

    static void createResultColorTextureCoords(cvf::Vec2fArray*                        textureCoords,
                                               const std::vector<TensorVisualization>& tensorVisualizations,
                                               const cvf::ScalarMapper*                mapper);

    static bool isTensorAddress(RigFemResultAddress address);
    static bool isValid(cvf::Vec3f resultVector);
    static bool isPressure(float principalValue);
    bool        isDrawable(cvf::Vec3f resultVector, bool showPrincipal) const;

    std::array<cvf::Vec3f, 5>   createArrowVertices(const TensorVisualization &tensorVisualization) const;
    std::array<uint, 8>         createArrowIndices(uint startIndex) const;

private:
    caf::PdmPointer<RimGeoMechView> m_rimReservoirView;
};
