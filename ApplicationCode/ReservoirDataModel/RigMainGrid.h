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
#include "RigGridBase.h"
#include <vector>
#include "RigCell.h"
#include "RigLocalGrid.h"
#include "cvfCollection.h"
#include "cvfBoundingBox.h"
#include "RifReaderInterface.h"

#include <QtGlobal>

class RigMainGrid : public RigGridBase
{
public:
    RigMainGrid();
    virtual ~RigMainGrid();

public:
    std::vector<cvf::Vec3d>&                nodes() {return m_nodes;}
    const std::vector<cvf::Vec3d>&          nodes() const {return m_nodes;}

    std::vector<RigCell>&                   cells() {return m_cells;}
    const std::vector<RigCell>&             cells() const {return m_cells;}

    void                                    addLocalGrid(RigLocalGrid* localGrid);
    size_t                                  gridCount() const           { return m_localGrids.size() + 1; }
    RigGridBase*                            gridByIndex(size_t localGridIndex);
    const RigGridBase*                      gridByIndex(size_t localGridIndex) const;
    
    void                                    computeCachedData();

    cvf::BoundingBox                        matrixModelActiveCellsBoundingBox() const;

    // Overrides
    virtual cvf::Vec3d                      displayModelOffset() const;
    void                                    setDisplayModelOffset(cvf::Vec3d offset);

    void                                    setFlipAxis(bool flipXAxis, bool flipYAxis);

private:
    void                                    initAllSubGridsParentGridPointer();
    void                                    initAllSubCellsMainGridCellIndex();
    void                                    computeActiveAndValidCellRanges();

private:
    std::vector<cvf::Vec3d>                 m_nodes;        ///< Global vertex table
    std::vector<RigCell>                    m_cells;        ///< Global array of all cells in the reservoir (including the ones in LGR's)
    cvf::Collection<RigLocalGrid>           m_localGrids;   ///< List of all the LGR's in this reservoir

    cvf::Vec3d                              m_displayModelOffset;

    bool                                    m_flipXAxis;
    bool                                    m_flipYAxis;
};

