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

#include <vector>
#include "RigCell.h"
#include "cvfVector3.h"
#include "cvfAssert.h"
#include "cvfObject.h"
#include "RigMainGrid.h"
#include "RigWellResults.h"
#include "RigActiveCellInfo.h"


class RigReservoir: public cvf::Object
{
public:
    RigReservoir();
    ~RigReservoir();

    RigMainGrid*            mainGrid() { return m_mainGrid.p(); }
    const RigMainGrid*      mainGrid() const { return m_mainGrid.p(); }

    void                    allGrids(std::vector<RigGridBase*>* grids);
    void                    allGrids(std::vector<const RigGridBase*>* grids) const;
    const RigGridBase*      grid(size_t index) const;

    void                    computeCachedData();

    void                    setWellResults(const cvf::Collection<RigWellResults>& data);
    const cvf::Collection<RigWellResults>&  wellResults() { return m_wellResults; }


    cvf::UByteArray*        wellCellsInGrid(size_t gridIndex);

    RigCell&                cellFromWellResultCell(const RigWellResultCell& wellResultCell);
    bool                    findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace, const RigWellResultCell& sourceWellCellResult, const RigWellResultCell& otherWellCellResult) const;

    RigActiveCellInfo*      activeCellInfo();


private:
    void                    computeFaults();
    void                    computeActiveCellData();
    void                    computeWellCellsPrGrid();

private:
    RigActiveCellInfo                   m_activeCellInfo;
    cvf::ref<RigMainGrid>               m_mainGrid;

    cvf::Collection<RigWellResults>     m_wellResults;
    cvf::Collection<cvf::UByteArray>    m_wellCellsInGrid;
};
