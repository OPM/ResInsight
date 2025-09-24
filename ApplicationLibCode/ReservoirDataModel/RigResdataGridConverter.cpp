/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RigResdataGridConverter.h"

#include "ExportCommands/RicEclipseCellResultToFileImpl.h"

#include "RigEclipseCaseData.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"

#include "RifReaderEclipseOutput.h"

#include "cvfArray.h"
#include "cvfStructGrid.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include <array>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigResdataGridConverter::exportGrid( const QString&         resultFileName,
                                          RigEclipseCaseData*    eclipseCase,
                                          bool                   exportInLocalCoordinates,
                                          const cvf::UByteArray* cellVisibilityOverrideForActnum /*= nullptr*/,
                                          const cvf::Vec3st&     min /*= cvf::Vec3st::ZERO*/,
                                          const cvf::Vec3st&     max /*= cvf::Vec3st::UNDEFINED*/,
                                          const cvf::Vec3st&     refinement /*= cvf::Vec3st( 1, 1, 1 ) */ )
{
    if ( !eclipseCase ) return false;

    // Create grid export adapter that handles all the complexity
    RigGridExportAdapter gridAdapter( eclipseCase, min, max, refinement, cellVisibilityOverrideForActnum );

    size_t ni = gridAdapter.cellCountI();
    size_t nj = gridAdapter.cellCountJ();
    size_t nk = gridAdapter.cellCountK();

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    // Get coordinate transformation if needed for local coordinates export
    std::array<float, 6> mapAxes = gridAdapter.mapAxes();

    if ( gridAdapter.useMapAxes() && exportInLocalCoordinates )
    {
        const RigMainGrid* mainGrid = eclipseCase->mainGrid();
        cvf::Vec3d         minPoint3d( mainGrid->boundingBox().min() );
        cvf::Vec2f         minPoint2f( minPoint3d.x(), minPoint3d.y() );
        cvf::Vec2f         origin( mapAxes[2] - minPoint2f.x(), mapAxes[3] - minPoint2f.y() );
        cvf::Vec2f         xPoint = cvf::Vec2f( mapAxes[4], mapAxes[5] ) - minPoint2f;
        cvf::Vec2f         yPoint = cvf::Vec2f( mapAxes[0], mapAxes[1] ) - minPoint2f;
        mapAxes                   = { yPoint.x(), yPoint.y(), origin.x(), origin.y(), xPoint.x(), xPoint.y() };
    }

    // Use the new simplified interface
    convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

    // Write to file
    QFile exportFile( resultFileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        return false;
    }

    // Write SPECGRID
    {
        QTextStream out( &exportFile );
        out << "SPECGRID\n";
        out << "  " << ni << "  " << nj << "  " << nk << "  1  F /\n\n";
    }

    // Write COORD
    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "COORD";
        std::vector<double> coordArrayDouble;
        coordArrayDouble.reserve( coordArray.size() );
        for ( const auto& v : coordArray )
        {
            coordArrayDouble.push_back( v );
        }

        int valuesPerRow = 6;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, coordArrayDouble, valuesPerRow );
    }

    // Write ZCORN
    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "ZCORN";
        std::vector<double> zcornArrayDouble;
        zcornArrayDouble.reserve( zcornArray.size() );
        for ( const auto& v : zcornArray )
        {
            zcornArrayDouble.push_back( v );
        }

        int valuesPerRow = 6;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, zcornArrayDouble, valuesPerRow );
    }

    // Write ACTNUM
    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "ACTNUM";
        std::vector<double> actnumArrayDouble;
        actnumArrayDouble.reserve( actnumArray.size() );
        for ( const auto& v : actnumArray )
        {
            actnumArrayDouble.push_back( v );
        }

        int valuesPerRow = 10;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, actnumArrayDouble, valuesPerRow );
    }

    // Write MAPAXES if needed
    if ( gridAdapter.useMapAxes() )
    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "MAPAXES";
        std::vector<double> mapAxesDouble;
        for ( const auto& v : mapAxes )
        {
            mapAxesDouble.push_back( v );
        }

        int valuesPerRow = 2; // MAPAXES uses 2 values per row, not 6
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, mapAxesDouble, valuesPerRow );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Convert grid cells to Eclipse corner point arrays (COORD, ZCORN, and ACTNUM)
///
/// This function converts ResInsight grid data into the Eclipse corner point grid format:
/// - COORD array contains pillar coordinates (6 values per pillar: x1,y1,z1,x2,y2,z2)
/// - ZCORN array contains Z values for cell corners (8 Z values per cell)
/// - ACTNUM array contains activity flags (1 for active, 0 for inactive)
///
/// Uses RigGridExportAdapter to handle refinement and coordinate transformations uniformly.
/// See Eclipse_Grid_Format.md for details
///
//--------------------------------------------------------------------------------------------------
void RigResdataGridConverter::convertGridToCornerPointArrays( const RigGridExportAdapter& gridAdapter,
                                                              std::vector<float>&         coordArray,
                                                              std::vector<float>&         zcornArray,
                                                              std::vector<int>&           actnumArray )
{
    auto findBestCornerForPillar = []( size_t i, size_t j, const RigGridExportAdapter& gridAdapter ) -> std::pair<cvf::Vec3d, cvf::Vec3d>
    {
        const size_t nx = gridAdapter.cellCountI();
        const size_t ny = gridAdapter.cellCountJ();
        const size_t nz = gridAdapter.cellCountK();

        cvf::Vec3d topCoord( 0.0, 0.0, 0.0 );
        cvf::Vec3d bottomCoord( 0.0, 0.0, 0.0 );

        double maxDistance = -std::numeric_limits<double>::max();

        // Find pillar coordinates from adjacent cells using the adapter
        for ( int di = -1; di <= 0; ++di )
        {
            for ( int dj = -1; dj <= 0; ++dj )
            {
                int cellI = static_cast<int>( i ) + di;
                int cellJ = static_cast<int>( j ) + dj;

                if ( cellI >= 0 && cellI < static_cast<int>( nx ) && cellJ >= 0 && cellJ < static_cast<int>( ny ) )
                {
                    // Get corners for this cell (from top and bottom layers)
                    auto topCorners    = gridAdapter.getCellCorners( cellI, cellJ, 0 );
                    auto bottomCorners = gridAdapter.getCellCorners( cellI, cellJ, nz - 1 );

                    // Determine which corner corresponds to this pillar
                    size_t cornerIdx = 0;
                    if ( di == 0 && dj == 0 )
                        cornerIdx = 0; // cell's SW corner
                    else if ( di == -1 && dj == 0 )
                        cornerIdx = 1; // cell's SE corner
                    else if ( di == 0 && dj == -1 )
                        cornerIdx = 3; // cell's NW corner
                    else if ( di == -1 && dj == -1 )
                        cornerIdx = 2; // cell's NE corner

                    cvf::Vec3d candidateTopCoord    = topCorners[cornerIdx];
                    cvf::Vec3d candidateBottomCoord = bottomCorners[cornerIdx + 4];

                    double distance = candidateTopCoord.pointDistance( candidateBottomCoord );
                    if ( distance > maxDistance )
                    {
                        topCoord    = candidateTopCoord;
                        bottomCoord = candidateBottomCoord;
                        maxDistance = distance;
                    }
                }
            }
        }

        return { topCoord, bottomCoord };
    };

    const size_t nx = gridAdapter.cellCountI();
    const size_t ny = gridAdapter.cellCountJ();
    const size_t nz = gridAdapter.cellCountK();

    // Resize arrays to correct size
    const size_t coordSize  = ( nx + 1 ) * ( ny + 1 ) * 6;
    const size_t zcornSize  = nx * ny * nz * 8;
    const size_t actnumSize = nx * ny * nz;

    coordArray.resize( coordSize, 0.0f );
    zcornArray.resize( zcornSize, 0.0f );
    actnumArray.resize( actnumSize, 0 );

    // Generate COORD array - pillars for the refined grid
    for ( size_t j = 0; j <= ny; ++j )
    {
        for ( size_t i = 0; i <= nx; ++i )
        {
            const auto& [topCoord, bottomCoord] = findBestCornerForPillar( i, j, gridAdapter );

            // Store pillar coordinates in COORD array
            size_t pillarIndex = j * ( nx + 1 ) + i;
            size_t coordIndex  = pillarIndex * 6;

            coordArray[coordIndex + 0] = static_cast<float>( topCoord.x() );
            coordArray[coordIndex + 1] = static_cast<float>( topCoord.y() );
            coordArray[coordIndex + 2] = static_cast<float>( -topCoord.z() ); // Negate Z for Eclipse convention
            coordArray[coordIndex + 3] = static_cast<float>( bottomCoord.x() );
            coordArray[coordIndex + 4] = static_cast<float>( bottomCoord.y() );
            coordArray[coordIndex + 5] = static_cast<float>( -bottomCoord.z() ); // Negate Z for Eclipse convention
        }
    }

    // Generate ZCORN array following Eclipse specification
    size_t zcornIdx = 0;

    for ( size_t k = 0; k < nz; ++k )
    {
        // Top layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Face 1: corners (0,3) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                auto topCorners        = gridAdapter.getFaceCorners( i, j, k, cvf::StructGridInterface::NEG_K );
                zcornArray[zcornIdx++] = static_cast<float>( -topCorners[0].z() ); // (-I,-J,top)
                zcornArray[zcornIdx++] = static_cast<float>( -topCorners[3].z() ); // (-I,+J,top)
            }

            // Face 2: corners (1,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                auto topCorners        = gridAdapter.getFaceCorners( i, j, k, cvf::StructGridInterface::NEG_K );
                zcornArray[zcornIdx++] = static_cast<float>( -topCorners[1].z() ); // (+I,-J,top)
                zcornArray[zcornIdx++] = static_cast<float>( -topCorners[2].z() ); // (+I,+J,top)
            }
        }

        // Bottom layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Face 1: corners (0,1) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                auto bottomCorners     = gridAdapter.getFaceCorners( i, j, k, cvf::StructGridInterface::POS_K );
                zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[0].z() ); // (-I,-J,bottom)
                zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[1].z() ); // (+I,-J,bottom)
            }

            // Face 2: corners (3,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                auto bottomCorners     = gridAdapter.getFaceCorners( i, j, k, cvf::StructGridInterface::POS_K );
                zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[3].z() ); // (-I,+J,bottom)
                zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[2].z() ); // (+I,+J,bottom)
            }
        }
    }

    // Generate ACTNUM array using the adapter
    for ( size_t k = 0; k < nz; ++k )
    {
        for ( size_t j = 0; j < ny; ++j )
        {
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIdx       = k * nx * ny + j * nx + i;
                actnumArray[cellIdx] = gridAdapter.isCellActive( i, j, k ) ? 1 : 0;
            }
        }
    }
}
