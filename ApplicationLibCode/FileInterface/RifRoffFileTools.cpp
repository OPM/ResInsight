/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RifRoffFileTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RicFaciesPropertiesImportTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimColorLegendCollection.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cafProgressInfo.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "Reader.hpp"

#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace std::chrono;

//--------------------------------------------------------------------------------------------------
/// NOTE: Moved from header file due to compile header when using RifRoffFileTools from
/// ApplicationLibCode/Commands
//--------------------------------------------------------------------------------------------------
template <typename IN, typename OUT>
static void convertToReservoirIndexOrder( int nx, int ny, int nz, const std::vector<IN>& in, std::vector<OUT>& out )
{
    CAF_ASSERT( static_cast<size_t>( nx * ny * nz ) == in.size() );

    out.resize( in.size(), -1 );

    int outIdx = 0;
    for ( int k = 0; k < nz; k++ )
    {
        for ( int j = 0; j < ny; j++ )
        {
            for ( int i = 0; i < nx; i++ )
            {
                int inIdx   = i * ny * nz + j * nz + ( nz - k - 1 );
                out[outIdx] = static_cast<OUT>( in[inIdx] );
                outIdx++;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifRoffFileTools::RifRoffFileTools()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifRoffFileTools::~RifRoffFileTools()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifRoffFileTools::openGridFile( const QString& fileName, RigEclipseCaseData* eclipseCase, QString* errorMessages )
{
    RiaLogging::info( QString( "Opening roff file: %1" ).arg( fileName ) );

    std::string filename = fileName.toStdString();

    std::ifstream stream( filename, std::ios::binary );
    if ( !stream.good() )
    {
        if ( errorMessages ) *errorMessages = QString( "Unable to open roff file" );
        return false;
    }

    auto getInt = []( auto values, const std::string& name )
    {
        auto v = std::find_if( values.begin(), values.end(), [&name]( const auto& arg ) { return arg.first == name; } );
        if ( v != values.end() )
            return std::get<int>( v->second );
        else
            throw std::runtime_error( "Missing parameter (integer): " + name );
    };

    auto getFloat = []( auto values, const std::string& name )
    {
        auto v = std::find_if( values.begin(), values.end(), [&name]( const auto& arg ) { return arg.first == name; } );
        if ( v != values.end() )
            return std::get<float>( v->second );
        else
            throw std::runtime_error( "Missing parameter (float): " + name );
    };

    try
    {
        const auto totalStart = high_resolution_clock::now();

        roff::Reader reader( stream );
        reader.parse();

        const auto tokenizeDone = high_resolution_clock::now();

        std::vector<std::pair<std::string, roff::RoffScalar>>  values     = reader.scalarNamedValues();
        std::vector<std::pair<std::string, roff::Token::Kind>> arrayTypes = reader.getNamedArrayTypes();

        int nx = getInt( values, "dimensions.nX" );
        int ny = getInt( values, "dimensions.nY" );
        int nz = getInt( values, "dimensions.nZ" );

        float xOffset = getFloat( values, "translate.xoffset" );
        float yOffset = getFloat( values, "translate.yoffset" );
        float zOffset = getFloat( values, "translate.zoffset" );

        float xScale = getFloat( values, "scale.xscale" );
        float yScale = getFloat( values, "scale.yscale" );
        float zScale = getFloat( values, "scale.zscale" );

        if ( RiaApplication::enableDevelopmentFeatures() )
        {
            RiaLogging::info( QString( "Grid dimensions: %1 %2 %3" ).arg( nx ).arg( ny ).arg( nz ) );
            RiaLogging::info( QString( "Offset: %1 %2 %3" ).arg( xOffset ).arg( yOffset ).arg( zOffset ) );
            RiaLogging::info( QString( "Scale: %1 %2 %3" ).arg( xScale ).arg( yScale ).arg( zScale ) );
        }

        std::vector<float> cornerLines = reader.getFloatArray( "cornerLines" + roff::Parser::postFixData() );
        std::vector<float> zValues     = reader.getFloatArray( "zvalues" + roff::Parser::postFixData() );
        std::vector<char>  splitEnz    = reader.getByteArray( "zvalues.splitEnz" );
        std::vector<char>  active      = reader.getByteArray( "active" + roff::Parser::postFixData() );

        const auto parsingDone = high_resolution_clock::now();

        unsigned int       zCornerSize = static_cast<unsigned int>( ( nx + 1 ) * ( ny + 1 ) * ( nz + 1 ) * 4u );
        std::vector<float> zCorners( zCornerSize, 0.0 );

        interpretSplitenzData( static_cast<int>( nz ) + 1, zOffset, zScale, splitEnz, zValues, zCorners );

        RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
        CVF_ASSERT( activeCellInfo );

        RigActiveCellInfo* fractureActiveCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );
        CVF_ASSERT( fractureActiveCellInfo );

        RigMainGrid* mainGrid = eclipseCase->mainGrid();
        CVF_ASSERT( mainGrid );

        mainGrid->setCellCounts( cvf::Vec3st( nx, ny, nz ) );
        mainGrid->setGridName( "Main grid" );

        size_t totalCellCount = nx * ny * nz;

        activeCellInfo->setGridCount( 1 );
        fractureActiveCellInfo->setGridCount( 1 );

        activeCellInfo->setReservoirCellCount( totalCellCount );
        fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

        // Reserve room for the cells and nodes and fill them with data
        mainGrid->reservoirCells().reserve( totalCellCount );
        mainGrid->nodes().reserve( 8 * totalCellCount );

        int               progTicks = 100;
        caf::ProgressInfo progInfo( progTicks, "" );

        int    cellCount      = static_cast<int>( totalCellCount );
        size_t cellStartIndex = mainGrid->reservoirCells().size();
        size_t nodeStartIndex = mainGrid->nodes().size();

        RigCell defaultCell;
        defaultCell.setHostGrid( mainGrid );
        mainGrid->reservoirCells().resize( cellStartIndex + cellCount, defaultCell );

        mainGrid->nodes().resize( nodeStartIndex + static_cast<size_t>( cellCount ) * 8, cvf::Vec3d( 0, 0, 0 ) );

        // Swap i and j to get correct faces
        const size_t cellMappingECLRi[8] = { 2, 3, 1, 0, 6, 7, 5, 4 };

        cvf::Vec3d offset( xOffset, yOffset, zOffset );
        cvf::Vec3d scale( xScale, yScale, zScale );

        std::vector<int> activeCells;
        convertToReservoirIndexOrder<char, int>( nx, ny, nz, active, activeCells );

        // Precompute the active cell matrix index
        size_t numActiveCells = computeActiveCellMatrixIndex( activeCells );

        // Loop over cells and fill them with data
#pragma omp for
        for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex )
        {
            RigCell& cell = mainGrid->cell( cellStartIndex + gridLocalCellIndex );

            cell.setGridLocalCellIndex( gridLocalCellIndex );

            // Active cell index
            int matrixActiveIndex = activeCells[gridLocalCellIndex];
            if ( matrixActiveIndex != -1 )
            {
                activeCellInfo->setCellResultIndex( cellStartIndex + gridLocalCellIndex, matrixActiveIndex );
            }

            cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );

            // Corner coordinates
            for ( int cIdx = 0; cIdx < 8; ++cIdx )
            {
                double* point  = mainGrid->nodes()[nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cellMappingECLRi[cIdx]].ptr();
                auto    corner = getCorner( *mainGrid, cornerLines, zCorners, gridLocalCellIndex, cIdx, offset, scale );

                point[0] = corner.x();
                point[1] = corner.y();
                point[2] = corner.z();

                cell.cornerIndices()[cIdx] = nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cIdx;
            }

            // Mark inactive long pyramid looking cells as invalid
            cell.setInvalid( cell.isLongPyramidCell() );
        }

        activeCellInfo->setGridActiveCellCounts( 0, numActiveCells );
        fractureActiveCellInfo->setGridActiveCellCounts( 0, 0 );

        mainGrid->initAllSubGridsParentGridPointer();
        activeCellInfo->computeDerivedData();
        fractureActiveCellInfo->computeDerivedData();

        if ( RiaApplication::enableDevelopmentFeatures() )
        {
            auto gridConstructionDone = high_resolution_clock::now();

            auto tokenizeDuration = duration_cast<milliseconds>( tokenizeDone - totalStart );
            RiaLogging::info( QString( "Tokenizing: %1 ms" ).arg( tokenizeDuration.count() ) );

            auto parsingDuration = duration_cast<milliseconds>( parsingDone - tokenizeDone );
            RiaLogging::info( QString( "Parsing: %1 ms" ).arg( parsingDuration.count() ) );

            auto gridConstructionDuration = duration_cast<milliseconds>( gridConstructionDone - parsingDone );
            RiaLogging::info( QString( "Grid Construction: %1 ms" ).arg( gridConstructionDuration.count() ) );

            auto totalDuration = duration_cast<milliseconds>( gridConstructionDone - totalStart );
            RiaLogging::info( QString( "Total: %1 ms" ).arg( totalDuration.count() ) );
        }
    }
    catch ( std::runtime_error& err )
    {
        RiaLogging::error( QString( "Roff file import failed: %1" ).arg( err.what() ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
// The indexing conventions for vertices in Roff file
//
//      1-------------0
//     /|            /|
//    / |           / |               /j
//   /  |          /  |              /
//  3-------------2   |             *---i
//  |   |         |   |             |
//  |   5---------|---4             |
//  |  /          |  /              |k
//  | /           | /
//  |/            |/
//  7-------------6
//  vertex indices
//

//
// The indexing conventions for vertices in ResInsight
//
//      7-------------6             |k
//     /|            /|             | /j
//    / |           / |             |/
//   /  |          /  |             *---i
//  4-------------5   |
//  |   |         |   |
//  |   3---------|---2
//  |  /          |  /
//  | /           | /
//  |/            |/
//  0-------------1
//  vertex indices
//
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RifRoffFileTools::getCorner( const RigMainGrid&        grid,
                                        const std::vector<float>& cornerLines,
                                        const std::vector<float>& zcorn,
                                        size_t                    cellIdx,
                                        int                       cornerIdx,
                                        const cvf::Vec3d&         offset,
                                        const cvf::Vec3d&         scale )
{
    size_t iOffset = 0;
    if ( cornerIdx != 1 && cornerIdx != 3 && cornerIdx != 5 && cornerIdx != 7 )
    {
        iOffset = 1;
    }

    size_t jOffset = 0;
    if ( cornerIdx != 2 && cornerIdx != 3 && cornerIdx != 6 && cornerIdx != 7 )
    {
        jOffset = 1;
    }

    size_t i;
    size_t j;
    size_t k;
    grid.ijkFromCellIndex( cellIdx, &i, &j, &k );

    size_t ny = grid.cellCountJ();
    size_t nz = grid.cellCountK();

    size_t cOffset = ( ( i + iOffset ) * ( ny + 1 ) + ( j + jOffset ) ) * 6;

    cvf::Vec3d top( cornerLines[cOffset], cornerLines[cOffset + 1], cornerLines[cOffset + 2] );
    cvf::Vec3d bottom( cornerLines[cOffset + 3], cornerLines[cOffset + 4], cornerLines[cOffset + 5] );

    int adjustedCornerIdx = cornerIdx;
    if ( cornerIdx == 4 ) adjustedCornerIdx = 0;
    if ( cornerIdx == 5 ) adjustedCornerIdx = 1;
    if ( cornerIdx == 6 ) adjustedCornerIdx = 2;
    if ( cornerIdx == 7 ) adjustedCornerIdx = 3;

    // Find the correct offset k direction
    size_t kOffset = 0;
    if ( cornerIdx > 3 )
    {
        kOffset = 1;
    }

    size_t zOffset = ( ( i + iOffset ) * ( ny + 1 ) * ( nz + 1 ) + ( j + jOffset ) * ( nz + 1 ) + ( k + kOffset ) ) * 4 + adjustedCornerIdx;

    double z = -zcorn[zOffset];
    double x = interpolate( top, bottom, z, 0 ) + offset.x();
    double y = interpolate( top, bottom, z, 1 ) + offset.y();

    cvf::Vec3d result = cvf::Vec3d( x, y, z );
    return result;
}

//--------------------------------------------------------------------------------------------------
/// Adapted from xtgeo
//--------------------------------------------------------------------------------------------------
double RifRoffFileTools::interpolate( const cvf::Vec3d& top, const cvf::Vec3d& bottom, double z, int idx )
{
    if ( fabs( bottom[idx] - top[idx] ) > 0.01 )
    {
        return top[idx] - ( z - top.z() ) * ( top[idx] - bottom[idx] ) / ( bottom.z() - top.z() );
    }
    else
    {
        return top[idx];
    }
}

//--------------------------------------------------------------------------------------------------
/// Adapted from xtgeo: https://github.com/equinor/xtgeo/blob/master/src/clib/xtg/grd3d_roff2xtgeo_splitenz.c
//--------------------------------------------------------------------------------------------------
void RifRoffFileTools::interpretSplitenzData( int                       nz,
                                              float                     zoffset,
                                              float                     zscale,
                                              const std::vector<char>&  splitenz,
                                              const std::vector<float>& zdata,
                                              std::vector<float>&       zcornsv )

{
    // We Read one corner line (pillar) from zdata at a time (one for each
    // i,j), transform it according to zoffset and zscale, and put it in to
    // zcornsv in reverse order.

    // As i and j order and size is the same for both zcornsv and zdata, we can
    // ignore it here and place one pillar at the time regardless of how many
    // pillars there are.

    size_t nzcorn    = zcornsv.size();
    size_t nsplitenz = splitenz.size();
    size_t nzdata    = zdata.size();

    size_t num_row = 4 * static_cast<size_t>( nz );
    if ( nzcorn % num_row != 0 ) throw std::runtime_error( "Incorrect size of zcorn." );
    if ( nsplitenz != nzcorn / 4 ) throw std::runtime_error( "Incorrect size of splitenz." );

    std::vector<float> pillar( num_row, 0.0 );
    size_t             it_zdata = 0, it_splitenz = 0, it_zcorn = 0;
    while ( it_zcorn < nzcorn )
    {
        for ( size_t it_pillar = 0; it_pillar < num_row; )
        {
            char split = splitenz[it_splitenz++];
            if ( split == 1 )
            {
                // There is one value for this corner which
                // we must duplicate 4 times in zcornsv
                if ( it_zdata >= nzdata ) throw std::runtime_error( "Incorrect size of zdata" );
                float val = ( zdata[it_zdata++] + zoffset ) * zscale;
                for ( int n = 0; n < 4; n++ )
                {
                    pillar[it_pillar++] = val;
                }
            }
            else if ( split == 4 )
            {
                // There are four value for this corner which
                // we must duplicate 4 times in zcornsv
                if ( it_zdata + 3 >= nzdata ) throw std::runtime_error( "Incorrect size of zdata" );
                // As we place the pillar in reverse order into zcornsv,
                // we must put zdata in reverse order into pillar to
                // preserve n,s,w,e directions.
                pillar[it_pillar + 3] = ( zdata[it_zdata++] + zoffset ) * zscale;
                pillar[it_pillar + 2] = ( zdata[it_zdata++] + zoffset ) * zscale;
                pillar[it_pillar + 1] = ( zdata[it_zdata++] + zoffset ) * zscale;
                pillar[it_pillar]     = ( zdata[it_zdata++] + zoffset ) * zscale;
                it_pillar += 4;
            }
            else
            {
                throw std::runtime_error( "Unsupported split type" );
            }
        }
        // Put the pillar into zcornsv in reverse order
        for ( size_t it_pillar = num_row; it_pillar >= 1; )
        {
            zcornsv[it_zcorn++] = pillar[--it_pillar];
        }
    }

    if ( it_splitenz != nsplitenz ) throw std::runtime_error( "Incorrect size of splitenz." );
    if ( it_zdata != nzdata ) throw std::runtime_error( "Incorrect size of zdata" );
    if ( it_zcorn != nzcorn ) throw std::runtime_error( "Incorrect size of zcorn." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifRoffFileTools::computeActiveCellMatrixIndex( std::vector<int>& activeCells )
{
    int activeMatrixIndex = 0;
    int cellCount         = static_cast<int>( activeCells.size() );
    for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; gridLocalCellIndex++ )
    {
        if ( activeCells[gridLocalCellIndex] != 0 )
        {
            activeCells[gridLocalCellIndex] = activeMatrixIndex;
            activeMatrixIndex++;
        }
        else
        {
            activeCells[gridLocalCellIndex] = -1;
        }
    }

    return activeMatrixIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::map<QString, QString>> RifRoffFileTools::createInputProperties( const QString&      fileName,
                                                                                     RigEclipseCaseData* eclipseCaseData )
{
    RiaLogging::info( QString( "Reading properties from roff file: %1" ).arg( fileName ) );

    std::string filename = fileName.toStdString();

    std::map<QString, QString> keywordMapping;

    std::ifstream stream( filename, std::ios::binary );
    if ( !stream.good() )
    {
        RiaLogging::error( "Unable to open roff file" );
        return std::make_pair( false, keywordMapping );
    }

    auto codeNamesAndValuesForKeyword = []( const std::string& keyword, roff::Reader& reader ) -> std::map<int, QString>
    {
        const std::string codeNamesKeyword  = keyword + roff::Parser::postFixCodeNames();
        const std::string codeValuesKeyword = keyword + roff::Parser::postFixCodeValues();

        if ( reader.getArrayLength( codeNamesKeyword ) > 0 &&
             reader.getArrayLength( codeNamesKeyword ) == reader.getArrayLength( codeValuesKeyword ) )
        {
            const auto fileCodeNames  = reader.getStringArray( codeNamesKeyword );
            const auto fileCodeValues = reader.getIntArray( codeValuesKeyword );

            std::map<int, QString> codeNamesAndValues;
            for ( size_t i = 0; i < fileCodeNames.size(); i++ )
            {
                codeNamesAndValues[fileCodeValues[i]] = QString::fromStdString( fileCodeNames[i] ).trimmed();
            }

            return codeNamesAndValues;
        };

        return {};
    };

    try
    {
        roff::Reader reader( stream );
        reader.parse();

        std::vector<std::pair<std::string, roff::Token::Kind>> arrayTypes = reader.getNamedArrayTypes();

        for ( auto [keyword, kind] : arrayTypes )
        {
            size_t keywordLength = reader.getArrayLength( keyword );
            if ( RiaApplication::enableDevelopmentFeatures() )
            {
                RiaLogging::info( QString( "Array found: '%1'. Type: %2 with size: %3." )
                                      .arg( QString::fromStdString( keyword ) )
                                      .arg( QString::fromStdString( roff::Token::kindToString( kind ) ) )
                                      .arg( keywordLength ) );
            }

            QString keywordUpperCase = QString::fromStdString( keyword ).toUpper();

            if ( eclipseCaseData->mainGrid()->cellCount() == keywordLength )
            {
                QString newResultName = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                            ->makeResultNameUnique( QString::fromStdString( keyword ) );
                // Special handling for active.data ==> ACTNUM
                if ( newResultName == QString( "active" ) + QString::fromStdString( roff::Parser::postFixData() ) )
                {
                    newResultName = "ACTNUM";
                }

                if ( !appendNewInputPropertyResult( eclipseCaseData, newResultName, keyword, kind, reader ) )
                {
                    RiaLogging::error(
                        QString( "Unable to import result '%1' from %2" ).arg( QString::fromStdString( keyword ) ).arg( fileName ) );
                    return std::make_pair( false, keywordMapping );
                }

                const auto codeNames            = codeNamesAndValuesForKeyword( keyword, reader );
                bool       anyValidCategoryName = false;
                for ( const auto& codeName : codeNames )
                {
                    if ( !codeName.second.isEmpty() )
                    {
                        anyValidCategoryName = true;
                    }
                }

                if ( anyValidCategoryName )
                {
                    RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;

                    int  caseId  = 0;
                    auto rimCase = eclipseCaseData->ownerCase();
                    if ( rimCase ) caseId = rimCase->caseId();

                    // Delete existing color legend, as new legend will be populated by values from file
                    colorLegendCollection->deleteColorLegend( caseId, newResultName );

                    RimColorLegend* colorLegend = nullptr;
                    if ( keywordUpperCase == RiaResultNames::facies() )
                    {
                        colorLegend = RicFaciesPropertiesImportTools::createColorLegendMatchDefaultRockColors( codeNames );
                    }
                    else
                    {
                        colorLegend = colorLegendCollection->createColorLegend( newResultName, codeNames );
                    }

                    colorLegendCollection->setDefaultColorLegendForResult( caseId, newResultName, colorLegend );
                    colorLegendCollection->updateAllRequiredEditors();
                }

                keywordMapping[QString::fromStdString( keyword )] = newResultName;
            }
            else if ( keywordUpperCase == RiaResultNames::facies() )
            {
                // We have the definition of facies name and value, but we do not have values for cells. Create color legend and get values
                // from other sources, i.e. Eclipse result files

                const auto codeNames = codeNamesAndValuesForKeyword( keyword, reader );
                RicFaciesPropertiesImportTools::createColorLegendMatchDefaultRockColors( codeNames );
            }
        }
    }
    catch ( std::runtime_error& err )
    {
        RiaLogging::error( QString( "Roff property file import failed: %1" ).arg( err.what() ) );
        return std::make_pair( false, keywordMapping );
    }

    return std::make_pair( true, keywordMapping );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifRoffFileTools::hasGridData( const QString& filename )
{
    // Check if arrayTypes contains grid info
    // - Alt 1. Look for tag "cornerLines" and its data with keyword "cornerLines.data" - actual representation of grid data
    // - Alt 2. Look for tag "filedata" and its filetype with keyword "filedata.filetype" - should be equal "grid" (not as robust, rather
    // look for "cornerLines" which is grid representation)

    std::ifstream stream( filename.toStdString(), std::ios::binary );
    if ( !stream.good() )
    {
        RiaLogging::error( "Unable to open roff file" );
        return false;
    }

    roff::Reader reader( stream );
    reader.parse();

    const std::vector<std::pair<std::string, roff::Token::Kind>> arrayTypes = reader.getNamedArrayTypes();

    const std::string cornerLinesDataKeyword = "cornerLines" + roff::Parser::postFixData();
    auto              cornerLinesDataItr     = std::find_if( arrayTypes.begin(),
                                            arrayTypes.end(),
                                            [&cornerLinesDataKeyword]( const auto& arrayType )
                                            { return arrayType.first == cornerLinesDataKeyword; } );

    return cornerLinesDataItr != arrayTypes.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RifRoffFileTools::readAndConvertToDouble( int nx, int ny, int nz, const std::string& keyword, roff::Token::Kind kind, roff::Reader& reader )
{
    std::vector<double> doubleVals;

    if ( kind == roff::Token::Kind::FLOAT )
    {
        std::vector<float> values = reader.getFloatArray( keyword );
        convertToReservoirIndexOrder<float, double>( nx, ny, nz, values, doubleVals );
    }
    else if ( kind == roff::Token::Kind::BOOL || kind == roff::Token::Kind::BYTE )
    {
        std::vector<char> values = reader.getByteArray( keyword );
        convertToReservoirIndexOrder<char, double>( nx, ny, nz, values, doubleVals );
    }
    else if ( kind == roff::Token::Kind::INT )
    {
        std::vector<int> values = reader.getIntArray( keyword );
        convertToReservoirIndexOrder<int, double>( nx, ny, nz, values, doubleVals );
    }
    else
    {
        RiaLogging::error( QString( "Unsupported property type '%1' for keyword '%2'." )
                               .arg( QString::fromStdString( roff::Token::kindToString( kind ) ) )
                               .arg( QString::fromStdString( keyword ) ) );
    }

    return doubleVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifRoffFileTools::appendNewInputPropertyResult( RigEclipseCaseData* caseData,
                                                     const QString&      resultName,
                                                     const std::string&  keyword,
                                                     roff::Token::Kind   kind,
                                                     roff::Reader&       reader )
{
    CVF_ASSERT( caseData );

    int                 nx     = static_cast<int>( caseData->mainGrid()->cellCountI() );
    int                 ny     = static_cast<int>( caseData->mainGrid()->cellCountJ() );
    int                 nz     = static_cast<int>( caseData->mainGrid()->cellCountK() );
    std::vector<double> values = readAndConvertToDouble( nx, ny, nz, keyword, kind, reader );

    auto mainGrid = caseData->mainGrid();
    if ( values.size() != mainGrid->cellCount() ) return false;

    // Set better invalid value for inactive cells: roff file has -999
    auto   activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    size_t cellCount      = mainGrid->cellCount();
    for ( size_t i = 0; i < cellCount; i++ )
    {
        if ( !activeCellInfo->isActive( mainGrid->reservoirCellIndex( i ) ) )
        {
            values[i] = HUGE_VAL;
        }
    }

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::INPUT_PROPERTY, RifRoffFileTools::mapFromType( kind ), resultName );
    caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createResultEntry( resAddr, false );

    auto newPropertyData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->modifiableCellScalarResultTimesteps( resAddr );

    newPropertyData->push_back( values );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ResultDataType RifRoffFileTools::mapFromType( roff::Token::Kind kind )
{
    switch ( kind )
    {
        case roff::Token::Kind::BOOL:
            return RiaDefines::ResultDataType::INTEGER;
        case roff::Token::Kind::BYTE:
            return RiaDefines::ResultDataType::INTEGER;
        case roff::Token::Kind::INT:
            return RiaDefines::ResultDataType::INTEGER;
    }

    return RiaDefines::ResultDataType::FLOAT;
}
