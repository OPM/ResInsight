/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RiaCellDividingTools.h"
#include "RiaDefines.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
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
    if ( !eclipseCase )
    {
        return false;
    }

    const RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    CVF_ASSERT( activeCellInfo );

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    cvf::Vec3st maxActual = max;
    if ( maxActual == cvf::Vec3st::UNDEFINED )
    {
        maxActual = cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );
    }

    size_t ni = ( maxActual.x() - min.x() + 1 ) * refinement.x();
    size_t nj = ( maxActual.y() - min.y() + 1 ) * refinement.y();
    size_t nk = ( maxActual.z() - min.z() + 1 ) * refinement.z();

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    // Get coordinate transformation if needed
    cvf::Mat4d mapAxisTrans;
    std::array<float, 6> mapAxes = mainGrid->mapAxesF();
    
    if ( mainGrid->useMapAxes() )
    {
        mapAxisTrans = mainGrid->mapAxisTransform();
        
        if ( exportInLocalCoordinates )
        {
            cvf::Vec3d minPoint3d( mainGrid->boundingBox().min() );
            cvf::Vec2f minPoint2f( minPoint3d.x(), minPoint3d.y() );
            cvf::Vec2f origin( mapAxes[2] - minPoint2f.x(), mapAxes[3] - minPoint2f.y() );
            cvf::Vec2f xPoint = cvf::Vec2f( mapAxes[4], mapAxes[5] ) - minPoint2f;
            cvf::Vec2f yPoint = cvf::Vec2f( mapAxes[0], mapAxes[1] ) - minPoint2f;
            mapAxes = { yPoint.x(), yPoint.y(), origin.x(), origin.y(), xPoint.x(), xPoint.y() };
            
            mapAxisTrans.setTranslation( mapAxisTrans.translation() - minPoint3d );
        }
    }

    // Build refined cell and node data
    std::vector<RigCell>    refinedCells;
    std::vector<cvf::Vec3d> refinedNodes;
    
    convertGridToCornerPointArrays( eclipseCase, activeCellInfo, cellVisibilityOverrideForActnum, 
                                   min, maxActual, refinement, mapAxisTrans, mainGrid->useMapAxes(),
                                   ni, nj, nk, coordArray, zcornArray, actnumArray );

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

        int valuesPerRow = 4;
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

        int valuesPerRow = 4;
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
    if ( mainGrid->useMapAxes() )
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
/// Supports refinement and coordinate transformations including MAPAXIS.
/// See Eclipse_Grid_Format.md for details
///
//--------------------------------------------------------------------------------------------------
void RigResdataGridConverter::convertGridToCornerPointArrays( RigEclipseCaseData*            eclipseCase,
                                                              const RigActiveCellInfo*      activeCellInfo,
                                                              const cvf::UByteArray*        cellVisibilityOverrideForActnum,
                                                              const cvf::Vec3st&             min,
                                                              const cvf::Vec3st&             max,
                                                              const cvf::Vec3st&             refinement,
                                                              const cvf::Mat4d&              mapAxisTransform,
                                                              bool                           useMapAxes,
                                                              size_t                         nx,
                                                              size_t                         ny,
                                                              size_t                         nz,
                                                              std::vector<float>&            coordArray,
                                                              std::vector<float>&            zcornArray,
                                                              std::vector<int>&              actnumArray )
{
    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    // Resize arrays to correct size
    size_t coordSize  = ( nx + 1 ) * ( ny + 1 ) * 6;
    size_t zcornSize  = nx * ny * nz * 8;
    size_t actnumSize = nx * ny * nz;

    coordArray.resize( coordSize, 0.0f );
    zcornArray.resize( zcornSize, 0.0f );
    actnumArray.resize( actnumSize, 0 );

    // Build refined cell data with coordinate transformations
    std::vector<std::vector<cvf::Vec3d>> refinedCellCorners;
    std::vector<int>                     refinedCellActivity;
    
    for ( size_t k = 0; k <= max.z() - min.z(); ++k )
    {
        for ( size_t j = 0; j <= max.y() - min.y(); ++j )
        {
            for ( size_t i = 0; i <= max.x() - min.x(); ++i )
            {
                size_t mainIndex = mainGrid->cellIndexFromIJK( min.x() + i, min.y() + j, min.z() + k );

                int active = activeCellInfo->isActive( mainIndex ) ? 1 : 0;
                if ( active && cellVisibilityOverrideForActnum )
                {
                    active = ( *cellVisibilityOverrideForActnum )[mainIndex];
                }

                std::array<cvf::Vec3d, 8> cellCorners;
                mainGrid->cellCornerVertices( mainIndex, cellCorners.data() );

                // Apply coordinate transformations
                if ( useMapAxes )
                {
                    for ( cvf::Vec3d& corner : cellCorners )
                    {
                        corner.transformPoint( mapAxisTransform );
                    }
                }

                // Handle refinement
                auto refinedCoords = RiaCellDividingTools::createHexCornerCoords( cellCorners, refinement.x(), refinement.y(), refinement.z() );

                // Store refined cell corners and activity for later processing
                size_t cellsPerOriginal = refinement.x() * refinement.y() * refinement.z();
                for ( size_t subCellIdx = 0; subCellIdx < cellsPerOriginal; ++subCellIdx )
                {
                    std::vector<cvf::Vec3d> corners( 8 );
                    for ( size_t cIdx = 0; cIdx < 8; ++cIdx )
                    {
                        corners[cIdx] = refinedCoords[subCellIdx * 8 + cIdx];
                    }
                    refinedCellCorners.push_back( corners );
                    refinedCellActivity.push_back( active );
                }
            }
        }
    }

    // Generate COORD array from refined grid pillars
    for ( size_t j = 0; j <= ny; ++j )
    {
        for ( size_t i = 0; i <= nx; ++i )
        {
            size_t pillarIndex = j * ( nx + 1 ) + i;
            size_t coordIndex  = pillarIndex * 6;

            cvf::Vec3d topCoord( 0.0, 0.0, 0.0 );
            cvf::Vec3d bottomCoord( 0.0, 0.0, 0.0 );
            bool       foundCoords = false;

            // Find pillar coordinates from adjacent refined cells
            for ( int di = -1; di <= 0 && !foundCoords; ++di )
            {
                for ( int dj = -1; dj <= 0 && !foundCoords; ++dj )
                {
                    int cellI = static_cast<int>( i ) + di;
                    int cellJ = static_cast<int>( j ) + dj;

                    if ( cellI >= 0 && cellI < static_cast<int>( nx ) && cellJ >= 0 && cellJ < static_cast<int>( ny ) )
                    {
                        // Get corners from top cell (k=0)
                        size_t cellIndex = static_cast<size_t>( cellJ * nx + cellI );
                        if ( cellIndex < refinedCellCorners.size() )
                        {
                            const auto& corners = refinedCellCorners[cellIndex];

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

                            topCoord = corners[cornerIdx];
                            bottomCoord = corners[cornerIdx + 4]; // bottom corner
                            foundCoords = true;
                        }
                    }
                }
            }

            // Store pillar coordinates in COORD array
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
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < refinedCellCorners.size() )
                {
                    const auto& corners = refinedCellCorners[cellIndex];
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[0].z() ); // SW top
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[3].z() ); // NW top
                }
                else
                {
                    zcornIdx += 2;
                }
            }

            // Face 2: corners (1,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < refinedCellCorners.size() )
                {
                    const auto& corners = refinedCellCorners[cellIndex];
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[1].z() ); // SE top
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[2].z() ); // NE top
                }
                else
                {
                    zcornIdx += 2;
                }
            }
        }

        // Bottom layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Face 1: corners (4,7) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < refinedCellCorners.size() )
                {
                    const auto& corners = refinedCellCorners[cellIndex];
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[4].z() ); // SW bottom
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[7].z() ); // NW bottom
                }
                else
                {
                    zcornIdx += 2;
                }
            }

            // Face 2: corners (5,6) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < refinedCellCorners.size() )
                {
                    const auto& corners = refinedCellCorners[cellIndex];
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[5].z() ); // SE bottom
                    zcornArray[zcornIdx++] = static_cast<float>( -corners[6].z() ); // NE bottom
                }
                else
                {
                    zcornIdx += 2;
                }
            }
        }
    }

    // Generate ACTNUM array 
    for ( size_t cellIdx = 0; cellIdx < actnumSize; ++cellIdx )
    {
        if ( cellIdx < refinedCellActivity.size() )
        {
            actnumArray[cellIdx] = refinedCellActivity[cellIdx];
        }
        else
        {
            actnumArray[cellIdx] = 0; // Inactive if no data
        }
    }
}
