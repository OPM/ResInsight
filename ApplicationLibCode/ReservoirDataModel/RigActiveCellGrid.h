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

    // size_t transferActiveInformation( int                     gridIndex,
    //                                   RigEclipseCaseData*     eclipseCaseData,
    //                                   size_t                  totalActiveCells,
    //                                   size_t                  matrixActiveCells,
    //                                   size_t                  fractureActiveCells,
    //                                   const std::vector<int>& activeMatrixIndexes,
    //                                   const std::vector<int>& activeFracIndexes,
    //                                   size_t                  inactiveCellIndex );

    // size_t cellIndexFromIJK( size_t i, size_t j, size_t k ) const override;
    // size_t cellIndexFromIJKUnguarded( size_t i, size_t j, size_t k ) const override;
    // bool   ijkFromCellIndex( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const override;
    // void   ijkFromCellIndexUnguarded( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const override;

    RigCell&       cell( size_t gridLocalCellIndex ) override;
    const RigCell& cell( size_t gridLocalCellIndex ) const override;
    // size_t         cellCount() const override;

    size_t totalCellCount() const override;

public: // only for use by file readers!
    std::map<size_t, RigCell>&       nativeCells();
    const std::map<size_t, RigCell>& nativeCells() const;
    void                             setTotalCellCount( size_t totalCellCount );

private:
    // std::vector<size_t>       m_globalToNativeMap;
    // std::vector<size_t>       m_nativeToGlobalMap;
    size_t                    m_totalCellCount;
    RigCell                   m_invalidCell;
    std::map<size_t, RigCell> m_nativeCells;
};
