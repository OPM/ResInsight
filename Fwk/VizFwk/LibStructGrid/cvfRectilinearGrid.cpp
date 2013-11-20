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
#include "cvfRectilinearGrid.h"
#include "cvfBoundingBox.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RectilinearGrid
/// \ingroup StructGrid
///
/// 
/// 
/// 
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RectilinearGrid::RectilinearGrid()
:   m_iCount(0), 
    m_jCount(0), 
    m_kCount(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<RectilinearGrid> RectilinearGrid::shallowCopy() const
{
    ref<RectilinearGrid> newGrid = new RectilinearGrid;

    newGrid->m_iCount = m_iCount;
    newGrid->m_jCount = m_jCount;
    newGrid->m_kCount = m_kCount;
    newGrid->m_iCoordinates = m_iCoordinates;
    newGrid->m_jCoordinates = m_jCoordinates;
    newGrid->m_kCoordinates = m_kCoordinates;

    size_t numScalarSets = m_scalarSets.size();
    size_t i;
    for (i = 0; i < numScalarSets; i++)
    {
        ref<DoubleArray> sclSet = m_scalarSets[i];
        CVF_ASSERT(sclSet.notNull());

        newGrid->m_scalarSets.push_back(sclSet.p());

        ref<DoubleArray> gridPointSclSet = m_gridPointScalarSets[i];
        newGrid->m_gridPointScalarSets.push_back(gridPointSclSet.p());
    }

    size_t numVectorSets = m_vectorSets.size();
    for (i = 0; i < numVectorSets; i++)
    {
        ref<Vec3dArray> vecSet = m_vectorSets[i];
        CVF_ASSERT(vecSet.notNull());

        newGrid->m_vectorSets.push_back(vecSet.p());
    }

    return newGrid;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::allocateGrid(uint numGridPointsI, uint numGridPointsJ, uint numGridPointsK)
{
    CVF_ASSERT(numGridPointsI >= 2);
    CVF_ASSERT(numGridPointsJ >= 2);
    CVF_ASSERT(numGridPointsK >= 2);

    m_iCount = numGridPointsI;
    m_jCount = numGridPointsJ;
    m_kCount = numGridPointsK;

    m_iCoordinates.resize(m_iCount);
    m_jCoordinates.resize(m_jCount);
    m_kCoordinates.resize(m_kCount);
    m_iCoordinates.setAll(0);
    m_jCoordinates.setAll(0);
    m_kCoordinates.setAll(0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::gridPointCountI() const
{
    return m_iCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::gridPointCountJ() const
{
    return m_jCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::gridPointCountK() const
{
    return m_kCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RectilinearGrid::gridPointCount() const
{
    if (m_iCount > 0 && m_jCount > 0 && m_kCount > 0)
    {
        return m_iCount*m_jCount*m_kCount;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::isLegalGridPoint(uint i, uint j, uint k) const
{
    if ((i < m_iCount) && 
        (j < m_jCount) && 
        (k < m_kCount))
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::cellCountI() const
{
    if (m_iCount > 1)
    {
        return m_iCount - 1;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::cellCountJ() const
{
    if (m_jCount > 1)
    {
        return m_jCount - 1;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::uint RectilinearGrid::cellCountK() const
{
    if (m_kCount > 1)
    {
        return m_kCount - 1;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the total number of cells in the grid
//--------------------------------------------------------------------------------------------------
size_t RectilinearGrid::cellCount() const
{
    if (m_iCount > 1 && m_jCount > 1 && m_kCount > 1)
    {
        return (m_iCount - 1)*(m_jCount - 1)*(m_kCount - 1);
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::isLegalCell(uint i, uint j, uint k) const
{
    if ((i < m_iCount - 1) && 
        (j < m_jCount - 1) && 
        (k < m_kCount - 1))
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::setCoordinatesI(const DoubleArray& coords)
{
    CVF_ASSERT(coords.size() == static_cast<size_t>(m_iCount));
    CVF_ASSERT(m_iCoordinates.size() == coords.size());

    m_iCoordinates = coords;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::setCoordinatesJ(const DoubleArray& coords)
{
    CVF_ASSERT(coords.size() == static_cast<size_t>(m_jCount));
    CVF_ASSERT(m_jCoordinates.size() == coords.size());

    m_jCoordinates = coords;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::setCoordinatesK(const DoubleArray& coords)
{
    CVF_ASSERT(coords.size() == static_cast<size_t>(m_kCount));
    CVF_ASSERT(m_kCoordinates.size() == coords.size());

    m_kCoordinates = coords;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::setCoordinatesToRegularGrid(const Vec3d& origo, const Vec3d& spacing)
{
    DoubleArray ivals;
    DoubleArray jvals;
    DoubleArray kvals;
    ivals.resize(m_iCount);
    jvals.resize(m_jCount);
    kvals.resize(m_kCount);

    uint i;
    for (i = 0; i < m_iCount; i++)
    {
        ivals[i] = origo.x() + i*spacing.x();
    }

    uint j;
    for (j = 0; j < m_jCount; j++)
    {
        jvals[j] = origo.y() + j*spacing.y();
    }

    uint k;
    for (k = 0; k < m_kCount; k++)
    {
        kvals[k] = origo.z() + k*spacing.z();
    }

    setCoordinatesI(ivals);
    setCoordinatesJ(jvals);
    setCoordinatesK(kvals);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DoubleArray& RectilinearGrid::coordinatesI() const
{
    CVF_ASSERT(m_iCoordinates.size() == m_iCount);
    return m_iCoordinates;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DoubleArray& RectilinearGrid::coordinatesJ() const
{
    CVF_ASSERT(m_jCoordinates.size() == m_jCount);
    return m_jCoordinates;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DoubleArray& RectilinearGrid::coordinatesK() const
{
    CVF_ASSERT(m_kCoordinates.size() == m_kCount);
    return m_kCoordinates;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d RectilinearGrid::minCoordinate() const
{
    if (m_iCount > 0 && m_jCount > 0 && m_kCount > 0)
    {
        return Vec3d(m_iCoordinates[0], m_jCoordinates[0], m_kCoordinates[0]);
    }
    else
    {
        return Vec3d::ZERO;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d RectilinearGrid::maxCoordinate() const
{
    if (m_iCount > 0 && m_jCount > 0 && m_kCount > 0)
    {
        return Vec3d(m_iCoordinates[m_iCount - 1], m_jCoordinates[m_jCount - 1], m_kCoordinates[m_kCount - 1]);
    }
    else
    {
        return Vec3d::ZERO;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::cellNeighbor(size_t cellIndex, Face face, size_t* neighborCellIndex) const
{
    uint i, j, k;
    if (!ijkFromCellIndex(cellIndex, &i, &j, &k))
    {
        return false;
    }

    if (face == BOTTOM)
    {
        if (k > 0)
        {
            k--;
        }
        else
        {
            return false;
        }
    }
    else if (face == TOP)
    {
        if (k < m_kCount - 2)
        {
            k++;
        }
        else
        {
            return false;
        }
    }
    else if (face == FRONT)
    {
        if (j > 0)
        {
            j--;
        }
        else
        {
            return false;
        }
    }
    else if (face == BACK)
    {
        if (j < m_jCount - 2)
        {
            j++;
        }
        else
        {
            return false;
        }
    }
    else if (face == LEFT)
    {
        if (i > 0)
        {
            i--;
        }
        else
        {
            return false;
        }
    }
    else if (face == RIGHT)
    {
        if (i < m_iCount - 2)
        {
            i++;
        }
        else
        {
            return false;
        }
    }

    *neighborCellIndex = cellIndexFromIJK(i, j, k);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RectilinearGrid::cellIndexFromIJK(uint i, uint j, uint k) const
{
    CVF_TIGHT_ASSERT(i < (m_iCount - 1));
    CVF_TIGHT_ASSERT(j < (m_jCount - 1));
    CVF_TIGHT_ASSERT(k < (m_kCount - 1));

    size_t ci = i + j*(m_iCount - 1) + k*((m_iCount - 1)*(m_jCount - 1));
    return ci;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::ijkFromCellIndex(size_t cellIndex, uint* i, uint* j, uint* k) const
{
    CVF_TIGHT_ASSERT(cellIndex < cellCount());

    uint ck = static_cast<uint>(cellIndex/((m_iCount - 1)*(m_jCount - 1)));
    uint cj = static_cast<uint>((cellIndex - ck*((m_iCount - 1)*(m_jCount - 1)))/(m_iCount - 1));
    uint ci = static_cast<uint>(cellIndex - (ck*((m_iCount - 1)*(m_jCount - 1)) + cj*(m_iCount - 1)));

    *i = ci;
    *j = cj;
    *k = ck;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::cellCornerVertices(size_t cellIndex, Vec3d vertices[8]) const
{
    //     7---------6                
    //    /|        /|     |k         
    //   / |       / |     | /j       
    //  4---------5  |     |/         
    //  |  3------|--2     *---i      
    //  | /       | /                 
    //  |/        |/                  
    //  0---------1                     

    uint i, j, k;
    ijkFromCellIndex(cellIndex, &i, &j, &k);

    vertices[0].set(m_iCoordinates[i],     m_jCoordinates[j],     m_kCoordinates[k]);
    vertices[1].set(m_iCoordinates[i + 1], m_jCoordinates[j],     m_kCoordinates[k]);
    vertices[2].set(m_iCoordinates[i + 1], m_jCoordinates[j + 1], m_kCoordinates[k]);
    vertices[3].set(m_iCoordinates[i],     m_jCoordinates[j + 1], m_kCoordinates[k]);

    vertices[4].set(m_iCoordinates[i],     m_jCoordinates[j],     m_kCoordinates[k + 1]);
    vertices[5].set(m_iCoordinates[i + 1], m_jCoordinates[j],     m_kCoordinates[k + 1]);
    vertices[6].set(m_iCoordinates[i + 1], m_jCoordinates[j + 1], m_kCoordinates[k + 1]);
    vertices[7].set(m_iCoordinates[i],     m_jCoordinates[j + 1], m_kCoordinates[k + 1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::cellFaceVertexIndices(Face face, ubyte vertexIndices[4])
{
    // The ordering of the faces is consistent with GLviewAPI's hexahedron element.
    // Note that the vertex ordering within a face is not consistent 
    //
    //     7---------6                Faces:
    //    /|        /|     |k           0 bottom   0, 3, 2, 1
    //   / |       / |     | /j         1 top      4, 5, 6, 7
    //  4---------5  |     |/           2 front    4, 0, 1, 5
    //  |  3------|--2     *---i        3 right    5, 1, 2, 6
    //  | /       | /                   4 back     6, 2, 3, 7
    //  |/        |/                    5 left     7, 3, 0, 4
    //  0---------1                     

    if (face == BOTTOM)
    {
        vertexIndices[0] = 0;
        vertexIndices[1] = 3;
        vertexIndices[2] = 2;
        vertexIndices[3] = 1;
    }
    else if (face == TOP)
    {
        vertexIndices[0] = 4;
        vertexIndices[1] = 5;
        vertexIndices[2] = 6;
        vertexIndices[3] = 7;
    }
    else if (face == FRONT)
    {
        vertexIndices[0] = 4;
        vertexIndices[1] = 0;
        vertexIndices[2] = 1;
        vertexIndices[3] = 5;
    }
    else if (face == RIGHT)
    {
        vertexIndices[0] = 5;
        vertexIndices[1] = 1;
        vertexIndices[2] = 2;
        vertexIndices[3] = 6;
    }
    else if (face == BACK)
    {
        vertexIndices[0] = 6;
        vertexIndices[1] = 2;
        vertexIndices[2] = 3;
        vertexIndices[3] = 7;
    }
    else if (face == LEFT)
    {
        vertexIndices[0] = 7;
        vertexIndices[1] = 3;
        vertexIndices[2] = 0;
        vertexIndices[3] = 4;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d RectilinearGrid::cellCentroid(size_t cellIndex) const
{
    uint i, j, k;
    ijkFromCellIndex(cellIndex, &i, &j, &k);

    Vec3d v(m_iCoordinates[i], m_jCoordinates[j], m_kCoordinates[k]);
    v += Vec3d(m_iCoordinates[i + 1], m_jCoordinates[j + 1], m_kCoordinates[k + 1]);

    return v/2;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::cellMinMaxCordinates(size_t cellIndex, Vec3d* minCoordinate, Vec3d* maxCoordinate) const
{
    uint i, j, k;
    ijkFromCellIndex(cellIndex, &i, &j, &k);

    minCoordinate->set(m_iCoordinates[i], m_jCoordinates[j], m_kCoordinates[k]);
    maxCoordinate->set(m_iCoordinates[i + 1], m_jCoordinates[j + 1], m_kCoordinates[k + 1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RectilinearGrid::gridPointIndexFromIJK(uint i, uint j, uint k) const
{
    CVF_TIGHT_ASSERT(i < m_iCount && j < m_jCount && k < m_kCount);

    size_t gpi = i + j*m_iCount + k*(m_iCount*m_jCount);
    return gpi;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d RectilinearGrid::gridPointCoordinate(uint i, uint j, uint k) const
{
    CVF_TIGHT_ASSERT(i < m_iCount && j < m_jCount && k < m_kCount);

    return Vec3d(m_iCoordinates[i], m_jCoordinates[j], m_kCoordinates[k]);
}


//--------------------------------------------------------------------------------------------------
/// Find the neighbor cells connected to a grid point
///
/// \return	The number of cell indices returned in \a neighborCellIndices
//--------------------------------------------------------------------------------------------------
uint RectilinearGrid::gridPointNeighborCells(uint i, uint j, uint k, size_t neighborCellIndices[8]) const
{
    CVF_TIGHT_ASSERT(i < m_iCount && j < m_jCount && k < m_kCount);

    uint count = 0;

    if (isLegalCell(i - 1, j - 1, k - 1))   neighborCellIndices[count++] = cellIndexFromIJK(i - 1, j - 1, k - 1);
    if (isLegalCell(i - 1, j - 1, k    ))   neighborCellIndices[count++] = cellIndexFromIJK(i - 1, j - 1, k    );
    if (isLegalCell(i - 1, j    , k - 1))   neighborCellIndices[count++] = cellIndexFromIJK(i - 1, j    , k - 1);
    if (isLegalCell(i - 1, j    , k    ))   neighborCellIndices[count++] = cellIndexFromIJK(i - 1, j    , k    );

    if (isLegalCell(i    , j - 1, k - 1))   neighborCellIndices[count++] = cellIndexFromIJK(i    , j - 1, k - 1);
    if (isLegalCell(i    , j - 1, k    ))   neighborCellIndices[count++] = cellIndexFromIJK(i    , j - 1, k    );
    if (isLegalCell(i    , j    , k - 1))   neighborCellIndices[count++] = cellIndexFromIJK(i    , j    , k - 1);
    if (isLegalCell(i    , j    , k    ))   neighborCellIndices[count++] = cellIndexFromIJK(i    , j    , k    );

    return count;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RectilinearGrid::addScalarSet(DoubleArray* scalarValues)
{
    CVF_TIGHT_ASSERT(scalarValues->size() == cellCount());

    m_scalarSets.push_back(scalarValues);
    m_gridPointScalarSets.push_back(NULL);

    return static_cast<uint>(m_scalarSets.size() - 1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RectilinearGrid::scalarSetCount() const
{
    return static_cast<uint>(m_scalarSets.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const DoubleArray* RectilinearGrid::scalarSet(uint scalarSetIndex) const
{
    CVF_ASSERT(scalarSetIndex < m_scalarSets.size());
    return m_scalarSets.at(scalarSetIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RectilinearGrid::cellScalar(uint scalarSetIndex, uint i, uint j, uint k) const
{
    CVF_ASSERT(scalarSetIndex < m_scalarSets.size());
    const DoubleArray* scl = scalarSet(scalarSetIndex);
    CVF_ASSERT(scl);

    size_t cellIndex = cellIndexFromIJK(i, j, k);
    CVF_ASSERT(cellIndex < scl->size());

    return scl->get(cellIndex);
}


//--------------------------------------------------------------------------------------------------
/// Compute averaged scalar values in all cell corners.
///
/// \todo  The current implementation is far from optimal and may have to be reworked.
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::cellCornerScalars(uint scalarSetIndex, uint i, uint j, uint k, double scalars[8]) const
{
    CVF_ASSERT(scalarSetIndex < m_scalarSets.size());

    scalars[0] = gridPointScalar(scalarSetIndex, i,     j,     k);
    scalars[1] = gridPointScalar(scalarSetIndex, i + 1, j,     k);
    scalars[2] = gridPointScalar(scalarSetIndex, i + 1, j + 1, k);
    scalars[3] = gridPointScalar(scalarSetIndex, i,     j + 1, k);
    scalars[4] = gridPointScalar(scalarSetIndex, i,     j,     k + 1);
    scalars[5] = gridPointScalar(scalarSetIndex, i + 1, j,     k + 1);
    scalars[6] = gridPointScalar(scalarSetIndex, i + 1, j + 1, k + 1);
    scalars[7] = gridPointScalar(scalarSetIndex, i,     j + 1, k + 1);
}


//--------------------------------------------------------------------------------------------------
/// Compute averaged scalar value in specified grid point
///
/// The average scalar value is computed as an unweighted average of the cell center scalar values of 
/// all the cells that are sharing the specified grid point.
//--------------------------------------------------------------------------------------------------
double RectilinearGrid::gridPointScalar(uint scalarSetIndex, uint i, uint j, uint k) const
{
    CVF_ASSERT(scalarSetIndex < m_scalarSets.size());
    CVF_ASSERT(i < m_iCount && j < m_jCount && k < m_kCount);

    double val = 0;

    if (m_gridPointScalarSets.at(scalarSetIndex) )
    {
        size_t idx = gridPointIndexFromIJK(i, j, k);
       
        const DoubleArray* gridPointArray = m_gridPointScalarSets.at(scalarSetIndex);
        CVF_ASSERT(gridPointArray);
        val = gridPointArray->get(idx);
    }
    else
    {
        const DoubleArray* scl = scalarSet(scalarSetIndex);
        CVF_ASSERT(scl);

        size_t cellIndices[8];
        uint numNeighbors = gridPointNeighborCells(i, j, k, cellIndices);
        CVF_ASSERT(numNeighbors > 0);

        cvf::uint n;
        for (n = 0; n < numNeighbors; n++)
        {
            val += scl->get(cellIndices[n]);
        }

        val /= static_cast<double>(numNeighbors);
    }

    return val;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::pointScalar(uint scalarSetIndex, const Vec3d& p, double* scalarValue) const
{
    // See:
    //  http://en.wikipedia.org/wiki/Trilinear_interpolation
    //  http://paulbourke.net/miscellaneous/interpolation/

    CVF_ASSERT(scalarValue);

    uint i, j, k;
    if (cellIJKFromCoordinate(p, &i, &j, &k))
    {
        double s[8];
        cellCornerScalars(scalarSetIndex, i, j, k, s);

        Vec3d min, max;
        cellMinMaxCordinates(cellIndexFromIJK(i, j, k), &min, &max);

        double dX = max.x() - min.x();
        double dY = max.y() - min.y();
        double dZ = max.z() - min.z();

        double interpolatedValue = 1/(dX*dY*dZ) * (
        s[0]* (max.x() - p.x())* (max.y() - p.y())* (max.z() - p.z()) +
        s[1]* (p.x() - min.x())* (max.y() - p.y())* (max.z() - p.z()) +
        s[3]* (max.x() - p.x())* (p.y() - min.y())* (max.z() - p.z()) +
        s[4]* (max.x() - p.x())* (max.y() - p.y())* (p.z() - min.z()) +
        s[5]* (p.x() - min.x())* (max.y() - p.y())* (p.z() - min.z()) +
        s[7]* (max.x() - p.x())* (p.y() - min.y())* (p.z() - min.z()) +
        s[2]* (p.x() - min.x())* (p.y() - min.y())* (max.z() - p.z()) +
        s[6]* (p.x() - min.x())* (p.y() - min.y())* (p.z() - min.z()));

        *scalarValue = interpolatedValue;
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Returns the grid point averaged values of the centroid scalarSetIndex 
//--------------------------------------------------------------------------------------------------
ref<DoubleArray> RectilinearGrid::computeGridPointScalarSet( uint scalarSetIndex ) const
{
    CVF_ASSERT(scalarSetIndex < scalarSetCount());

    ref<DoubleArray> result = new DoubleArray;

    result->reserve(gridPointCount());
    uint i,j,k;
    for (k = 0; k < gridPointCountK(); ++k)
    {
        for (j = 0; j < gridPointCountJ(); ++j)
        {
            for (i = 0; i < gridPointCountI(); ++i)
            {
                result->add(gridPointScalar(scalarSetIndex, i, j, k));
            }
        }
    }

    CVF_ASSERT(gridPointCount() == result->size());

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Set a vector of scalar results for the grid points (cell corners). 
/// The values are understood as related to the centroid scalars with the 
/// same scalarSetIndex
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::setGridPointScalarSet( uint scalarSetIndex, DoubleArray* gridPointScalarValues )
{
    CVF_ASSERT(scalarSetIndex < scalarSetCount());
    m_gridPointScalarSets[scalarSetIndex] = gridPointScalarValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RectilinearGrid::addVectorSet(Vec3dArray* vectorValues)
{
    CVF_TIGHT_ASSERT(vectorValues->size() == cellCount());

    m_vectorSets.push_back(vectorValues);

    return static_cast<uint>(m_vectorSets.size() - 1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RectilinearGrid::vectorSetCount() const
{
    return static_cast<uint>(m_vectorSets.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Vec3dArray* RectilinearGrid::vectorSet(uint vectorSetIndex) const
{
    CVF_ASSERT(vectorSetIndex < m_vectorSets.size());
    return m_vectorSets.at(vectorSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// Returns each vectors that have at least resultVectorLengthThreshold length, while omitting stride
/// result points between each. Each "line" of returned result points are offset one compared to 
/// the "previous" line to get a more evenly distributed field.
//--------------------------------------------------------------------------------------------------
void RectilinearGrid::filteredCellCenterResultVectors(Vec3dArray& positions, Vec3dArray& resultVectors, uint vectorSetIndex, uint stride, const double resultVectorLengthThreshold) const
{
    uint i = 0;
    uint j = 0;
    uint k = 0;
    std::vector<size_t> filteredCellIndices;

    const Vec3dArray* vectorResult = vectorSet(vectorSetIndex);
    CVF_ASSERT(vectorResult);

    uint indexAdd = 1 + stride;

    while (k < cellCountK())
    {
        while (j < cellCountJ())
        {
            while (i < cellCountI())
            {
                size_t cellIndex = cellIndexFromIJK(i, j, k);
                double lenght = vectorResult->get(cellIndex).length();

                if (lenght >= resultVectorLengthThreshold)
                {
                    filteredCellIndices.push_back(cellIndex);
                }

                i += indexAdd;
            }

            j += 1;
            i = indexAdd ? (j+k) % indexAdd : 0;
        }

        j = 0;
        k += 1;
    }

    positions.reserve(filteredCellIndices.size());
    resultVectors.reserve(filteredCellIndices.size());

    size_t idx;
    for (idx = 0; idx < filteredCellIndices.size(); idx++)
    {
        positions.add(cellCentroid(filteredCellIndices[idx]));
        resultVectors.add(vectorResult->get(filteredCellIndices[idx]));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RectilinearGrid::cellIJKFromCoordinate(const Vec3d& coord, uint* i, uint* j, uint* k) const
{
    cvf::BoundingBox bb;
    bb.add(minCoordinate());
    bb.add(maxCoordinate());

    if (!bb.contains(coord))
    {
        return false;
    }
   
    if (i)
    {
        DoubleArray::const_iterator it = std::lower_bound(m_iCoordinates.begin(), m_iCoordinates.end(), coord.x());

        uint gridPointInsertionIndex = static_cast<uint>(it - m_iCoordinates.begin());
        CVF_ASSERT(gridPointInsertionIndex < m_iCount);

        if (gridPointInsertionIndex > 0) gridPointInsertionIndex -= 1;

        *i = gridPointInsertionIndex;
    }

    if (j)
    {
        DoubleArray::const_iterator it = std::lower_bound(m_jCoordinates.begin(), m_jCoordinates.end(), coord.y());
        uint gridPointInsertionIndex = static_cast<uint>(it - m_jCoordinates.begin());
        CVF_ASSERT(gridPointInsertionIndex < m_jCount);

        if (gridPointInsertionIndex > 0) gridPointInsertionIndex -= 1;

        *j = gridPointInsertionIndex;
    }

    if (k)
    {
        DoubleArray::const_iterator it = std::lower_bound(m_kCoordinates.begin(), m_kCoordinates.end(), coord.z());
        uint gridPointInsertionIndex = static_cast<uint>(it - m_kCoordinates.begin());
        CVF_ASSERT(gridPointInsertionIndex < m_kCount);

        if (gridPointInsertionIndex > 0) gridPointInsertionIndex -= 1;

        *k = gridPointInsertionIndex;
    }

    return true;
}


} // namespace cvf

