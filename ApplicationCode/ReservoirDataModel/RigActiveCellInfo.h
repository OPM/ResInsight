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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <vector>


class RigActiveCellInfo : public cvf::Object
{
public:
    RigActiveCellInfo();

    void                setReservoirCellCount(size_t reservoirCellCount);
    size_t              reservoirCellCount() const;
    size_t              reservoirActiveCellCount() const;
    size_t              reservoirCellResultCount() const;
    bool                isCoarseningActive() const;

    bool                isActive(size_t reservoirCellIndex) const;
    size_t              cellResultIndex(size_t reservoirCellIndex) const;
    void                setCellResultIndex(size_t reservoirCellIndex, size_t globalResultCellIndex);

    void                setGridCount(size_t gridCount);
    void                setGridActiveCellCounts(size_t gridIndex, size_t activeCellCount);
    void                gridActiveCellCounts(size_t gridIndex, size_t& activeCellCount) const;
    void                computeDerivedData();

    void                setIJKBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max);
    void                IJKBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const;

    cvf::BoundingBox    geometryBoundingBox() const;
    void                setGeometryBoundingBox(cvf::BoundingBox bb);

    void                clear();

    void                addLgr(int lgrStartIndex, int cellCount);

private:
    class GridActiveCellCounts
    {
    public:
        GridActiveCellCounts();

        size_t          activeCellCount() const;
        void            setActiveCellCount(size_t activeCellCount);

    private:
        size_t          m_activeCellCount;
    };


private:
    std::vector<GridActiveCellCounts>   m_perGridActiveCellInfo;

    std::vector<size_t>                 m_cellIndexToResultIndex;

    size_t                              m_reservoirActiveCellCount;
    size_t                              m_reservoirCellResultCount;

    cvf::Vec3st                         m_activeCellPositionMin;
    cvf::Vec3st                         m_activeCellPositionMax;

    cvf::BoundingBox                    m_activeCellsBoundingBox;
};
