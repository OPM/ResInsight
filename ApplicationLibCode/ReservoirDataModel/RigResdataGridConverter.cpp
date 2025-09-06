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
/// Generate refined cell corners using trilinear interpolation within the original cell
/// This ensures refined cells are strictly contained within the original cell bounds
//--------------------------------------------------------------------------------------------------
static std::array<cvf::Vec3d, 8> getRefinedCellCorners( const std::array<cvf::Vec3d, 8>& originalCorners,
                                                        size_t                           refinementI,
                                                        size_t                           refinementJ,
                                                        size_t                           refinementK,
                                                        size_t                           subI,
                                                        size_t                           subJ,
                                                        size_t                           subK )
{
    // Use ResInsight's proven face-based refinement approach
    auto allRefinedCorners = RiaCellDividingTools::createHexCornerCoords( originalCorners, refinementI, refinementJ, refinementK );

    // Calculate the linear index of the specific subcell we want
    // createHexCornerCoords uses for(z) for(y) for(x) loops, so X is fastest changing (X-major)
    // Linear index = z * (nx * ny) + y * nx + x
    size_t subcellIndex     = subK * ( refinementI * refinementJ ) + subJ * refinementI + subI;
    size_t cornerStartIndex = subcellIndex * 8;

    // Extract the 8 corners for this specific subcell
    std::array<cvf::Vec3d, 8> refinedCorners;
    for ( size_t i = 0; i < 8; ++i )
    {
        refinedCorners[i] = allRefinedCorners[cornerStartIndex + i];
    }

    return refinedCorners;
}

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

    cvf::Vec3st maxActual =
        max.isUndefined() ? cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 ) : max;

    size_t ni = ( maxActual.x() - min.x() + 1 ) * refinement.x();
    size_t nj = ( maxActual.y() - min.y() + 1 ) * refinement.y();
    size_t nk = ( maxActual.z() - min.z() + 1 ) * refinement.z();

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    // Get coordinate transformation if needed
    cvf::Mat4d           mapAxisTrans;
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
            mapAxes           = { yPoint.x(), yPoint.y(), origin.x(), origin.y(), xPoint.x(), xPoint.y() };

            mapAxisTrans.setTranslation( mapAxisTrans.translation() - minPoint3d );
        }
    }

    // Build refined cell and node data
    std::vector<RigCell>    refinedCells;
    std::vector<cvf::Vec3d> refinedNodes;

    convertGridToCornerPointArrays( eclipseCase,
                                    activeCellInfo,
                                    cellVisibilityOverrideForActnum,
                                    min,
                                    maxActual,
                                    refinement,
                                    mapAxisTrans,
                                    mainGrid->useMapAxes(),
                                    ni,
                                    nj,
                                    nk,
                                    coordArray,
                                    zcornArray,
                                    actnumArray );

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
void RigResdataGridConverter::convertGridToCornerPointArrays( RigEclipseCaseData*      eclipseCase,
                                                              const RigActiveCellInfo* activeCellInfo,
                                                              const cvf::UByteArray*   cellVisibilityOverrideForActnum,
                                                              const cvf::Vec3st&       min,
                                                              const cvf::Vec3st&       max,
                                                              const cvf::Vec3st&       refinement,
                                                              const cvf::Mat4d&        mapAxisTransform,
                                                              bool                     useMapAxes,
                                                              size_t                   nx,
                                                              size_t                   ny,
                                                              size_t                   nz,
                                                              std::vector<float>&      coordArray,
                                                              std::vector<float>&      zcornArray,
                                                              std::vector<int>&        actnumArray )
{
    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    // Resize arrays to correct size
    size_t coordSize  = ( nx + 1 ) * ( ny + 1 ) * 6;
    size_t zcornSize  = nx * ny * nz * 8;
    size_t actnumSize = nx * ny * nz;

    coordArray.resize( coordSize, 0.0f );
    zcornArray.resize( zcornSize, 0.0f );
    actnumArray.resize( actnumSize, 0 );

    // Helper lambda to get refined cell corners on demand
    auto getRefinedCellCornersOnDemand = [&]( size_t refinedI, size_t refinedJ, size_t refinedK ) -> std::array<cvf::Vec3d, 8>
    {
        // Calculate which original cell this refined cell belongs to
        size_t origI = min.x() + refinedI / refinement.x();
        size_t origJ = min.y() + refinedJ / refinement.y();
        size_t origK = min.z() + refinedK / refinement.z();

        // Calculate subcell indices within the original cell
        size_t subI = refinedI % refinement.x();
        size_t subJ = refinedJ % refinement.y();
        size_t subK = refinedK % refinement.z();

        // Get original cell corners
        size_t                    mainIndex       = mainGrid->cellIndexFromIJK( origI, origJ, origK );
        std::array<cvf::Vec3d, 8> originalCorners = mainGrid->cellCornerVertices( mainIndex );

        // Apply coordinate transformations
        if ( useMapAxes )
        {
            for ( cvf::Vec3d& corner : originalCorners )
            {
                corner.transformPoint( mapAxisTransform );
            }
        }

        // Generate refined cell corners
        if ( refinement.x() > 1 || refinement.y() > 1 || refinement.z() > 1 )
        {
            return getRefinedCellCorners( originalCorners, refinement.x(), refinement.y(), refinement.z(), subI, subJ, subK );
        }
        else
        {
            return originalCorners; // No refinement
        }
    };

    // Generate COORD array - pillars for the refined grid
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
                    int refinedI = static_cast<int>( i ) + di;
                    int refinedJ = static_cast<int>( j ) + dj;

                    if ( refinedI >= 0 && refinedI < static_cast<int>( nx ) && refinedJ >= 0 && refinedJ < static_cast<int>( ny ) )
                    {
                        // Get corners for this refined cell (from top layer k=0)
                        auto corners = getRefinedCellCornersOnDemand( refinedI, refinedJ, 0 );

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

                        topCoord    = corners[cornerIdx];
                        bottomCoord = corners[cornerIdx + 4]; // bottom corner (from top layer - will be corrected below)
                        foundCoords = true;
                    }
                }
            }

            // For pillars, we need to find the true top and bottom coordinates across all layers
            if ( foundCoords && nz > 1 )
            {
                // Get bottom coordinate from the bottom layer
                for ( int di = -1; di <= 0; ++di )
                {
                    for ( int dj = -1; dj <= 0; ++dj )
                    {
                        int refinedI = static_cast<int>( i ) + di;
                        int refinedJ = static_cast<int>( j ) + dj;

                        if ( refinedI >= 0 && refinedI < static_cast<int>( nx ) && refinedJ >= 0 && refinedJ < static_cast<int>( ny ) )
                        {
                            // Get corners for this refined cell from bottom layer
                            auto bottomCorners = getRefinedCellCornersOnDemand( refinedI, refinedJ, nz - 1 );

                            size_t cornerIdx = 0;
                            if ( di == 0 && dj == 0 )
                                cornerIdx = 0;
                            else if ( di == -1 && dj == 0 )
                                cornerIdx = 1;
                            else if ( di == 0 && dj == -1 )
                                cornerIdx = 3;
                            else if ( di == -1 && dj == -1 )
                                cornerIdx = 2;

                            bottomCoord = bottomCorners[cornerIdx + 4]; // bottom face corner
                            break;
                        }
                    }
                    if ( bottomCoord != topCoord ) break; // Found different bottom coordinate
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
    size_t zcornIdx      = 0;
    bool   hasRefinement = ( refinement.x() > 1 || refinement.y() > 1 || refinement.z() > 1 );

    // No mapping needed - trilinear interpolation now matches face corner convention

    for ( size_t k = 0; k < nz; ++k )
    {
        // Top layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Face 1: corners (0,3) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                if ( hasRefinement )
                {
                    // Calculate which original cell this refined cell belongs to
                    size_t origI = min.x() + i / refinement.x();
                    size_t origJ = min.y() + j / refinement.y();
                    size_t origK = min.z() + k / refinement.z();

                    // Calculate subcell indices within the original cell
                    size_t subI = i % refinement.x();
                    size_t subJ = j % refinement.y();
                    size_t subK = k % refinement.z();

                    // Get original cell corners using cellCornerVertices (matches createHexCornerCoords expected order)
                    size_t                    mainIndex   = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    std::array<cvf::Vec3d, 8> cellCorners = mainGrid->cellCornerVertices( mainIndex );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : cellCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    // Generate refined cell corners using ResInsight's refinement algorithm
                    auto refinedCorners =
                        getRefinedCellCorners( cellCorners, refinement.x(), refinement.y(), refinement.z(), subI, subJ, subK );

                    // Eclipse Face 1: corners (0,3) which should be (-I,-J,top) and (-I,+J,top)
                    // For refined subcells: corner 0 is always (-I,-J) and corner 3 is always (-I,+J) of that subcell
                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[0].z() ); // (-I,-J,top)
                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[3].z() ); // (-I,+J,top)
                }
                else
                {
                    // Use original cell face extraction for non-refinement case
                    size_t origI      = min.x() + i;
                    size_t origJ      = min.y() + j;
                    size_t origK      = min.z() + k;
                    size_t mainIndex  = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    auto   cell       = mainGrid->cell( mainIndex );
                    auto   topCorners = cell.faceCorners( cvf::StructGridInterface::NEG_K );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : topCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    zcornArray[zcornIdx++] = static_cast<float>( -topCorners[0].z() ); // (-I,-J)
                    zcornArray[zcornIdx++] = static_cast<float>( -topCorners[3].z() ); // (+I,-J)
                }
            }

            // Face 2: corners (1,2) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                if ( hasRefinement )
                {
                    // Calculate which original cell this refined cell belongs to
                    size_t origI = min.x() + i / refinement.x();
                    size_t origJ = min.y() + j / refinement.y();
                    size_t origK = min.z() + k / refinement.z();

                    // Calculate subcell indices within the original cell
                    size_t subI = i % refinement.x();
                    size_t subJ = j % refinement.y();
                    size_t subK = k % refinement.z();

                    // Get original cell corners using cellCornerVertices (matches createHexCornerCoords expected order)
                    size_t                    mainIndex   = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    std::array<cvf::Vec3d, 8> cellCorners = mainGrid->cellCornerVertices( mainIndex );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : cellCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    // Generate refined cell corners using ResInsight's refinement algorithm
                    auto refinedCorners =
                        getRefinedCellCorners( cellCorners, refinement.x(), refinement.y(), refinement.z(), subI, subJ, subK );

                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[1].z() ); // (+I,-J,top)
                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[2].z() ); // (+I,+J,top)
                }
                else
                {
                    // Use original cell face extraction for non-refinement case
                    size_t origI      = min.x() + i;
                    size_t origJ      = min.y() + j;
                    size_t origK      = min.z() + k;
                    size_t mainIndex  = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    auto   cell       = mainGrid->cell( mainIndex );
                    auto   topCorners = cell.faceCorners( cvf::StructGridInterface::NEG_K );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : topCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    zcornArray[zcornIdx++] = static_cast<float>( -topCorners[1].z() ); // (-I,+J)
                    zcornArray[zcornIdx++] = static_cast<float>( -topCorners[2].z() ); // (+I,+J)
                }
            }
        }

        // Bottom layer interface
        for ( size_t j = 0; j < ny; ++j )
        {
            // Face 1: corners (4,5) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                if ( hasRefinement )
                {
                    // Calculate which original cell this refined cell belongs to
                    size_t origI = min.x() + i / refinement.x();
                    size_t origJ = min.y() + j / refinement.y();
                    size_t origK = min.z() + k / refinement.z();

                    // Calculate subcell indices within the original cell
                    size_t subI = i % refinement.x();
                    size_t subJ = j % refinement.y();
                    size_t subK = k % refinement.z();

                    // Get original cell corners using cellCornerVertices (matches createHexCornerCoords expected order)
                    size_t                    mainIndex   = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    std::array<cvf::Vec3d, 8> cellCorners = mainGrid->cellCornerVertices( mainIndex );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : cellCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    // Generate refined cell corners using ResInsight's refinement algorithm
                    auto refinedCorners =
                        getRefinedCellCorners( cellCorners, refinement.x(), refinement.y(), refinement.z(), subI, subJ, subK );

                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[4].z() ); // (-I,-J,bottom)
                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[5].z() ); // (+I,-J,bottom)
                }
                else
                {
                    // Use original cell face extraction for non-refinement case
                    size_t origI         = min.x() + i;
                    size_t origJ         = min.y() + j;
                    size_t origK         = min.z() + k;
                    size_t mainIndex     = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    auto   cell          = mainGrid->cell( mainIndex );
                    auto   bottomCorners = cell.faceCorners( cvf::StructGridInterface::POS_K );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : bottomCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[0].z() ); // (-I,-J)
                    zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[1].z() ); // (+I,-J)
                }
            }

            // Face 2: corners (6,7) for all cells in row j
            for ( size_t i = 0; i < nx; ++i )
            {
                if ( hasRefinement )
                {
                    // Calculate which original cell this refined cell belongs to
                    size_t origI = min.x() + i / refinement.x();
                    size_t origJ = min.y() + j / refinement.y();
                    size_t origK = min.z() + k / refinement.z();

                    // Calculate subcell indices within the original cell
                    size_t subI = i % refinement.x();
                    size_t subJ = j % refinement.y();
                    size_t subK = k % refinement.z();

                    // Get original cell corners using cellCornerVertices (matches createHexCornerCoords expected order)
                    size_t                    mainIndex   = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    std::array<cvf::Vec3d, 8> cellCorners = mainGrid->cellCornerVertices( mainIndex );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : cellCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    // Generate refined cell corners using ResInsight's refinement algorithm
                    auto refinedCorners =
                        getRefinedCellCorners( cellCorners, refinement.x(), refinement.y(), refinement.z(), subI, subJ, subK );

                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[7].z() ); // (-I,+J,bottom)
                    zcornArray[zcornIdx++] = static_cast<float>( -refinedCorners[6].z() ); // (+I,+J,bottom)
                }
                else
                {
                    // Use original cell face extraction for non-refinement case
                    size_t origI         = min.x() + i;
                    size_t origJ         = min.y() + j;
                    size_t origK         = min.z() + k;
                    size_t mainIndex     = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                    auto   cell          = mainGrid->cell( mainIndex );
                    auto   bottomCorners = cell.faceCorners( cvf::StructGridInterface::POS_K );

                    // Apply coordinate transformations if needed
                    if ( useMapAxes )
                    {
                        for ( cvf::Vec3d& corner : bottomCorners )
                        {
                            corner.transformPoint( mapAxisTransform );
                        }
                    }

                    zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[3].z() ); // (-I,+J)
                    zcornArray[zcornIdx++] = static_cast<float>( -bottomCorners[2].z() ); // (+I,+J)
                }
            }
        }
    }

    // Generate ACTNUM array
    for ( size_t k = 0; k < nz; ++k )
    {
        for ( size_t j = 0; j < ny; ++j )
        {
            for ( size_t i = 0; i < nx; ++i )
            {
                size_t cellIdx = k * nx * ny + j * nx + i;

                // Calculate which original cell this refined cell belongs to
                size_t origI = min.x() + i / refinement.x();
                size_t origJ = min.y() + j / refinement.y();
                size_t origK = min.z() + k / refinement.z();

                size_t mainIndex = mainGrid->cellIndexFromIJK( origI, origJ, origK );
                int    active    = activeCellInfo->isActive( mainIndex ) ? 1 : 0;

                if ( active && cellVisibilityOverrideForActnum )
                {
                    active = ( *cellVisibilityOverrideForActnum )[mainIndex];
                }

                actnumArray[cellIdx] = active;
            }
        }
    }
}
