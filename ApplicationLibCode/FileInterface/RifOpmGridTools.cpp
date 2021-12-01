/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RifOpmGridTools.h"

#include "RigMainGrid.h"

#include "../Application/Tools/RiaWeightedMeanCalculator.h"
#include "RifReaderEclipseOutput.h"
#include "opm/io/eclipse/EGrid.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::importAndUpdateCoordinates( const std::string& gridFilePath, RigMainGrid* riMainGrid )
{
    CAF_ASSERT( riMainGrid );

    try
    {
        Opm::EclIO::EGrid opmMainGrid( gridFilePath );

        transferCoordinates( opmMainGrid, opmMainGrid, riMainGrid, riMainGrid );

        auto lgrNames = opmMainGrid.list_of_lgrs();
        for ( const auto& lgrName : lgrNames )
        {
            RigGridBase* riLgrGrid = nullptr;

            for ( size_t i = 0; i < riMainGrid->gridCount(); i++ )
            {
                auto candidate = riMainGrid->gridByIndex( i );
                if ( candidate->gridName() == lgrName )
                {
                    riLgrGrid = candidate;
                }
            }

            if ( riLgrGrid )
            {
                Opm::EclIO::EGrid opmLgrGrid( gridFilePath, lgrName );

                transferCoordinates( opmMainGrid, opmLgrGrid, riMainGrid, riLgrGrid );
            }
        }
    }
    catch ( ... )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmGridTools::transferCoordinates( Opm::EclIO::EGrid& opmMainGrid,
                                           Opm::EclIO::EGrid& opmGrid,
                                           RigMainGrid*       riMainGrid,
                                           RigGridBase*       riGrid )
{
    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return;

    // Ordering of corner nodes are different on file compared to ResInsight data structures
    const size_t* cellMappingECLRi = RifReaderEclipseOutput::eclipseCellIndexMapping();

    std::array<double, 8> X;
    std::array<double, 8> Y;
    std::array<double, 8> Z;

    auto& nodes = riMainGrid->nodes();

    // Compute the center of the LGR radial grid cells for each K layer
    std::map<int, std::pair<double, double>> radialGridCenterTopLayer;

    if ( riGrid != riMainGrid )
    {
        auto hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();

        std::map<int, std::vector<std::pair<double, double>>> xyCenterPerLayer;

        for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
        {
            bool useCartesianCoords = false;
            auto mainGridCellIndex  = hostCellGlobalIndices[cIdx];
            opmMainGrid.getCellCorners( mainGridCellIndex, X, Y, Z, useCartesianCoords );

            auto ijkLocalGrid = opmGrid.ijk_from_global_index( cIdx );
            auto layer        = ijkLocalGrid[2];
            
            // Four corners for top
            for ( size_t i = 4; i < 8; i++ )
            {
                auto& xyCoords = xyCenterPerLayer[layer];
                xyCoords.push_back( { X[i], Y[i] } );
            }
        }

        for ( const auto [k, xyCoords] : xyCenterPerLayer )
        {
            RiaWeightedMeanCalculator<double> xCoord;
            RiaWeightedMeanCalculator<double> yCoord;

            for ( const auto [x, y] : xyCoords )
            {
                xCoord.addValueAndWeight( x, 1.0 );
                yCoord.addValueAndWeight( y, 1.0 );
            }

            radialGridCenterTopLayer[k] = { xCoord.weightedMean(), yCoord.weightedMean() };
        }
    }

    for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
    {
        opmGrid.getCellCorners( cIdx, X, Y, Z, false );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto nodeStartIndex = riGrid->reservoirCellIndex( cIdx ) * 8;
        auto ijk            = opmGrid.ijk_from_global_index( cIdx );

        double xCenterCoord = 0.0;
        double yCenterCoord = 0.0;

        if ( radialGridCenterTopLayer.count( ijk[2] ) > 0 )
        {
            auto [xCenter, yCenter] = radialGridCenterTopLayer[ijk[2]];
            xCenterCoord            = xCenter;
            yCenterCoord            = yCenter;
        }

        for ( size_t i = 0; i < 8; i++ )
        {
            // std::cout << X[i] << " " << Y[i] << " " << Z[i] << "\n";

            size_t nodeIndex = nodeStartIndex + cellMappingECLRi[i];

            auto& node = nodes[nodeIndex];
            node.x()   = X[i] + xCenterCoord;
            node.y()   = Y[i] + yCenterCoord;
            node.z()   = -Z[i];
        }
    }
}
