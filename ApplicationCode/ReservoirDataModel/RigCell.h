/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cafFixedArray.h"

namespace cvf
{
    class Ray;
}



class RigCell
{
public:
    RigCell();
    ~RigCell(); // Not virtual, to save space. Do not inherit from this class

    caf::SizeTArray8&       cornerIndices()                                     { return m_cornerIndices;}
    const caf::SizeTArray8& cornerIndices() const                               { return m_cornerIndices;}

    bool                    isInvalid() const                                   { return m_isInvalid; }
    void                    setInvalid( bool val )                              { m_isInvalid = val; }

    size_t                  cellIndex() const                                   { return m_cellIndex; }
    void                    setCellIndex(size_t val)                            { m_cellIndex = val; }

    RigLocalGrid*           subGrid() const                                     { return m_subGrid; }
    void                    setSubGrid(RigLocalGrid* subGrid)                   { m_subGrid = subGrid; }

    RigGridBase*            hostGrid() const                                    { return m_hostGrid; }
    void                    setHostGrid(RigGridBase* hostGrid)                  { m_hostGrid = hostGrid; }

    size_t                  parentCellIndex() const                             { return m_parentCellIndex; }
    void                    setParentCellIndex(size_t parentCellIndex)          { m_parentCellIndex = parentCellIndex; }
    size_t                  mainGridCellIndex() const                           { return m_mainGridCellIndex; }
    void                    setMainGridCellIndex(size_t mainGridCellContainingThisCell) { m_mainGridCellIndex = mainGridCellContainingThisCell; }

    bool                    isInCoarseCell() const                              { return m_isInCoarseCell; }
    void                    setInCoarseCell(bool isInCoarseCell)                { m_isInCoarseCell = isInCoarseCell; }

    void                    setCellFaceFault(cvf::StructGridInterface::FaceType face)       { m_cellFaceFaults[face] = true; }
    bool                    isCellFaceFault(cvf::StructGridInterface::FaceType face) const  { return m_cellFaceFaults[face]; }

    cvf::Vec3d              center() const;
    cvf::Vec3d              faceCenter(cvf::StructGridInterface::FaceType face) const;
    bool                    firstIntersectionPoint(const cvf::Ray& ray, cvf::Vec3d* intersectionPoint) const;
    bool                    isLongPyramidCell(double maxHeightFactor = 5, double nodeNearTolerance = 1e-3 ) const;
private:
    caf::SizeTArray8        m_cornerIndices;

    size_t                  m_cellIndex;                ///< This cells index in the grid it belongs to.
    RigGridBase*            m_hostGrid;
    RigLocalGrid*           m_subGrid;

    size_t                  m_parentCellIndex; ///< Grid cell index of the cell in the parent grid containing this cell
    size_t                  m_mainGridCellIndex;
    bool                    m_isInCoarseCell; 

    bool                    m_cellFaceFaults[6];

    bool                    m_isInvalid;
};
