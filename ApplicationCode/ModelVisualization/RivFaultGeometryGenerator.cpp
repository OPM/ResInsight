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

#include "RivFaultGeometryGenerator.h"

#include <cmath>

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfStructGridScalarDataAccess.h"
#include "cvfScalarMapper.h"

#include "RigFault.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFaultGeometryGenerator::RivFaultGeometryGenerator(const cvf::StructGridInterface* grid, const RigFault* fault)
   : m_grid(grid),
   m_fault(fault)
{
    m_showNativeFaultFaces = true;
    m_showOppositeFaultFaces = true;
    m_limitFaultsToFilters = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFaultGeometryGenerator::~RivFaultGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT(m_vertices.notNull());

    if (m_vertices->size() == 0) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromQuadVertexArray(m_vertices.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// Must call generateSurface first 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::createMeshDrawable()
{

    if (!(m_vertices.notNull() && m_vertices->size() != 0)) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(m_vertices.p());

    cvf::ref<cvf::UIntArray> indices = lineIndicesFromQuadVertexArray(m_vertices.p());
    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(indices.p());

    geo->addPrimitiveSet(prim.p());
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::createOutlineMeshDrawable(double creaseAngle)
{
    if (!(m_vertices.notNull() && m_vertices->size() != 0)) return NULL;

    cvf::OutlineEdgeExtractor ee(creaseAngle, *m_vertices);

    cvf::ref<cvf::UIntArray> indices = lineIndicesFromQuadVertexArray(m_vertices.p());
    ee.addPrimitives(4, *indices);

    cvf::ref<cvf::UIntArray> lineIndices = ee.lineIndices();
    if (lineIndices->size() == 0)
    {
        return NULL;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(lineIndices.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(m_vertices.p());
    geo->addPrimitiveSet(prim.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UIntArray> RivFaultGeometryGenerator::lineIndicesFromQuadVertexArray(const cvf::Vec3fArray* vertexArray)
{
    CVF_ASSERT(vertexArray);

    size_t numVertices = vertexArray->size();
    int numQuads = static_cast<int>(numVertices/4);
    CVF_ASSERT(numVertices%4 == 0);

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->resize(numQuads*8);

#pragma omp parallel for
    for (int i = 0; i < numQuads; i++)
    {        
        int idx = 8*i;
        indices->set(idx + 0, i*4 + 0);
        indices->set(idx + 1, i*4 + 1);
        indices->set(idx + 2, i*4 + 1);
        indices->set(idx + 3, i*4 + 2);
        indices->set(idx + 4, i*4 + 2);
        indices->set(idx + 5, i*4 + 3);
        indices->set(idx + 6, i*4 + 3);
        indices->set(idx + 7, i*4 + 0);
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::computeArrays()
{
    std::vector<cvf::Vec3f> vertices;
    m_quadsToGridCells.clear();
    m_quadsToFace.clear();

    cvf::Vec3d offset = m_grid->displayModelOffset();

    const std::vector<RigFault::FaultFace>& faultFaces = m_fault->faultFaces();

    if (m_showNativeFaultFaces)
    {
#pragma omp parallel for
        for (int fIdx = 0; fIdx < faultFaces.size(); fIdx++)
        {
            size_t cellIndex = faultFaces[fIdx].m_nativeGlobalCellIndex;

            if (m_limitFaultsToFilters)
            {
                if (!(*m_cellVisibility)[cellIndex]) continue;
            }

            cvf::StructGridInterface::FaceType face = faultFaces[fIdx].m_nativeFace;

            cvf::Vec3d cornerVerts[8];
            m_grid->cellCornerVertices(cellIndex, cornerVerts);

            cvf::ubyte faceConn[4];
            m_grid->cellFaceVertexIndices(face, faceConn);

            // Critical section to avoid two threads accessing the arrays at the same time.
    #pragma omp critical
            {
                int n;
                for (n = 0; n < 4; n++)
                {
                    vertices.push_back(cvf::Vec3f(cornerVerts[faceConn[n]] - offset));
                }

                // Keep track of the source cell index per quad
                m_quadsToGridCells.push_back(cellIndex);
                m_quadsToFace.push_back(face);
            }
        }
    }

    if (m_showOppositeFaultFaces)
    {
#pragma omp parallel for
        for (int fIdx = 0; fIdx < faultFaces.size(); fIdx++)
        {
            size_t cellIndex = faultFaces[fIdx].m_oppositeGlobalCellIndex;
            cvf::StructGridInterface::FaceType face = faultFaces[fIdx].m_oppositeFace;

            /*
            size_t i, j, k, ni, nj, nk;
            m_grid->ijkFromCellIndex(currentCellIndex, &i, &j, &k);
            m_grid->neighborIJKAtCellFace(i, j, k, currentFace, &ni, &nj, &nk);

            size_t cellIndex = m_grid->cellIndexFromIJK(ni, nj, nk);
            */
            if (m_limitFaultsToFilters)
            {
                if (!(*m_cellVisibility)[cellIndex]) continue;
            }

            cvf::Vec3d cornerVerts[8];
            m_grid->cellCornerVertices(cellIndex, cornerVerts);

            cvf::ubyte faceConn[4];
            m_grid->cellFaceVertexIndices(face, faceConn);

            // Critical section to avoid two threads accessing the arrays at the same time.
#pragma omp critical
            {
                int n;
                for (n = 0; n < 4; n++)
                {
                    vertices.push_back(cvf::Vec3f(cornerVerts[faceConn[n]] - offset));
                }

                // Keep track of the source cell index per quad
                m_quadsToGridCells.push_back(cellIndex);
                m_quadsToFace.push_back(face);
            }
        }
    }

    m_vertices = new cvf::Vec3fArray;
    m_vertices->assign(vertices);
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture. 
/// Undefined values are coded with a y-texture coordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::textureCoordinates(cvf::Vec2fArray* textureCoords, const cvf::StructGridScalarDataAccess* dataAccessObject, const cvf::ScalarMapper* mapper) const
{
    if (!dataAccessObject) return;

    size_t numVertices = m_quadsToGridCells.size()*4;

    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double cellScalarValue;
    cvf::Vec2f texCoord;

#pragma omp parallel for private(texCoord, cellScalarValue)
    for (int i = 0; i < static_cast<int>(m_quadsToGridCells.size()); i++)
    {
        cellScalarValue = dataAccessObject->cellScalar(m_quadsToGridCells[i]);
        texCoord = mapper->mapToTextureCoord(cellScalarValue);
        if (cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[i*4 + j] = texCoord;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Array<size_t> > RivFaultGeometryGenerator::triangleToSourceGridCellMap() const
{
    cvf::ref<cvf::Array<size_t> > triangles = new cvf::Array<size_t>(2*m_quadsToGridCells.size());
#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(m_quadsToGridCells.size()); i++)
    {
        triangles->set(i*2,   m_quadsToGridCells[i]);
        triangles->set(i*2+1, m_quadsToGridCells[i]);
    }

    return triangles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::setCellVisibility(const cvf::UByteArray* cellVisibility)
{
    m_cellVisibility = cellVisibility;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivFaultGeometryGenerator::quadToGridCellIndices() const
{
    return m_quadsToGridCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::StructGridInterface::FaceType>& RivFaultGeometryGenerator::quadToFace() const
{
    return m_quadsToFace;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::setShowNativeFaultFaces(bool showNativeFaultFaces)
{
    m_showNativeFaultFaces = showNativeFaultFaces;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::setShowOppositeFaultFaces(bool showOppositeFaultFaces)
{
    m_showOppositeFaultFaces = showOppositeFaultFaces;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFaultGeometryGenerator::setLimitFaultsToFilter(bool limitFaultsToFilter)
{
//    m_limitFaultsToFilters = limitFaultsToFilter;
}

