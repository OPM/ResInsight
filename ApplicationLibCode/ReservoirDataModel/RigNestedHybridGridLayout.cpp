/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigNestedHybridGridLayout.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNestedHybridGridLayout::RigNestedHybridGridLayout( const cvf::Vec3st& originalCellCounts, size_t kFactor )
    : m_originalCellCounts( originalCellCounts )
    , m_kFactor( std::max( kFactor, size_t( 1 ) ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3st& RigNestedHybridGridLayout::originalCellCounts() const
{
    return m_originalCellCounts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNestedHybridGridLayout::kFactor() const
{
    return m_kFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk0> RigNestedHybridGridLayout::originalIjkFromFlatIjk( const caf::VecIjk0& flatIjk ) const
{
    const size_t ok = flatIjk.z() / m_kFactor;
    if ( flatIjk.x() >= m_originalCellCounts.x() || flatIjk.y() >= m_originalCellCounts.y() || ok >= m_originalCellCounts.z() )
        return std::nullopt;

    return caf::VecIjk0( flatIjk.x(), flatIjk.y(), ok );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RigNestedHybridGridLayout::flatIjkFromOriginalIjk( const caf::VecIjk0& originalIjk ) const
{
    return caf::VecIjk0( originalIjk.x(), originalIjk.y(), originalIjk.z() * m_kFactor );
}
