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
#include "RigMainGrid.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathGeometryTools.h"

#include "cafVecIjk.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface::~RifReaderRftInterface() = default;

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
std::vector<double>
    RifReaderRftInterface::computeMeasuredDepth( const QString& wellName, const QDateTime& timeStep, RigEclipseWellLogExtractor* eclExtractor )
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

        // Estimate measured depth for cells that do not have  measured depth
        auto estimatedMeasuredDepth = RigWellPathGeometryTools::interpolateMdFromTvd( wellPathMd, wellPathTvd, tvdValuesToEstimate );

        double previousMd = std::numeric_limits<double>::infinity();

        // Replace infinity MD values with estimated MD values based on well path geometry
        // previousMd contains the last known measured depth value as we move along the well path

        size_t estimatedIndex = 0;
        for ( auto& measuredDepth : avgMeasuredDepthForCells )
        {
            if ( measuredDepth == std::numeric_limits<double>::infinity() )
            {
                // No measured depth for cell is found, try to estimate MD based on MD for previous cell
                if ( estimatedIndex < estimatedMeasuredDepth.size() )
                {
                    double estimatedMd = estimatedMeasuredDepth[estimatedIndex++];
                    if ( previousMd != std::numeric_limits<double>::infinity() )
                    {
                        if ( previousMd < estimatedMd )
                        {
                            // The estimated MD is larger than previous MD, use the estimated MD
                            measuredDepth = estimatedMd;
                        }
                        else
                        {
                            // The estimated MD is smaller than previous MD, use the previous MD + 1.0
                            measuredDepth = previousMd + 1.0;
                        }
                    }
                    else
                    {
                        // We do not have a valid previous MD, use the estimated MD
                        measuredDepth = estimatedMd;
                    }
                }
                else
                {
                    measuredDepth = previousMd + 1.0;
                }
            }

            // Assign the current MD as previous MD to be used for next estimated MD
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
