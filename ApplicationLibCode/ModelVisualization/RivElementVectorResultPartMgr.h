/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cvfArray.h"
#include "cvfObject.h"
#include "cvfTransform.h"
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

class RimEclipseView;
class RimElementVectorResult;

class RivElementVectorResultPartMgr : public cvf::Object
{
public:
    RivElementVectorResultPartMgr( RimEclipseView* reservoirView );
    ~RivElementVectorResultPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t timeStepIndex );
    void setTransform( cvf::Transform* scaleTransform );

private:
    struct ElementVectorResultVisualization
    {
        ElementVectorResultVisualization( cvf::Vec3d faceCenter, cvf::Vec3d faceNormal, double result, double approximateCellLength )
            : faceCenter( faceCenter )
            , faceNormal( faceNormal )
            , result( result )
            , approximateCellLength( approximateCellLength )
        {
        }

        cvf::Vec3f faceCenter;
        cvf::Vec3f faceNormal;
        double     result;
        double     approximateCellLength;
    };

private:
    cvf::ref<cvf::Part> createPart( const RimElementVectorResult&                        result,
                                    const std::vector<ElementVectorResultVisualization>& tensorVisualizations ) const;

    static void
        createResultColorTextureCoords( cvf::Vec2fArray*                                     textureCoords,
                                        const std::vector<ElementVectorResultVisualization>& elementVectorResultVisualizations,
                                        const cvf::ScalarMapper*                             mapper );

    std::array<cvf::Vec3f, 7> createArrowVertices( const ElementVectorResultVisualization& tensorVisualization ) const;
    std::array<uint, 2>       createArrowShaftIndices( uint startIndex ) const;
    std::array<uint, 6>       createArrowHeadIndices( uint startIndex ) const;

    double scaleLogarithmically( double value ) const;

private:
    caf::PdmPointer<RimEclipseView> m_rimReservoirView;
    cvf::ref<cvf::Transform>        m_scaleTransform;
};
