/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "cvfStructGridGeometryGenerator.h"
#include "cvfCellRange.h"

namespace cvf
{
    class StructGridInterface;
    class ModelBasicList;
    class Transform;
    class Part;
}

class RigFault;


//==================================================================================================
///
///
//==================================================================================================

class RivFaultGeometryGenerator : public cvf::Object
{
public:
    RivFaultGeometryGenerator(const cvf::StructGridInterface* grid, const RigFault* fault);
    ~RivFaultGeometryGenerator();

    void setCellVisibility(const cvf::UByteArray* cellVisibilities );
    
    void setShowNativeFaultFaces(bool showNativeFaultFaces);
    void setShowOppositeFaultFaces(bool showOppositeFaultFaces);
    
    void textureCoordinates(cvf::Vec2fArray* textureCoords, 
        const cvf::StructGridScalarDataAccess* dataAccessObject, 
        const cvf::ScalarMapper* mapper) const;

    // Mapping between cells and geometry
    cvf::ref<cvf::Array<size_t> >   triangleToSourceGridCellMap() const;

    const std::vector<size_t>&      quadToGridCellIndices() const;
    const std::vector<cvf::StructGridInterface::FaceType>& quadToFace() const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>    generateSurface();
    cvf::ref<cvf::DrawableGeo>    createMeshDrawable();
    cvf::ref<cvf::DrawableGeo>    createOutlineMeshDrawable(double creaseAngle);

private:
    static cvf::ref<cvf::UIntArray> lineIndicesFromQuadVertexArray(const cvf::Vec3fArray* vertexArray);

    void computeArrays();

private:
    // Input
    cvf::cref<cvf::StructGridInterface> m_grid;
    cvf::cref<RigFault>                 m_fault;
    cvf::cref<cvf::UByteArray>          m_cellVisibility;
    
    bool                                m_showNativeFaultFaces;
    bool                                m_showOppositeFaultFaces;

    // Created arrays
    cvf::ref<cvf::Vec3fArray>           m_vertices;
    
    // Mappings
    std::vector<size_t>                 m_triangleIndexToGridCellIndex;
    std::vector<size_t>                 m_quadsToGridCells;
    std::vector<cvf::StructGridInterface::FaceType>   m_quadsToFace;
};
