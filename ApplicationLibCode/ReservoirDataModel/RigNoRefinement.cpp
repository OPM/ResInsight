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

#include "RigNoRefinement.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNoRefinement::RigNoRefinement( const cvf::Vec3st& sectorSize )
    : m_sectorSize( sectorSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigRefinement> RigNoRefinement::clone() const
{
    return std::make_unique<RigNoRefinement>( *this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNoRefinement::subcellCount( Dimension /*dim*/, size_t /*origIndex*/ ) const
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNoRefinement::cumulativeOffset( Dimension dim, size_t origIndex ) const
{
    if ( origIndex > m_sectorSize[dim] ) return 0;
    return origIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNoRefinement::totalRefinedCount( Dimension dim ) const
{
    return m_sectorSize[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigNoRefinement::mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const
{
    if ( m_sectorSize[dim] == 0 ) return { 0, 0 };
    return { refinedIndex, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigNoRefinement::cumulativeFractions( Dimension /*dim*/, size_t /*origIndex*/ ) const
{
    static const std::vector<double> defaultFraction = { 1.0 };
    return defaultFraction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNoRefinement::sectorSize( Dimension dim ) const
{
    return m_sectorSize[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNoRefinement::hasRefinement() const
{
    return false;
}
