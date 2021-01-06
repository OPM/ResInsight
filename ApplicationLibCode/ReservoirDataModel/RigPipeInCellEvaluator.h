/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cvfArray.h"
#include "cvfObject.h"

class RigPipeInCellEvaluator : public cvf::Object
{
public:
    RigPipeInCellEvaluator( const std::vector<cvf::ubyte>& isWellPipeVisibleForResultWellIndex,
                            const cvf::UIntArray*          gridCellToResultWellIndexMap )
        : m_isWellPipeVisibleForWellIndex( isWellPipeVisibleForResultWellIndex )
        , m_gridCellToWellIndexMap( gridCellToResultWellIndexMap )
    {
    }

    bool isWellPipeInCell( size_t cellIndex ) const
    {
        cvf::uint wellIndex = m_gridCellToWellIndexMap->get( cellIndex );

        if ( wellIndex == cvf::UNDEFINED_UINT )
        {
            return false;
        }

        return m_isWellPipeVisibleForWellIndex[wellIndex];
    }

private:
    const std::vector<cvf::ubyte>& m_isWellPipeVisibleForWellIndex;
    const cvf::UIntArray*          m_gridCellToWellIndexMap;
};
