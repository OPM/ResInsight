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
void RifOpmGridTools::importAndUpdateCoordinates( const std::string& gridFilePath, RigMainGrid* mainGrid )
{
    CAF_ASSERT( mainGrid );

    //    auto filePath = "e:/models/from_equinor_sftp/2021.10.1-Radial-testcase/global-radial/GLOBAL-RADIAL.EGRID";

    const size_t* cellMappingECLRi = RifReaderEclipseOutput::eclipseCellIndexMapping();

    try
    {
        Opm::EclIO::EGrid eGrid( gridFilePath );

        size_t cellCount = eGrid.totalNumberOfCells();

        if ( cellCount != mainGrid->cellCount() ) return;

        auto dims     = eGrid.dimension();
        bool isRadial = eGrid.is_radial();

        std::array<double, 8> X;
        std::array<double, 8> Y;
        std::array<double, 8> Z;

        auto&  cornerNodes    = mainGrid->nodes();
        size_t nodeStartIndex = 0;

        for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
        {
            eGrid.getCellCorners( cIdx, X, Y, Z );

            for ( size_t i = 0; i < 8; i++ )
            {
                // std::cout << X[i] << " " << Y[i] << " " << Z[i] << "\n";

                size_t nodeIndex = nodeStartIndex + cIdx * 8 + cellMappingECLRi[i];

                auto& node = cornerNodes[nodeIndex];
                node.x()   = X[i];
                node.y()   = Y[i];
                node.z()   = -Z[i];
            }
        }
    }
    catch ( ... )
    {
    }
}
