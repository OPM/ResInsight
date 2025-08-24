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

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RiaDefines.h"

#include "cvfArray.h"
#include "cvfStructGrid.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>

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

    std::vector<float> coordArray;
    std::vector<float> zcornArray;

    auto nodes = mainGrid->nodes();

    std::vector<RigCell> cells;

    for ( size_t k = 0; k <= max.z() - min.z(); ++k )
    {
        for ( size_t j = 0; j <= max.y() - min.y(); ++j )
        {
            for ( size_t i = 0; i <= max.x() - min.x(); ++i )
            {
                size_t mainIndex = mainGrid->cellIndexFromIJK( min.x() + i, min.y() + j, min.z() + k );

                cells.push_back( mainGrid->cell( mainIndex ) );
            }
        }
    }

    size_t ni = ( max.x() - min.x() + 1 ) * refinement.x();
    size_t nj = ( max.y() - min.y() + 1 ) * refinement.y();
    size_t nk = ( max.z() - min.z() + 1 ) * refinement.z();

    convertGridToCornerPointArrays( cells, nodes, ni, nj, nk, coordArray, zcornArray );

    QFile exportFile( resultFileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        return false;
    }

    {
        QTextStream out( &exportFile );
        out << "SPECGRID\n";
        out << "  " << ni << "  " << nj << "  " << nk << "  1  F /\n";
    }

    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "COORD";
        std::vector<double> coordArrayDouble;
        // convert from float to double
        for ( const auto& v : coordArray )
        {
            coordArrayDouble.push_back( v );
        }

        int valuesPerRow = 4;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, coordArrayDouble, valuesPerRow );
    }

    {
        bool                writeEchoKeywordsInExporterObject = true;
        QString             keyword                           = "ZCORN";
        std::vector<double> coordArrayDouble;
        // convert from float to double
        for ( const auto& v : zcornArray )
        {
            coordArrayDouble.push_back( v );
        }

        int valuesPerRow = 4;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, coordArrayDouble, valuesPerRow );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Convert grid cells to Eclipse corner point arrays (COORD and ZCORN)
///
/// This function converts a set of grid cells into the Eclipse corner point grid format:
/// - COORD array contains pillar coordinates (6 values per pillar: x1,y1,z1,x2,y2,z2)
/// - ZCORN array contains Z values for cell corners (8 Z values per cell)
///
/// See Eclipse_Grid_Format.md for details
///
//--------------------------------------------------------------------------------------------------
void RigResdataGridConverter::convertGridToCornerPointArrays( const std::vector<RigCell>&    cells,
                                                              const std::vector<cvf::Vec3d>& nodes,
                                                              size_t                         nx,
                                                              size_t                         ny,
                                                              size_t                         nz,
                                                              std::vector<float>&            coordArray,
                                                              std::vector<float>&            zcornArray )
{
    // Resize arrays to correct size
    size_t coordSize = ( nx + 1 ) * ( ny + 1 ) * 6;
    size_t zcornSize = nx * ny * nz * 8;

    coordArray.resize( coordSize, 0.0f );
    zcornArray.resize( zcornSize, 0.0f );

    // Create COORD array - pillar coordinates
    // Each pillar connects (i,j) positions from top to bottom of grid
    for ( size_t j = 0; j <= ny; ++j )
    {
        for ( size_t i = 0; i <= nx; ++i )
        {
            size_t pillarIndex = j * ( nx + 1 ) + i;
            size_t coordIndex  = pillarIndex * 6;

            // Find top and bottom coordinates for this pillar
            // Use corner nodes from adjacent cells to define pillar
            cvf::Vec3d topCoord( 0.0, 0.0, 0.0 );
            cvf::Vec3d bottomCoord( 0.0, 0.0, 0.0 );

            // Sample from available cells to get pillar coordinates
            bool foundCoords = false;

            // Check all adjacent cells for this pillar position
            for ( int di = -1; di <= 0 && !foundCoords; ++di )
            {
                for ( int dj = -1; dj <= 0 && !foundCoords; ++dj )
                {
                    int cellI = static_cast<int>( i ) + di;
                    int cellJ = static_cast<int>( j ) + dj;

                    if ( cellI >= 0 && cellI < static_cast<int>( nx ) && cellJ >= 0 && cellJ < static_cast<int>( ny ) )
                    {
                        // Find a cell at k=0 to get pillar coordinates
                        size_t cellIndex = static_cast<size_t>( cellJ * nx + cellI );

                        if ( cellIndex < cells.size() )
                        {
                            const RigCell& cell = cells[cellIndex];

                            // Determine which corner of this cell corresponds to our pillar
                            size_t cornerIdx = 0;
                            if ( di == 0 && dj == 0 )
                                cornerIdx = 0; // cell's (0,0) corner
                            else if ( di == -1 && dj == 0 )
                                cornerIdx = 1; // cell's (1,0) corner
                            else if ( di == 0 && dj == -1 )
                                cornerIdx = 3; // cell's (0,1) corner
                            else if ( di == -1 && dj == -1 )
                                cornerIdx = 2; // cell's (1,1) corner

                            size_t nodeIndex = cell.cornerIndices()[cornerIdx];
                            if ( nodeIndex < nodes.size() )
                            {
                                topCoord    = nodes[nodeIndex];
                                foundCoords = true;

                                // Find bottom coordinate from deepest cell
                                bottomCoord = topCoord;
                                for ( size_t k = 0; k < nz; ++k )
                                {
                                    size_t deepCellIndex = k * nx * ny + static_cast<size_t>( cellJ * nx + cellI );
                                    if ( deepCellIndex < cells.size() )
                                    {
                                        const RigCell& deepCell      = cells[deepCellIndex];
                                        size_t         deepCornerIdx = cornerIdx + 4; // bottom corners are +4 from top
                                        size_t         deepNodeIndex = deepCell.cornerIndices()[deepCornerIdx];
                                        if ( deepNodeIndex < nodes.size() )
                                        {
                                            bottomCoord = nodes[deepNodeIndex];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Store pillar coordinates in COORD array
            coordArray[coordIndex + 0] = static_cast<float>( topCoord.x() );
            coordArray[coordIndex + 1] = static_cast<float>( topCoord.y() );
            coordArray[coordIndex + 2] = static_cast<float>( -topCoord.z() );
            coordArray[coordIndex + 3] = static_cast<float>( bottomCoord.x() );
            coordArray[coordIndex + 4] = static_cast<float>( bottomCoord.y() );
            coordArray[coordIndex + 5] = static_cast<float>( -bottomCoord.z() );
        }
    }

    // Create ZCORN array following Eclipse specification
    // Following the pseudocode: organize by layer, then by j, then by face (not corner)

    size_t zcornIdx = 0;

    for ( size_t k = 0; k < nz; ++k )
    {
        // Top layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Export z values for face 1 (corners 0,3) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < cells.size() )
                {
                    const RigCell& cell       = cells[cellIndex];
                    auto           topCorners = cell.faceCorners( cvf::StructGridInterface::NEG_K );
                    zcornArray[zcornIdx++]    = static_cast<float>( -topCorners[0].z() ); // (-I,-J)
                    zcornArray[zcornIdx++]    = static_cast<float>( -topCorners[3].z() ); // (+I,-J)
                }
                else
                {
                    zcornIdx += 2;
                }
            }

            // Export z values for face 2 (corners 1,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < cells.size() )
                {
                    const RigCell& cell       = cells[cellIndex];
                    auto           topCorners = cell.faceCorners( cvf::StructGridInterface::NEG_K );
                    zcornArray[zcornIdx++]    = static_cast<float>( -topCorners[1].z() ); // (-I,+J)
                    zcornArray[zcornIdx++]    = static_cast<float>( -topCorners[2].z() ); // (+I,+J)
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
            // Export z values for face 1 (corners 0,1) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < cells.size() )
                {
                    const RigCell& cell          = cells[cellIndex];
                    auto           bottomCorners = cell.faceCorners( cvf::StructGridInterface::POS_K );
                    zcornArray[zcornIdx++]       = static_cast<float>( -bottomCorners[0].z() ); // (-I,-J)
                    zcornArray[zcornIdx++]       = static_cast<float>( -bottomCorners[1].z() ); // (+I,-J)
                }
                else
                {
                    zcornIdx += 2;
                }
            }

            // Export z values for face 2 (corners 3,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIndex = k * nx * ny + j * nx + i;
                if ( cellIndex < cells.size() )
                {
                    const RigCell& cell          = cells[cellIndex];
                    auto           bottomCorners = cell.faceCorners( cvf::StructGridInterface::POS_K );
                    zcornArray[zcornIdx++]       = static_cast<float>( -bottomCorners[3].z() ); // (-I,+J)
                    zcornArray[zcornIdx++]       = static_cast<float>( -bottomCorners[2].z() ); // (+I,+J)
                }
                else
                {
                    zcornIdx += 2;
                }
            }
        }
    }
}
