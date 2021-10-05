/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cvfVector3.h"

#include "RigFemPart.h"
#include "cvfStructGrid.h"

#include <vector>

namespace cvf
{
class DrawableGeo;
class ScalarMapper;

} // namespace cvf

class RigFemPartScalarDataAccess;
class RimGeoMechPart;

//==================================================================================================
//
//
//
//==================================================================================================
class RivFemPartTriangleToElmMapper : public cvf::Object
{
public:
    size_t triangleCount() const { return m_trianglesToElementIndex.size(); }

    int  elementIndex( size_t triangleIdx ) const { return m_trianglesToElementIndex[triangleIdx]; }
    char elementFace( size_t triangleIdx ) const { return m_trianglesToElmFace[triangleIdx]; }

    // Interface for building the mappings
    std::vector<int>&  triangleToElmIndexMap() { return m_trianglesToElementIndex; }
    std::vector<char>& triangleToElmFaceMap() { return m_trianglesToElmFace; }

private:
    std::vector<int>  m_trianglesToElementIndex;
    std::vector<char> m_trianglesToElmFace;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RivFemPartGeometryGenerator : public cvf::Object
{
public:
    explicit RivFemPartGeometryGenerator( const RigFemPart* femPart, cvf::Vec3d displayOffset );
    ~RivFemPartGeometryGenerator() override;

    // Setup methods

    void setElementVisibility( const cvf::UByteArray* cellVisibility );

    // Access, valid after generation is done

    const RigFemPart* activePart() { return m_femPart.p(); }

    // Generated geometry

    cvf::ref<cvf::DrawableGeo> generateSurface( const std::vector<cvf::Vec3f> nodeCoordinates );
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();
    cvf::ref<cvf::DrawableGeo> createOutlineMeshDrawable( double creaseAngle );

    const std::vector<size_t>& quadVerticesToNodeIdxMapping() const { return m_quadVerticesToNodeIdx; }
    const std::vector<size_t>& quadVerticesToGlobalElmNodeIdx() const { return m_quadVerticesToGlobalElmNodeIdx; }
    const std::vector<size_t>& quadVerticesToGlobalElmFaceNodeIdx() const
    {
        return m_quadVerticesToGlobalElmFaceNodeIdx;
    }
    const std::vector<size_t>& quadVerticesToGlobalElmIdx() const { return m_quadVerticesToGlobalElmIdx; }

    RivFemPartTriangleToElmMapper* triangleToElementMapper() { return m_triangleMapper.p(); }

    static cvf::ref<cvf::DrawableGeo> createMeshDrawableFromSingleElement( const RigFemPart* grid,
                                                                           size_t            elementIndex,
                                                                           const cvf::Vec3d& displayModelOffset );

private:
    void computeArrays( const std::vector<cvf::Vec3f> nodeCoordinates );

private:
    // Input
    cvf::cref<RigFemPart>      m_femPart; // The part being processed
    cvf::cref<cvf::UByteArray> m_elmVisibility;
    cvf::Vec3d                 m_displayOffset;

    // Created arrays
    cvf::ref<cvf::Vec3fArray> m_quadVertices;
    std::vector<size_t>       m_quadVerticesToNodeIdx;
    std::vector<size_t>       m_quadVerticesToGlobalElmNodeIdx;
    std::vector<size_t>       m_quadVerticesToGlobalElmFaceNodeIdx;
    std::vector<size_t>       m_quadVerticesToGlobalElmIdx;

    // Mappings
    cvf::ref<RivFemPartTriangleToElmMapper> m_triangleMapper;
};
