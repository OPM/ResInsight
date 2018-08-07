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

#include "RifReaderInterface.h"
#include "RigFault.h"

#include "cvfBase.h"

#include "cvfVector3.h"
#include "cvfBoundingBox.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <vector>
#include <string>


class RigMainGrid;
class RigCell;
class RigActiveCellInfo;

class RigGridBase : public cvf::StructGridInterface
{
public:
    explicit RigGridBase(RigMainGrid* mainGrid);
    virtual ~RigGridBase();

    void                        setGridPointDimensions(const cvf::Vec3st& gridDimensions)   { m_gridPointDimensions = gridDimensions;}
    cvf::Vec3st                 gridPointDimensions()                                       { return m_gridPointDimensions; }

    size_t                      cellCount() const { return cellCountI() * cellCountJ() * cellCountK(); }
    RigCell&                    cell(size_t gridLocalCellIndex);
    const RigCell&              cell(size_t gridLocalCellIndex) const;
    
    size_t                      reservoirCellIndex(size_t gridLocalCellIndex) const;
    void                        setIndexToStartOfCells(size_t indexToStartOfCells) { m_indexToStartOfCells = indexToStartOfCells; }
    
    void                        setGridIndex(size_t index) { m_gridIndex = index; }
    size_t                      gridIndex() const { return m_gridIndex; }

    void                        setGridId(int id) { m_gridId = id; }
    int                         gridId() const { return m_gridId; }

    double                      characteristicIJCellSize() const;

    std::string                 gridName() const;
    void                        setGridName(const std::string& gridName);

    bool                        isMainGrid() const;
    RigMainGrid*                mainGrid() const { return m_mainGrid; }

    size_t                      coarseningBoxCount() const { return m_coarseningBoxInfo.size(); }
    size_t                      addCoarseningBox(size_t i1, size_t i2, size_t j1, size_t j2, size_t k1, size_t k2);

    void                        coarseningBox(size_t coarseningBoxIndex, size_t* i1, size_t* i2, size_t* j1, size_t* j2, size_t* k1, size_t* k2) const;

    cvf::BoundingBox            boundingBox();

  
protected:
    friend class RigMainGrid;//::initAllSubGridsParentGridPointer();
    void                        initSubGridParentPointer();
    void                        initSubCellsMainGridCellIndex();

    // Interface implementation
public:
    size_t gridPointCountI() const override;
    size_t gridPointCountJ() const override;
    size_t gridPointCountK() const override;

    cvf::Vec3d minCoordinate() const override;
    cvf::Vec3d maxCoordinate() const override;
    cvf::Vec3d displayModelOffset() const override;

    size_t cellIndexFromIJK( size_t i, size_t j, size_t k ) const override;
    bool ijkFromCellIndex( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const override;

    bool cellIJKFromCoordinate( const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k ) const override;
    void cellCornerVertices( size_t cellIndex, cvf::Vec3d vertices[8] ) const override;
    cvf::Vec3d cellCentroid( size_t cellIndex ) const override;

    void cellMinMaxCordinates( size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate ) const override;

    size_t gridPointIndexFromIJK( size_t i, size_t j, size_t k ) const override;
    cvf::Vec3d gridPointCoordinate( size_t i, size_t j, size_t k ) const override;

    bool isCellValid( size_t i, size_t j, size_t k ) const override;
    bool cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex ) const override;

private:
    std::string                 m_gridName;
    cvf::Vec3st                 m_gridPointDimensions;
    size_t                      m_indexToStartOfCells; ///< Index into the global cell array stored in main-grid where this grids cells starts.
    size_t                      m_gridIndex; ///< The LGR index of this grid. Starts with 1. Main grid has index 0.
    int                         m_gridId; ///< The LGR id of this grid. Main grid has id 0.
    RigMainGrid*                m_mainGrid;
    cvf::BoundingBox            m_boundingBox;

    std::vector<std::array<size_t, 6>>    m_coarseningBoxInfo;

};


class RigGridCellFaceVisibilityFilter : public cvf::CellFaceVisibilityFilter
{
public:
    explicit RigGridCellFaceVisibilityFilter(const RigGridBase* grid)
        :   m_grid(grid)
    {
    }

    bool isFaceVisible( size_t i, size_t j, size_t k, cvf::StructGridInterface::FaceType face, const cvf::UByteArray* cellVisibility ) const override;

private:
    const RigGridBase* m_grid;
};
