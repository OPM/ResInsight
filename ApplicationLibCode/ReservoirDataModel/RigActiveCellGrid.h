/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigMainGrid.h"

#include <vector>

class RigActiveCellGrid : public RigMainGrid
{
public:
    RigActiveCellGrid();
    ~RigActiveCellGrid() override;

    RigCell&       cell( size_t gridLocalCellIndex ) override;
    const RigCell& cell( size_t gridLocalCellIndex ) const override;

    size_t totalCellCount() const override;
    size_t totalActiveCellCount() const;

    std::vector<size_t> activeLocalCellIndices( bool skipInvalidCells ) const;

protected: // only for use by file readers!
    friend class RifReaderOpmCommonActive;
    std::map<size_t, RigCell>&       nativeCells();
    const std::map<size_t, RigCell>& nativeCells() const;
    void                             setTotalCellCount( size_t totalCellCount );
    void                             setTotalActiveCellCount( size_t totalActiveCellCount );

private:
    size_t                    m_totalCellCount;
    size_t                    m_totalActiveCellCount;
    RigCell                   m_invalidCell;
    std::map<size_t, RigCell> m_nativeCells;
};
