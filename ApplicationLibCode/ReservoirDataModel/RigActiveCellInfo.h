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

#include "RigTypeSafeIndex.h"

#include "cafVecIjk.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"

#include <vector>

class RigActiveCellInfo : public cvf::Object
{
public:
    RigActiveCellInfo();

    void   setReservoirCellCount( size_t reservoirCellCount );
    size_t reservoirCellCount() const;
    size_t reservoirActiveCellCount() const;

    bool                            isActive( size_t reservoirCellIndex ) const;
    size_t                          cellResultIndex( size_t reservoirCellIndex ) const;
    void                            setCellResultIndex( size_t reservoirCellIndex, size_t globalResultCellIndex );
    std::vector<ReservoirCellIndex> activeReservoirCellIndices() const;

    void   setGridCount( size_t gridCount );
    void   setGridActiveCellCounts( size_t gridIndex, size_t activeCellCount );
    size_t gridActiveCellCounts( size_t gridIndex ) const;
    void   computeDerivedData();

    void                                  setIjkBoundingBox( const caf::VecIjk0& min, const caf::VecIjk0& max );
    std::pair<caf::VecIjk0, caf::VecIjk0> ijkBoundingBox() const;

    cvf::BoundingBox geometryBoundingBox() const;
    void             setGeometryBoundingBox( cvf::BoundingBox bb );

    void clear();

    void addLgr( size_t cellCount );

private:
    class GridActiveCellCounts
    {
    public:
        GridActiveCellCounts();

        size_t activeCellCount() const;
        void   setActiveCellCount( size_t activeCellCount );

    private:
        size_t m_activeCellCount;
    };

private:
    std::vector<GridActiveCellCounts> m_perGridActiveCellInfo;

    std::vector<size_t>             m_cellIndexToResultIndex;
    std::vector<ReservoirCellIndex> m_activeCellIndices;

    size_t m_reservoirActiveCellCount;

    caf::VecIjk0 m_activeCellPositionMin;
    caf::VecIjk0 m_activeCellPositionMax;

    cvf::BoundingBox m_activeCellsBoundingBox;
};
