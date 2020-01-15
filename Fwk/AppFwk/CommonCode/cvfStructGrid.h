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

#include <cstddef>
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafAppEnum.h"



namespace cvf {

class CellFilterBase;

// Navneforslag
//    StructGridGeometryGeneratorInterface

// Main purpose of this class is to define the interface to be used by geometry generators
class StructGridInterface : public Object
{
public:
    enum FaceType
    {
        POS_I,
        NEG_I,
        POS_J,
        NEG_J,
        POS_K,
        NEG_K,
        NO_FACE
    };

    typedef caf::AppEnum<StructGridInterface::FaceType> FaceEnum;


    enum class GridAxisType
    {
        AXIS_I,
        AXIS_J,
        AXIS_K,
        NO_AXIS
    };

public:
    StructGridInterface();

    virtual size_t   gridPointCountI() const = 0;
    virtual size_t   gridPointCountJ() const = 0;
    virtual size_t   gridPointCountK() const = 0;

    size_t           cellCountI() const;
    size_t           cellCountJ() const;
    size_t           cellCountK() const;

    virtual bool        isCellValid(size_t i, size_t j, size_t k) const = 0;

    virtual cvf::Vec3d  minCoordinate() const = 0;
    virtual cvf::Vec3d  maxCoordinate() const = 0;
    void                characteristicCellSizes(double* iSize, double* jSize, double* kSize) const;

    virtual cvf::Vec3d  displayModelOffset() const;

    virtual bool        cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex) const = 0;

    virtual size_t      cellIndexFromIJK(size_t i, size_t j, size_t k) const = 0;
    virtual bool        ijkFromCellIndex(size_t cellIndex, size_t* i, size_t* j, size_t* k) const = 0;

    virtual bool        cellIJKFromCoordinate(const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k) const = 0;

    virtual void        cellCornerVertices(size_t cellIndex, cvf::Vec3d vertices[8]) const = 0;
    virtual cvf::Vec3d  cellCentroid(size_t cellIndex) const = 0;
    virtual void        cellMinMaxCordinates(size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate) const = 0;

    virtual size_t      gridPointIndexFromIJK(size_t i, size_t j, size_t k) const = 0;
    virtual cvf::Vec3d  gridPointCoordinate(size_t i, size_t j, size_t k) const = 0;


public:
    static void cellFaceVertexIndices(FaceType face, cvf::ubyte vertexIndices[4]);
    static FaceType oppositeFace(FaceType face);
    static void neighborIJKAtCellFace(size_t i, size_t j, size_t k, StructGridInterface::FaceType face, size_t* ni, size_t* nj, size_t* nk);

    static GridAxisType gridAxisFromFace(FaceType face);

private:
    mutable double m_characteristicCellSizeI;
    mutable double m_characteristicCellSizeJ;
    mutable double m_characteristicCellSizeK;
};

} // namespace cvf
