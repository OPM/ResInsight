/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifReaderRftInterface.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "cafVecIjk.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderRftInterface::eclipseRftAddresses( const QString& wellName, const QDateTime& timeStep )
{
    std::set<RifEclipseRftAddress> matchingAddresses;
    std::set<RifEclipseRftAddress> allAddresses = eclipseRftAddresses();
    for ( const RifEclipseRftAddress& address : allAddresses )
    {
        if ( address.wellName() == wellName && address.timeStep() == timeStep )
        {
            matchingAddresses.insert( address );
        }
    }
    return matchingAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifReaderRftInterface::getTvd( const QString&                                  wellName,
                                                   const QDateTime&                                timeStep,
                                                   const std::vector<RigEclipseWellLogExtractor*>& extractor )
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifReaderRftInterface::getTvd( const QString& wellName, const QDateTime& timeStep, RigEclipseWellLogExtractor* extractor )
{
    // return getTvd( wellName, timeStep, std::vector<RigEclipseWellLogExtractor*>( 1, extractor ) );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifReaderRftInterface::getMd( const QString&                                  wellName,
                                                  const QDateTime&                                timeStep,
                                                  const std::vector<RigEclipseWellLogExtractor*>& extractor )
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///  for all RFT cells
//   if cell is intersected
//   get measured depth for cell, compute average MD based on all intersections for cell
//--------------------------------------------------------------------------------------------------
std::vector<double> RifReaderRftInterface::getMd( const QString& wellName, const QDateTime& timeStep, RigEclipseWellLogExtractor* eclExtractor )
{
    if ( !eclExtractor ) return {};
    if ( !eclExtractor->caseData() ) return {};

    if ( eclExtractor->cellIntersectionMDs().empty() ) return {};

    auto mainGrid = eclExtractor->caseData()->mainGrid();
    if ( !mainGrid ) return {};

    std::vector<double> avgMeasuredDepthForCells;

    auto cellIjk = cellIndices( wellName, timeStep );
    for ( const caf::VecIjk& ijk : cellIjk )
    {
        auto globalCellIndex = mainGrid->cellIndexFromIJK( ijk.i(), ijk.j(), ijk.k() );

        auto avgMd = eclExtractor->averageMdForCell( globalCellIndex );
        if ( avgMd.has_value() )
        {
            avgMeasuredDepthForCells.push_back( avgMd.value() );
        }
        avgMeasuredDepthForCells.push_back( std::numeric_limits<double>::infinity() );
    }

    return avgMeasuredDepthForCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::VecIjk> RifReaderRftInterface::cellIndices( const QString& wellName, const QDateTime& timeStep )
{
    return {};
}
