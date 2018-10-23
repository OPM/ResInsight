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


#include "RigCell.h"
#include "RigGridBase.h"
#include "RigLocalGrid.h"
#include "RigNNCData.h"

#include "cvfBoundingBox.h"
#include "cvfCollection.h"

#include <vector>

class RigActiveCellInfo;

namespace cvf
{
    class BoundingBoxTree;
}


class RigMainGrid : public RigGridBase
{
public:
    RigMainGrid();
    ~RigMainGrid() override;

public:
    std::vector<cvf::Vec3d>&                nodes();
    const std::vector<cvf::Vec3d>&          nodes() const;

    std::vector<RigCell>&                   globalCellArray();
    const std::vector<RigCell>&             globalCellArray() const;

    RigGridBase*                            gridAndGridLocalIdxFromGlobalCellIdx(size_t globalCellIdx, size_t* gridLocalCellIdx);
    const RigGridBase*                      gridAndGridLocalIdxFromGlobalCellIdx(size_t globalCellIdx, size_t* gridLocalCellIdx) const;

    const RigCell&                          cellByGridAndGridLocalCellIdx(size_t gridIdx, size_t gridLocalCellIdx) const;
    size_t                                  reservoirCellIndexByGridAndGridLocalCellIndex(size_t gridIdx, size_t gridLocalCellIdx) const;
    size_t                                  findReservoirCellIndexFromPoint(const cvf::Vec3d& point) const;
    std::vector<size_t>                     findAllReservoirCellIndicesMatching2dPoint(const cvf::Vec2d& point2d) const;
    void                                    addLocalGrid(RigLocalGrid* localGrid);
    size_t                                  gridCount() const;
    RigGridBase*                            gridByIndex(size_t localGridIndex);
    const RigGridBase*                      gridByIndex(size_t localGridIndex) const;
    RigGridBase*                            gridById(int localGridId);
   
    RigNNCData*                             nncData();
    void                                    setFaults(const cvf::Collection<RigFault>& faults);
    const cvf::Collection<RigFault>&        faults();
    void                                    calculateFaults(const RigActiveCellInfo* activeCellInfo, bool forceCalculation = false);

    void distributeNNCsToFaults();

    const RigFault*                         findFaultFromCellIndexAndCellFace(size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face) const;
    bool                                    isFaceNormalsOutwards() const;

    void                                    computeCachedData();
    void                                    initAllSubGridsParentGridPointer();

    cvf::Vec3d                              displayModelOffset() const override;
    void                                    setDisplayModelOffset(cvf::Vec3d offset);

    void                                    setFlipAxis(bool flipXAxis, bool flipYAxis);
    void                                    findIntersectingCells(const cvf::BoundingBox& inputBB, std::vector<size_t>* cellIndices) const;

    cvf::BoundingBox                        boundingBox() const;

private:
    void                                    initAllSubCellsMainGridCellIndex();
    void                                    buildCellSearchTree();
    bool                                    hasFaultWithName(const QString& name) const;

private:
    std::vector<cvf::Vec3d>                 m_nodes;        ///< Global vertex table
    std::vector<RigCell>                    m_cells;        ///< Global array of all cells in the reservoir (including the ones in LGR's)
    cvf::Collection<RigLocalGrid>           m_localGrids;   ///< List of all the LGR's in this reservoir
    std::vector<size_t>                     m_gridIdToIndexMapping; ///< Mapping from LGR Id to index.


    cvf::Collection<RigFault>               m_faults;
    cvf::ref<RigNNCData>                    m_nncData;
    cvf::ref<RigFaultsPrCellAccumulator>    m_faultsPrCellAcc;

    cvf::Vec3d                              m_displayModelOffset;
    cvf::ref<cvf::BoundingBoxTree>          m_cellSearchTree;
    mutable cvf::BoundingBox                m_boundingBox;    

    bool                                    m_flipXAxis;
    bool                                    m_flipYAxis;
};

