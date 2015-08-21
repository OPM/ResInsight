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


#include "cvfStructGrid.h"

class RigFemPart;

class RigFemPartGrid : public cvf::StructGridInterface
{
public:
    RigFemPartGrid(RigFemPart* femPart);
    virtual ~RigFemPartGrid();

    virtual size_t      gridPointCountI() const;
    virtual size_t      gridPointCountJ() const;
    virtual size_t      gridPointCountK() const;

    virtual bool        isCellValid(size_t i, size_t j, size_t k) const;
    virtual cvf::Vec3d  minCoordinate() const;
    virtual cvf::Vec3d  maxCoordinate() const;
    virtual bool        cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex) const;
    virtual size_t      cellIndexFromIJK(size_t i, size_t j, size_t k) const;
    virtual bool        ijkFromCellIndex(size_t cellIndex, size_t* i, size_t* j, size_t* k) const;
    virtual bool        cellIJKFromCoordinate(const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k) const;
    virtual void        cellCornerVertices(size_t cellIndex, cvf::Vec3d vertices[8]) const;
    virtual cvf::Vec3d  cellCentroid(size_t cellIndex) const;
    virtual void        cellMinMaxCordinates(size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate) const;
    virtual size_t      gridPointIndexFromIJK(size_t i, size_t j, size_t k) const;
    virtual cvf::Vec3d  gridPointCoordinate(size_t i, size_t j, size_t k) const;

 
 private:
    void                generateStructGridData();

    cvf::Vec3i          findMainIJKFaces(int elementIndex);

    int                 findElmIdxForIJK000();

    int                 perpendicularFaceInDirection(cvf::Vec3f direction, int perpFaceIdx, int elmIdx);
    RigFemPart*         m_femPart;

    std::vector<cvf::Vec3i> m_ijkPrElement;
    cvf::Vec3st         m_elmentIJKCounts;

};




