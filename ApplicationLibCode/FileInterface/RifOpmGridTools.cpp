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

        transferCoordinates( opmMainGrid, riMainGrid, riMainGrid );

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

                transferCoordinates( opmLgrGrid, riMainGrid, riLgrGrid );
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
void RifOpmGridTools::transferCoordinates( Opm::EclIO::EGrid& opmGrid, RigMainGrid* riMainGrid, RigGridBase* riGrid )
{
    size_t cellCount = opmGrid.totalNumberOfCells();
    if ( cellCount != riGrid->cellCount() ) return;

    // Ordering of corner nodes are different on file compared to ResInsight data structures
    const size_t* cellMappingECLRi = RifReaderEclipseOutput::eclipseCellIndexMapping();

    std::array<double, 8> X;
    std::array<double, 8> Y;
    std::array<double, 8> Z;

    auto& nodes = riMainGrid->nodes();

    for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
    {
        opmGrid.getCellCorners( cIdx, X, Y, Z );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto nodeStartIndex = riGrid->reservoirCellIndex( cIdx ) * 8;

        for ( size_t i = 0; i < 8; i++ )
        {
            // std::cout << X[i] << " " << Y[i] << " " << Z[i] << "\n";

            size_t nodeIndex = nodeStartIndex + cellMappingECLRi[i];

            auto& node = nodes[nodeIndex];
            node.x()   = X[i];
            node.y()   = Y[i];
            node.z()   = -Z[i];
        }
    }
}
