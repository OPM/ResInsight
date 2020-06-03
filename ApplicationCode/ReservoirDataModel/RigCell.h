/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include "RigLocalGrid.h"

#include "cvfStructGrid.h"

#include <array>

namespace cvf
{
class Ray;
}

class RigCell
{
public:
    RigCell();
    ~RigCell(); // Not virtual, to save space. Do not inherit from this class

    std::array<size_t, 8>&       cornerIndices() { return m_cornerIndices; }
    const std::array<size_t, 8>& cornerIndices() const { return m_cornerIndices; }

    void faceIndices( cvf::StructGridInterface::FaceType face, std::array<size_t, 4>* faceIndices ) const;

    bool isInvalid() const { return m_isInvalid; }
    void setInvalid( bool val ) { m_isInvalid = val; }

    size_t gridLocalCellIndex() const { return m_gridLocalCellIndex; }
    void   setGridLocalCellIndex( size_t val ) { m_gridLocalCellIndex = val; }

    RigLocalGrid* subGrid() const { return m_subGrid; }
    void          setSubGrid( RigLocalGrid* subGrid ) { m_subGrid = subGrid; }
    void          removeSubGrid( RigLocalGrid* subGrid ) { m_subGrid = nullptr; }

    RigGridBase* hostGrid() const { return m_hostGrid; }
    void         setHostGrid( RigGridBase* hostGrid ) { m_hostGrid = hostGrid; }

    size_t parentCellIndex() const { return m_parentCellIndex; }
    void   setParentCellIndex( size_t parentCellIndex ) { m_parentCellIndex = parentCellIndex; }
    size_t mainGridCellIndex() const { return m_mainGridCellIndex; }
    void   setMainGridCellIndex( size_t mainGridCellContainingThisCell )
    {
        m_mainGridCellIndex = mainGridCellContainingThisCell;
    }

    size_t coarseningBoxIndex() const { return m_coarseningBoxIndex; }
    void   setCoarseningBoxIndex( size_t coarseningBoxIndex ) { m_coarseningBoxIndex = coarseningBoxIndex; }

    void setCellFaceFault( cvf::StructGridInterface::FaceType face ) { m_cellFaceFaults[face] = true; }
    bool isCellFaceFault( cvf::StructGridInterface::FaceType face ) const { return m_cellFaceFaults[face]; }

    cvf::Vec3d center() const;
    cvf::Vec3d faceCenter( cvf::StructGridInterface::FaceType face ) const;
    cvf::Vec3d faceNormalWithAreaLength( cvf::StructGridInterface::FaceType face ) const;
    double     volume() const;

    int  firstIntersectionPoint( const cvf::Ray& ray, cvf::Vec3d* intersectionPoint ) const;
    bool isLongPyramidCell( double maxHeightFactor = 5, double nodeNearTolerance = 1e-3 ) const;
    bool isCollapsedCell( double nodeNearTolerance = 1e-3 ) const;

private:
    std::array<size_t, 8> m_cornerIndices;

    size_t        m_gridLocalCellIndex; ///< This cells index in the grid it belongs to.
    RigGridBase*  m_hostGrid;
    RigLocalGrid* m_subGrid;

    size_t m_parentCellIndex; ///< Grid cell index of the cell in the parent grid containing this cell
    size_t m_mainGridCellIndex;

    size_t m_coarseningBoxIndex; ///< If defined, index into list of coarsening boxes in RigGridBase

    bool m_cellFaceFaults[6];

    bool m_isInvalid;
};
