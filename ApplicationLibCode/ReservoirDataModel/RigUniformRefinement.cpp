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

#include "RigUniformRefinement.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigUniformRefinement::RigUniformRefinement( const cvf::Vec3st& refinement, const cvf::Vec3st& sectorSize )
    : m_sectorSize( sectorSize )
    , m_refinement( refinement )
{
    for ( auto dim : { DimI, DimJ, DimK } )
    {
        m_cachedFractions[dim] = generateEqualFractions( m_refinement[dim] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigRefinement> RigUniformRefinement::clone() const
{
    return std::make_unique<RigUniformRefinement>( *this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigUniformRefinement::subcellCount( Dimension dim, size_t origIndex ) const
{
    if ( origIndex >= m_sectorSize[dim] ) return 1;
    return m_refinement[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigUniformRefinement::cumulativeOffset( Dimension dim, size_t origIndex ) const
{
    if ( origIndex > m_sectorSize[dim] ) return 0;
    return origIndex * m_refinement[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigUniformRefinement::totalRefinedCount( Dimension dim ) const
{
    return m_sectorSize[dim] * m_refinement[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigUniformRefinement::mapRefinedToOriginal( Dimension dim, size_t refinedIndex ) const
{
    if ( m_sectorSize[dim] == 0 ) return { 0, 0 };
    return { refinedIndex / m_refinement[dim], refinedIndex % m_refinement[dim] };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigUniformRefinement::cumulativeFractions( Dimension dim, size_t /*origIndex*/ ) const
{
    return m_cachedFractions[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigUniformRefinement::sectorSize( Dimension dim ) const
{
    return m_sectorSize[dim];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigUniformRefinement::hasRefinement() const
{
    return m_refinement.x() > 1 || m_refinement.y() > 1 || m_refinement.z() > 1;
}
