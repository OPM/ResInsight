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
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

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
// Compute average measured depth for cell based on grid intersections for cells. If the well path geometry do not contain measured depth
// for a grid cell, the measured depth is estimated based on existing geometry for the well path.
//--------------------------------------------------------------------------------------------------
std::vector<double> RifReaderRftInterface::computeMeasuredDepth( const QString& wellName, const QDateTime& timeStep, RigEclipseWellLogExtractor* eclExtractor )
{
    if ( !eclExtractor ) return {};
    if ( !eclExtractor->caseData() ) return {};

    if ( eclExtractor->cellIntersectionMDs().empty() ) return {};

    auto mainGrid = eclExtractor->caseData()->mainGrid();
    if ( !mainGrid ) return {};

    std::vector<double> avgMeasuredDepthForCells;
    std::vector<double> tvdValuesToEstimate;

    auto cellIjk = cellIndices( wellName, timeStep );
    for ( const caf::VecIjk& ijk : cellIjk )
    {
        auto globalCellIndex = mainGrid->cellIndexFromIJK( ijk.i(), ijk.j(), ijk.k() );

        auto avgMd = eclExtractor->averageMdForCell( globalCellIndex );
        if ( avgMd.has_value() )
        {
            avgMeasuredDepthForCells.push_back( avgMd.value() );
        }
        else
        {
            // The RFT cell is not part of cells intersected by well path
            // Use the TVD of cell center to estimate measured depth

            avgMeasuredDepthForCells.push_back( std::numeric_limits<double>::infinity() );
            auto center = mainGrid->cell( globalCellIndex ).center();
            tvdValuesToEstimate.push_back( -center.z() );
        }
    }

    if ( !tvdValuesToEstimate.empty() )
    {
        auto wellPathMd  = eclExtractor->wellPathGeometry()->measuredDepths();
        auto wellPathTvd = eclExtractor->wellPathGeometry()->trueVerticalDepths();

        auto estimatedMeasuredDepth = RigWellPathGeometryTools::interpolateMdFromTvd( wellPathMd, wellPathTvd, tvdValuesToEstimate );

        double previousMd = std::numeric_limits<double>::infinity();

        size_t estimatedIndex = 0;
        for ( auto& measuredDepth : avgMeasuredDepthForCells )
        {
            if ( measuredDepth == std::numeric_limits<double>::infinity() )
            {
                if ( estimatedIndex < estimatedMeasuredDepth.size() )
                {
                    double candidateMd = estimatedMeasuredDepth[estimatedIndex++];
                    if ( previousMd != std::numeric_limits<double>::infinity() )
                    {
                        if ( previousMd < candidateMd )
                        {
                            measuredDepth = candidateMd;
                        }
                        else
                        {
                            measuredDepth = previousMd + 1.0;
                        }
                    }
                }
                else
                {
                    measuredDepth = previousMd + 1.0;
                }
            }

            previousMd = measuredDepth;
        }
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
