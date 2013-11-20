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


#pragma once

#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfCollection.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class RectilinearGrid : public Object
{
public:
    enum Face
    {
        BOTTOM, // -K
        TOP,    // +K
        FRONT,  // -J
        RIGHT,  // +I
        BACK,   // +J
        LEFT    // -I
    };

public:
    RectilinearGrid();

    ref<RectilinearGrid>    shallowCopy() const;

    void                    allocateGrid(uint numGridPointsI, uint numGridPointsJ, uint numGridPointsK);
    uint                    gridPointCountI() const;
    uint                    gridPointCountJ() const;
    uint                    gridPointCountK() const;
    size_t                  gridPointCount() const;
    bool                    isLegalGridPoint(uint i, uint j, uint k) const;

    uint                    cellCountI() const;
    uint                    cellCountJ() const;
    uint                    cellCountK() const;
    size_t                  cellCount() const;
    bool                    isLegalCell(uint i, uint j, uint k) const;

    void                    setCoordinatesI(const DoubleArray& coords);
    void                    setCoordinatesJ(const DoubleArray& coords);
    void                    setCoordinatesK(const DoubleArray& coords);
    void                    setCoordinatesToRegularGrid(const Vec3d& origo, const Vec3d& spacing);
    const DoubleArray&      coordinatesI() const;
    const DoubleArray&      coordinatesJ() const;
    const DoubleArray&      coordinatesK() const;

    Vec3d                   minCoordinate() const;
    Vec3d                   maxCoordinate() const;

    bool                    cellNeighbor(size_t cellIndex, Face face, size_t* neighborCellIndex) const;
    size_t                  cellIndexFromIJK(uint i, uint j, uint k) const;
    bool                    ijkFromCellIndex(size_t cellIndex, uint* i, uint* j, uint* k) const;
    bool                    cellIJKFromCoordinate(const Vec3d& coord, uint* i, uint* j, uint* k) const;

    void                    cellCornerVertices(size_t cellIndex, Vec3d vertices[8]) const;
    static void             cellFaceVertexIndices(Face face, ubyte vertexIndices[4]);
    Vec3d                   cellCentroid(size_t cellIndex) const;
    void                    cellMinMaxCordinates(size_t cellIndex, Vec3d* minCoordinate, Vec3d* maxCoordinate) const;
    
    size_t                  gridPointIndexFromIJK(uint i, uint j, uint k) const;
    Vec3d                   gridPointCoordinate(uint i, uint j, uint k) const;

    uint                    gridPointNeighborCells(uint i, uint j, uint k, size_t neighborCellIndices[8]) const;

    // Scalar results
    uint                    addScalarSet(DoubleArray* scalarValues);
    uint                    scalarSetCount() const;
    const DoubleArray*      scalarSet(uint scalarSetIndex) const;

    ref<DoubleArray>        computeGridPointScalarSet(uint scalarSetIndex) const;
    void                    setGridPointScalarSet(uint scalarSetIndex, DoubleArray* gridPointScalarValues);

    double                  cellScalar(uint scalarSetIndex, uint i, uint j, uint k) const;
    void                    cellCornerScalars(uint scalarSetIndex, uint i, uint j, uint k, double scalars[8]) const;
    double                  gridPointScalar(uint scalarSetIndex, uint i, uint j, uint k) const;
    bool                    pointScalar(uint scalarSetIndex, const Vec3d& p, double* scalarValue) const;

    // Vector results
    uint                    addVectorSet(Vec3dArray* vectorValues);
    uint                    vectorSetCount() const;
    const Vec3dArray*       vectorSet(uint vectorSetIndex) const;
    
    //void                    filteredCellCenterResultVectors(Vec3dArray& positions, Vec3dArray& resultVectors, const double minPositionDistance, const double resultVectorLengthThreshold) const;
    void                    filteredCellCenterResultVectors(Vec3dArray& positions, Vec3dArray& resultVectors, uint vectorSetIndex, uint stride, const double resultVectorLengthThreshold) const;
private:
    uint                    m_iCount;       // Number of grid points in I direction (number of cells is numGridPoints - 1)
    uint                    m_jCount;       
    uint                    m_kCount;
    DoubleArray             m_iCoordinates; // Grid point coordinates in I direction, one entry per grid point in I direction
    DoubleArray             m_jCoordinates;
    DoubleArray             m_kCoordinates;

    Collection<DoubleArray> m_scalarSets;           // Set of scalar results
    Collection<DoubleArray> m_gridPointScalarSets;  // Set of scalar results for grid points. Corresponds to the scalarSets

    Collection<Vec3dArray>  m_vectorSets;           // Set of vector results
};

}
