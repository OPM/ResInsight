//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfStructGridGeometry.h"
#include "cvfRectilinearGrid.h"
#include "cvfDebugTimer.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::StructGridGeometry
/// \ingroup StructGrid
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
StructGridGeometry::StructGridGeometry(const RectilinearGrid* grid)
:   m_grid(grid),
    m_cellMinI(UNDEFINED_UINT),
    m_cellMinJ(UNDEFINED_UINT),
    m_cellMinK(UNDEFINED_UINT),
    m_cellMaxI(UNDEFINED_UINT),
    m_cellMaxJ(UNDEFINED_UINT),
    m_cellMaxK(UNDEFINED_UINT),
    m_scalarSetIndex(UNDEFINED_UINT),
    m_mapNodeAveragedScalars(false)
{
    CVF_ASSERT(grid);

    setCellRegionFullGrid();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
StructGridGeometry::~StructGridGeometry()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setCellRegion(uint minI, uint minJ, uint minK, uint maxI, uint maxJ, uint maxK)
{
    m_cellMinI = minI;
    m_cellMinJ = minJ;
    m_cellMinK = minK;
    m_cellMaxI = maxI;
    m_cellMaxJ = maxJ;
    m_cellMaxK = maxK;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setCellRegionSlabI(uint i)
{
    setCellRegionFullGrid();

    m_cellMinI = i;
    m_cellMaxI = i;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setCellRegionSlabJ(uint j)
{
    setCellRegionFullGrid();

    m_cellMinJ = j;
    m_cellMaxJ = j;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setCellRegionSlabK(uint k)
{
    setCellRegionFullGrid();

    m_cellMinK = k;
    m_cellMaxK = k;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setCellRegionFullGrid()
{
    uint cellCountI = m_grid->cellCountI();
    uint cellCountJ = m_grid->cellCountJ();
    uint cellCountK = m_grid->cellCountK();
    if (cellCountI >= 1 && cellCountJ >= 1 && cellCountK >= 1)
    {
        m_cellMinI = 0;
        m_cellMinJ = 0;
        m_cellMinK = 0;
        m_cellMaxI = cellCountI - 1;
        m_cellMaxJ = cellCountJ - 1;
        m_cellMaxK = cellCountK - 1;
    }
    else
    {
        resetCellRegion();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool StructGridGeometry::isCellRegionValid() const
{
    if (m_cellMinI == UNDEFINED_UINT  || m_cellMinJ == UNDEFINED_UINT || m_cellMinK == UNDEFINED_UINT) return false;
    if (m_cellMaxI == UNDEFINED_UINT  || m_cellMaxJ == UNDEFINED_UINT || m_cellMaxK == UNDEFINED_UINT) return false;

    if (m_cellMaxI < m_cellMinI) return false;
    if (m_cellMaxJ < m_cellMinJ) return false;
    if (m_cellMaxK < m_cellMinK) return false;

    uint cellCountI = m_grid->cellCountI();
    uint cellCountJ = m_grid->cellCountJ();
    uint cellCountK = m_grid->cellCountK();
    if (m_cellMaxI >= cellCountI) return false;
    if (m_cellMaxJ >= cellCountJ) return false;
    if (m_cellMaxK >= cellCountK) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::resetCellRegion()
{
    m_cellMinI = UNDEFINED_UINT;
    m_cellMinJ = UNDEFINED_UINT;
    m_cellMinK = UNDEFINED_UINT;
    m_cellMaxI = UNDEFINED_UINT;
    m_cellMaxJ = UNDEFINED_UINT;
    m_cellMaxK = UNDEFINED_UINT;
}


//--------------------------------------------------------------------------------------------------
/// Configure scalar field that should be mapped onto the surface
/// 
/// \param scalarSetIndex       Index of the cell scalar set that should be mapped
/// \param mapper               Mapper to use
/// \param nodeAveragedScalars  If true, node averaged scalar values will be computed based on the cell
///                             scalar set and these values will be mapped onto the surface.
//--------------------------------------------------------------------------------------------------
void StructGridGeometry::setMapScalar(uint scalarSetIndex, const ScalarMapper* mapper, bool nodeAveragedScalars)
{
    CVF_ASSERT(scalarSetIndex < m_grid->scalarSetCount());
    CVF_ASSERT(mapper);

    m_scalarSetIndex = scalarSetIndex;
    m_scalarMapper = mapper;
    m_mapNodeAveragedScalars = nodeAveragedScalars;
}


//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
/// 
/// If a map scalar has been set, the returned drawable will have both colors and texture coordinates
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometry::generateSurface() const
{
    //DebugTimer tim("StructGridGeometry::generateSurface()");

    if (!isCellRegionValid())
    {
        return NULL;
    }

    uint numCellsI = (m_cellMaxI - m_cellMinI) + 1;
    uint numCellsJ = (m_cellMaxJ - m_cellMinJ) + 1;
    uint numCellsK = (m_cellMaxK - m_cellMinK) + 1;

    uint numQuads = 2*numCellsI*numCellsJ + 2*numCellsI*numCellsK + 2*numCellsJ*numCellsK;
    uint numVerts = 4*numQuads;

    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(numVerts);

    bool doMapScalar = false;
    ref<Vec2fArray> textureCoordArray;
    ref<Color3ubArray> colorArray;
    if (m_scalarSetIndex != UNDEFINED_UINT && m_scalarMapper.notNull())
    {
        textureCoordArray = new Vec2fArray;
        textureCoordArray->reserve(numVerts);
        colorArray = new Color3ubArray;
        colorArray->reserve(numVerts);

        doMapScalar = true;
    }


    ubyte connBottom[4];
    ubyte connTop[4];
    ubyte connFront[4];
    ubyte connRight[4];
    ubyte connBack[4];
    ubyte connLeft[4];
    m_grid->cellFaceVertexIndices(RectilinearGrid::BOTTOM, connBottom);
    m_grid->cellFaceVertexIndices(RectilinearGrid::TOP,    connTop);
    m_grid->cellFaceVertexIndices(RectilinearGrid::FRONT,  connFront);
    m_grid->cellFaceVertexIndices(RectilinearGrid::RIGHT,  connRight);
    m_grid->cellFaceVertexIndices(RectilinearGrid::BACK,   connBack);
    m_grid->cellFaceVertexIndices(RectilinearGrid::LEFT,   connLeft);


    uint k;
    for (k = m_cellMinK; k <= m_cellMaxK; k++)
    {
        uint j;
        for (j = m_cellMinJ; j <= m_cellMaxJ; j++)
        {
            uint i;
            for (i = m_cellMinI; i <= m_cellMaxI; i++)
            {
                // Only consider cells at the edges of the region
                if (k == m_cellMinK || k == m_cellMaxK ||
                    j == m_cellMinJ || j == m_cellMaxJ ||
                    i == m_cellMinI || i == m_cellMaxI)
                {
                    // Count number of visible faces in this cell and add pointer to the face's local connectivity table
                    uint numVisibleFaces = 0;
                    const ubyte* visibleFacesConnects[6];
                    if (k == m_cellMinK)    visibleFacesConnects[numVisibleFaces++] = connBottom;
                    if (k == m_cellMaxK)    visibleFacesConnects[numVisibleFaces++] = connTop;
                    if (j == m_cellMinJ)    visibleFacesConnects[numVisibleFaces++] = connFront;
                    if (j == m_cellMaxJ)    visibleFacesConnects[numVisibleFaces++] = connBack;
                    if (i == m_cellMinI)    visibleFacesConnects[numVisibleFaces++] = connLeft;
                    if (i == m_cellMaxI)    visibleFacesConnects[numVisibleFaces++] = connRight;
                    CVF_TIGHT_ASSERT(numVisibleFaces > 0);

                    size_t cellIndex = m_grid->cellIndexFromIJK(i, j, k);

                    Vec3d cornerVerts[8];
                    m_grid->cellCornerVertices(cellIndex, cornerVerts);

                    double cornerScalars[8];
                    if (doMapScalar)
                    {
                        if (m_mapNodeAveragedScalars)
                        {
                            m_grid->cellCornerScalars(m_scalarSetIndex, i, j, k, cornerScalars);
                        }
                        else
                        {
                            // When not doing node averaging, just use the the cell center scalar value for all corners
                            // Not very elegant, nor optimal with regards to performance, but keeps the code simple
                            double cellScalarValue = m_grid->cellScalar(m_scalarSetIndex, i, j, k);
                            int n;
                            for (n = 0; n < 8; n++)
                            {
                                cornerScalars[n] = cellScalarValue;
                            }
                        }
                    }

                    uint face;
                    for (face = 0; face < numVisibleFaces; face++)
                    {
                        const ubyte* faceConn = visibleFacesConnects[face];
                        int n;
                        for (n = 0; n < 4; n++)
                        {
                            vertices->add(Vec3f(cornerVerts[faceConn[n]]));

                            if (doMapScalar)
                            {
                                textureCoordArray->add(m_scalarMapper->mapToTextureCoord(cornerScalars[faceConn[n]]));
                                colorArray->add(m_scalarMapper->mapToColor(cornerScalars[faceConn[n]]));
                            }
                        }
                    }
                }
            }
        }
    }


    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray(vertices.p());

    if (doMapScalar)
    {
        CVF_ASSERT(colorArray.notNull());
        CVF_ASSERT(textureCoordArray.notNull());
        CVF_ASSERT(colorArray->size() == vertices->size());
        CVF_ASSERT(textureCoordArray->size() == vertices->size());

        geo->setColorArray(colorArray.p());
        geo->setTextureCoordArray(textureCoordArray.p());
    }

    //Trace::show("StructGridGeometry::generateSurface()  #verts=%d  #quads=%d", vertices->size(), vertices->size()/4);
    //tim.reportTimeMS();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// 
/// This function does not generate all the nodes on the surface of the grid. Nodes are only
/// generated on the edges of the structured grid
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometry::generateSimplifiedMeshLines() const
{
    if (!isCellRegionValid())
    {
        return NULL;
    }

    uint numCellsI = (m_cellMaxI - m_cellMinI) + 1;
    uint numCellsJ = (m_cellMaxJ - m_cellMinJ) + 1;
    uint numCellsK = (m_cellMaxK - m_cellMinK) + 1;

    uint numLines = 4*(numCellsI + 1) + 4*(numCellsJ + 1) + 4*(numCellsK + 1);
    uint numVerts = numLines;

    Vec3d min = m_grid->gridPointCoordinate(m_cellMinI, m_cellMinJ, m_cellMinK);
    Vec3d max = m_grid->gridPointCoordinate(m_cellMaxI + 1, m_cellMaxJ + 1, m_cellMaxK + 1);

    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(numVerts);

    ref<UIntArray> indices = new UIntArray;
    indices->reserve(2*numLines);

    uint i;
    for (i = m_cellMinI; i <= m_cellMaxI + 1; i++)
    {
        uint baseIdx = static_cast<uint>(vertices->size());

        double coordX = m_grid->coordinatesI()[i];
        Vec3d v1(coordX, min.y(), min.z());
        Vec3d v2(coordX, max.y(), min.z());
        Vec3d v3(coordX, max.y(), max.z());
        Vec3d v4(coordX, min.y(), max.z());

        vertices->add(Vec3f(v1));
        vertices->add(Vec3f(v2));
        vertices->add(Vec3f(v3));
        vertices->add(Vec3f(v4));
        indices->add(baseIdx);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 3);
        indices->add(baseIdx + 3);
        indices->add(baseIdx);
    }

    uint j;
    for (j = m_cellMinJ; j <= m_cellMaxJ + 1; j++)
    {
        uint baseIdx = static_cast<uint>(vertices->size());

        double coordY = m_grid->coordinatesJ()[j];
        Vec3d v1(min.x(), coordY, min.z());
        Vec3d v2(max.x(), coordY, min.z());
        Vec3d v3(max.x(), coordY, max.z());
        Vec3d v4(min.x(), coordY, max.z());

        vertices->add(Vec3f(v1));
        vertices->add(Vec3f(v2));
        vertices->add(Vec3f(v3));
        vertices->add(Vec3f(v4));
        indices->add(baseIdx);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 3);
        indices->add(baseIdx + 3);
        indices->add(baseIdx);
    }

    uint k;
    for (k = m_cellMinK; k <= m_cellMaxK + 1; k++)
    {
        uint baseIdx = static_cast<uint>(vertices->size());

        double coordZ = m_grid->coordinatesK()[k];
        Vec3d v1(min.x(), min.y(), coordZ);
        Vec3d v2(max.x(), min.y(), coordZ);
        Vec3d v3(max.x(), max.y(), coordZ);
        Vec3d v4(min.x(), max.y(), coordZ);

        vertices->add(Vec3f(v1));
        vertices->add(Vec3f(v2));
        vertices->add(Vec3f(v3));
        vertices->add(Vec3f(v4));
        indices->add(baseIdx);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 1);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 2);
        indices->add(baseIdx + 3);
        indices->add(baseIdx + 3);
        indices->add(baseIdx);
    }

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_LINES);
    primSet->setIndices(indices.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridGeometry::generateOutline() const
{
    if (!isCellRegionValid())
    {
        return NULL;
    }

    Vec3f min(m_grid->minCoordinate());
    Vec3f max(m_grid->maxCoordinate());

    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(8);
    verts->add(Vec3f(min.x(), min.y(), min.z()));
    verts->add(Vec3f(max.x(), min.y(), min.z()));
    verts->add(Vec3f(max.x(), max.y(), min.z()));
    verts->add(Vec3f(min.x(), max.y(), min.z()));
    verts->add(Vec3f(min.x(), min.y(), max.z()));
    verts->add(Vec3f(max.x(), min.y(), max.z()));
    verts->add(Vec3f(max.x(), max.y(), max.z()));
    verts->add(Vec3f(min.x(), max.y(), max.z()));

    ref<UIntArray> indices = new UIntArray;
    indices->reserve(24);
    indices->add(0);    indices->add(1);
    indices->add(1);    indices->add(2);
    indices->add(2);    indices->add(3);
    indices->add(3);    indices->add(0);
    indices->add(4);    indices->add(5);
    indices->add(5);    indices->add(6);
    indices->add(6);    indices->add(7);
    indices->add(7);    indices->add(4);
    indices->add(0);    indices->add(4);
    indices->add(1);    indices->add(5);
    indices->add(2);    indices->add(6);
    indices->add(3);    indices->add(7);

    ref<PrimitiveSetIndexedUInt> ps = new PrimitiveSetIndexedUInt(PT_LINES);
    ps->setIndices(indices.p());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(verts.p());
    geo->addPrimitiveSet(ps.p());

    return geo;
}


} // namespace cvf

