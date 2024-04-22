/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RifEclipseInputFileTools.h"

#include "RiaCellDividingTools.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"
#include "RiaStringEncodingTools.h"
#include "RiaTextStringTools.h"

#include "ExportCommands/RicEclipseCellResultToFileImpl.h"

#include "RifEclipseInputPropertyLoader.h"
#include "RifEclipseKeywordContent.h"
#include "RifEclipseTextFileReader.h"
#include "RifReaderEclipseOutput.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "cafProgressInfo.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "ert/ecl/ecl_box.hpp"
#include "ert/ecl/ecl_grid.hpp"
#include "ert/ecl/ecl_kw.h"

QString includeKeyword( "INCLUDE" );
QString faultsKeyword( "FAULTS" );
QString editKeyword( "EDIT" );
QString gridKeyword( "GRID" );
QString pathsKeyword( "PATHS" );

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseInputFileTools::RifEclipseInputFileTools()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseInputFileTools::~RifEclipseInputFileTools()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::openGridFile( const QString& fileName, RigEclipseCaseData* eclipseCase, bool readFaultData, QString* errorMessages )
{
    std::string filename = fileName.toStdString();

    auto objects = RifEclipseTextFileReader::readKeywordAndValues( filename );

    ecl_kw_type* specGridKw = nullptr;
    ecl_kw_type* zCornKw    = nullptr;
    ecl_kw_type* coordKw    = nullptr;
    ecl_kw_type* actNumKw   = nullptr;
    ecl_kw_type* mapAxesKw  = nullptr;

    for ( const auto& obj : objects )
    {
        {
            std::string keyword = "SPECGRID";
            if ( obj.keyword.compare( keyword ) == 0 )
            {
                std::vector<int> intValues;
                for ( const auto& val : obj.values )
                {
                    intValues.push_back( val );
                }

                specGridKw = ecl_kw_alloc_new( keyword.data(), (int)obj.values.size(), ECL_INT, intValues.data() );
                continue;
            }
        }

        {
            std::string keyword = "COORD";
            if ( obj.keyword.compare( keyword ) == 0 )
            {
                coordKw = ecl_kw_alloc_new( keyword.data(), (int)obj.values.size(), ECL_FLOAT, obj.values.data() );
                continue;
            }
        }

        {
            std::string keyword = "ZCORN";
            if ( obj.keyword.compare( keyword ) == 0 )
            {
                zCornKw = ecl_kw_alloc_new( keyword.data(), (int)obj.values.size(), ECL_FLOAT, obj.values.data() );
                continue;
            }
        }

        {
            std::string keyword = "ACTNUM";
            if ( obj.keyword.compare( keyword ) == 0 )
            {
                std::vector<int> intValues;
                for ( const auto& val : obj.values )
                {
                    intValues.push_back( val );
                }

                actNumKw = ecl_kw_alloc_new( keyword.data(), (int)obj.values.size(), ECL_INT, intValues.data() );
                continue;
            }
        }

        {
            std::string keyword = "MAPAXES";
            if ( obj.keyword.compare( keyword ) == 0 )
            {
                mapAxesKw = ecl_kw_alloc_new( keyword.data(), (int)obj.values.size(), ECL_FLOAT, obj.values.data() );
                continue;
            }
        }
    }

    if ( specGridKw && zCornKw && coordKw )
    {
        int nx = ecl_kw_iget_int( specGridKw, 0 );
        int ny = ecl_kw_iget_int( specGridKw, 1 );
        int nz = ecl_kw_iget_int( specGridKw, 2 );

        ecl_grid_type* inputGrid = ecl_grid_alloc_GRDECL_kw( nx, ny, nz, zCornKw, coordKw, actNumKw, mapAxesKw );

        RifReaderEclipseOutput::transferGeometry( inputGrid, eclipseCase );

        if ( readFaultData )
        {
            bool isFaultKeywordPresent = false;
            for ( const auto& keywordObj : objects )
            {
                if ( keywordObj.keyword == "FAULTS" ) isFaultKeywordPresent = true;
            }

            if ( isFaultKeywordPresent )
            {
                importFaultsFromFile( eclipseCase, fileName );
            }
        }

        bool useMapAxes = ecl_grid_use_mapaxes( inputGrid );
        eclipseCase->mainGrid()->setUseMapAxes( useMapAxes );

        if ( useMapAxes )
        {
            std::array<double, 6> mapAxesValues;
            ecl_grid_init_mapaxes_data_double( inputGrid, mapAxesValues.data() );
            eclipseCase->mainGrid()->setMapAxes( mapAxesValues );
        }

        ecl_kw_free( specGridKw );
        ecl_kw_free( zCornKw );
        ecl_kw_free( coordKw );
        if ( actNumKw ) ecl_kw_free( actNumKw );
        if ( mapAxesKw ) ecl_kw_free( mapAxesKw );

        ecl_grid_free( inputGrid );

        QString errorMessages;

        // Import additional keywords as input properties
        RifEclipseInputPropertyLoader::createInputPropertiesFromKeywords( eclipseCase, objects, &errorMessages );

        if ( !errorMessages.isEmpty() ) RiaLogging::error( errorMessages );

        return true;
    }

    QString txt = "Missing required keywords :";
    if ( !specGridKw ) txt += " SPECGRID";
    if ( !zCornKw ) txt += " ZCORN";
    if ( !coordKw ) txt += " COORD";

    RiaLogging::error( txt );

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::exportGrid( const QString&         fileName,
                                           RigEclipseCaseData*    eclipseCase,
                                           bool                   exportInLocalCoordinates,
                                           const cvf::UByteArray* cellVisibilityOverrideForActnum,
                                           const cvf::Vec3st&     min,
                                           const cvf::Vec3st&     maxIn,
                                           const cvf::Vec3st&     refinement )
{
    if ( !eclipseCase )
    {
        return false;
    }

    const RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    CVF_ASSERT( activeCellInfo );

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    cvf::Vec3st max = maxIn;
    if ( max == cvf::Vec3st::UNDEFINED )
    {
        max = cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );
    }

    int ecl_nx = static_cast<int>( ( max.x() - min.x() + 1 ) * refinement.x() );
    int ecl_ny = static_cast<int>( ( max.y() - min.y() + 1 ) * refinement.y() );
    int ecl_nz = static_cast<int>( ( max.z() - min.z() + 1 ) * refinement.z() );

    CVF_ASSERT( ecl_nx > 0 && ecl_ny > 0 && ecl_nz > 0 );

    size_t cellsPerOriginal = refinement.x() * refinement.y() * refinement.z();

    caf::ProgressInfo progress( mainGrid->cellCount() * 2, "Save Eclipse Grid" );
    int               cellProgressInterval = 1000;

    std::vector<float*> ecl_corners;
    ecl_corners.reserve( mainGrid->cellCount() * cellsPerOriginal );
    std::vector<int*> ecl_coords;
    ecl_coords.reserve( mainGrid->cellCount() * cellsPerOriginal );

    std::array<float, 6> mapAxes      = mainGrid->mapAxesF();
    cvf::Mat4d           mapAxisTrans = mainGrid->mapAxisTransform();
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

    const size_t* cellMappingECLRi = RifReaderEclipseOutput::eclipseCellIndexMapping();

    int outputCellIndex = 0;

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

                if ( mainGrid->useMapAxes() )
                {
                    for ( cvf::Vec3d& corner : cellCorners )
                    {
                        corner.transformPoint( mapAxisTrans );
                    }
                }

                auto refinedCoords = RiaCellDividingTools::createHexCornerCoords( cellCorners, refinement.x(), refinement.y(), refinement.z() );

                for ( size_t subK = 0; subK < refinement.z(); ++subK )
                {
                    for ( size_t subJ = 0; subJ < refinement.y(); ++subJ )
                    {
                        for ( size_t subI = 0; subI < refinement.x(); ++subI )
                        {
                            int* ecl_cell_coords = new int[5];
                            ecl_cell_coords[0]   = static_cast<int>( i * refinement.x() + subI + 1 );
                            ecl_cell_coords[1]   = static_cast<int>( j * refinement.y() + subJ + 1 );
                            ecl_cell_coords[2]   = static_cast<int>( k * refinement.z() + subK + 1 );
                            ecl_cell_coords[3]   = outputCellIndex++;
                            ecl_cell_coords[4]   = active;
                            ecl_coords.push_back( ecl_cell_coords );

                            size_t subIndex = subI + subJ * refinement.x() + subK * refinement.x() * refinement.y();

                            float* ecl_cell_corners = new float[24];
                            for ( size_t cIdx = 0; cIdx < 8; ++cIdx )
                            {
                                const auto cellCorner                            = refinedCoords[subIndex * 8 + cIdx];
                                ecl_cell_corners[cellMappingECLRi[cIdx] * 3]     = cellCorner[0];
                                ecl_cell_corners[cellMappingECLRi[cIdx] * 3 + 1] = cellCorner[1];
                                ecl_cell_corners[cellMappingECLRi[cIdx] * 3 + 2] = -cellCorner[2];
                            }
                            ecl_corners.push_back( ecl_cell_corners );
                        }
                    }
                }
            }
        }

        if ( outputCellIndex % cellProgressInterval == 0 )
        {
            progress.setProgress( outputCellIndex / cellsPerOriginal );
        }
    }

    // Do not perform the transformation (applyMapaxes == false):
    // The coordinates have been transformed to the map axes coordinate system already.
    // However, send the map axes data in to resdata so that the coordinate system description is saved.
    bool           applyMapaxes = false;
    ecl_grid_type* mainEclGrid =
        ecl_grid_alloc_GRID_data( (int)ecl_coords.size(), ecl_nx, ecl_ny, ecl_nz, 5, &ecl_coords[0], &ecl_corners[0], applyMapaxes, mapAxes.data() );
    progress.setProgress( mainGrid->cellCount() );

    for ( float* floatArray : ecl_corners )
    {
        delete floatArray;
    }

    for ( int* intArray : ecl_coords )
    {
        delete intArray;
    }

    FILE* filePtr = util_fopen( RiaStringEncodingTools::toNativeEncoded( fileName ).data(), "w" );

    if ( !filePtr )
    {
        return false;
    }

    ert_ecl_unit_enum ecl_units = ECL_METRIC_UNITS;
    if ( eclipseCase->unitsType() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        ecl_units = ECL_FIELD_UNITS;
    else if ( eclipseCase->unitsType() == RiaDefines::EclipseUnitSystem::UNITS_LAB )
        ecl_units = ECL_LAB_UNITS;

    ecl_grid_fprintf_grdecl2( mainEclGrid, filePtr, ecl_units );
    ecl_grid_free( mainEclGrid );
    fclose( filePtr );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::exportKeywords( const QString&              resultFileName,
                                               RigEclipseCaseData*         eclipseCase,
                                               const std::vector<QString>& keywords,
                                               bool                        writeEchoKeywords,
                                               const cvf::Vec3st&          min,
                                               const cvf::Vec3st&          maxIn,
                                               const cvf::Vec3st&          refinement )
{
    QFile exportFile( resultFileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        return false;
    }

    if ( writeEchoKeywords )
    {
        QTextStream stream( &exportFile );
        stream << "NOECHO\n";
    }

    RigCaseCellResultsData* cellResultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo*      activeCells     = cellResultsData->activeCellInfo();
    RigMainGrid*            mainGrid        = eclipseCase->mainGrid();

    cvf::Vec3st max = maxIn;
    if ( max == cvf::Vec3st::UNDEFINED )
    {
        max = cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );
    }

    caf::ProgressInfo progress( keywords.size(), "Saving Keywords" );

    auto allResultAddresses = cellResultsData->existingResults();

    auto findResultAddress = [&allResultAddresses]( const QString& keyword ) -> RigEclipseResultAddress
    {
        for ( const auto& adr : allResultAddresses )
        {
            if ( adr.resultName() == keyword )
            {
                return adr;
            }
        }

        return {};
    };

    for ( const QString& keyword : keywords )
    {
        RigEclipseResultAddress resAddr = findResultAddress( keyword );
        if ( !cellResultsData->hasResultEntry( resAddr ) ) continue;

        cellResultsData->ensureKnownResultLoaded( resAddr );
        CVF_ASSERT( !cellResultsData->cellScalarResults( resAddr ).empty() );

        std::vector<double> resultValues = cellResultsData->cellScalarResults( resAddr )[0];
        if ( resultValues.empty() ) continue;

        double defaultExportValue = 0.0;
        if ( RiaResultNames::isCategoryResult( keyword ) )
        {
            defaultExportValue = 1.0;
        }

        RiaDefines::PorosityModelType porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

        // Create result accessor object for main grid at time step zero (static result date is always at first time step
        auto resultAcc = RigResultAccessorFactory::createFromResultAddress( eclipseCase, 0, porosityModel, 0, resAddr );

        std::vector<double> filteredResults;
        filteredResults.reserve( resultValues.size() );

        for ( size_t k = min.z() * refinement.z(); k <= max.z() * refinement.z(); ++k )
        {
            size_t mainK = k / refinement.z();
            for ( size_t j = min.y() * refinement.y(); j <= max.y() * refinement.y(); ++j )
            {
                size_t mainJ = j / refinement.y();
                for ( size_t i = min.x() * refinement.x(); i <= max.x() * refinement.x(); ++i )
                {
                    size_t mainI = i / refinement.x();

                    size_t reservoirCellIndex = mainGrid->cellIndexFromIJK( mainI, mainJ, mainK );

                    size_t resIndex = activeCells->cellResultIndex( reservoirCellIndex );
                    if ( resIndex != cvf::UNDEFINED_SIZE_T )
                    {
                        auto value = resultAcc->cellScalarGlobIdx( reservoirCellIndex );
                        filteredResults.push_back( value );
                    }
                    else
                    {
                        filteredResults.push_back( defaultExportValue );
                    }
                }
            }
        }

        // Multiple keywords can be exported to same file, so write ECHO keywords outside the loop
        bool writeEchoKeywordsInExporterObject = false;
        RicEclipseCellResultToFileImpl::writeDataToTextFile( &exportFile, writeEchoKeywordsInExporterObject, keyword, filteredResults );

        progress.incrementProgress();
    }

    if ( writeEchoKeywords )
    {
        QTextStream stream( &exportFile );
        stream << "ECHO\n";
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::saveFault( QString                                 completeFilename,
                                          const RigMainGrid*                      mainGrid,
                                          const std::vector<RigFault::FaultFace>& faultFaces,
                                          QString                                 faultName,
                                          const cvf::Vec3st&                      min,
                                          const cvf::Vec3st&                      maxIn,
                                          const cvf::Vec3st&                      refinement )
{
    QFile exportFile( completeFilename );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        RiaLogging::error( "Could not open the file : " + completeFilename );
        return;
    }

    QTextStream stream( &exportFile );
    stream << "FAULTS" << '\n';

    stream << "-- Name  I1  I2  J1  J2  K1  K2  Face ( I/J/K )" << '\n';

    saveFault( stream, mainGrid, faultFaces, faultName, min, maxIn, refinement );
    stream << "/" << '\n';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::saveFault( QTextStream&                            stream,
                                          const RigMainGrid*                      mainGrid,
                                          const std::vector<RigFault::FaultFace>& faultFaces,
                                          QString                                 faultName,
                                          const cvf::Vec3st&                      min,
                                          const cvf::Vec3st&                      maxIn,
                                          const cvf::Vec3st&                      refinement )
{
    // 'NAME'     1   1      1    1     1     2      J             /

    if ( faultName.contains( ' ' ) )
    {
        RiaLogging::error( QString( "Fault name '%1' contains spaces" ).arg( faultName ) );
        return;
    }
    else if ( faultName.length() > 8 )
    {
        // Keep going anyway, eclipse files sometimes have longer than
        // the specified 8 characters in the name without Eclipse complaining
        RiaLogging::warning( QString( "Fault name '%1' is longer than 8 characters" ).arg( faultName ) );
    }

    std::vector<RigFault::CellAndFace> faultCellAndFaces;

    cvf::Vec3st max = maxIn;
    if ( max == cvf::Vec3st::UNDEFINED )
    {
        max = cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );
    }

    for ( const RigFault::FaultFace& faultCellAndFace : faultFaces )
    {
        size_t i, j, k;
        bool   ok = mainGrid->ijkFromCellIndex( faultCellAndFace.m_nativeReservoirCellIndex, &i, &j, &k );
        if ( !ok ) continue;

        if ( i < min.x() || i > max.x() || j < min.y() || j > max.y() || k < min.z() || k > max.z() ) continue;

        size_t shifted_i = ( i - min.x() ) * refinement.x();
        size_t shifted_j = ( j - min.y() ) * refinement.y();
        size_t shifted_k = ( k - min.z() ) * refinement.z();

        //  2x2 Refinement of Original Cell 0, 0
        // Y/J  POS_J boundary
        // ^  _______________
        // | |       |       |
        // | |  0,1  |  1,1  |
        // | |_______|_______|   POS_I boundary
        // | |       |       |
        // | |  0,0  |  1,0  |
        // | |_______|_______|
        // ---------------------> X/I
        //       NEG_J boundary
        //
        //  POS_J gets shifted 1 index in J direction, NEG_J stays the same in J but spans two I.
        //  POS_I gets shifted 1 index in I direction, NEG_I stays the same in I but spans two J.

        if ( refinement != cvf::Vec3st( 1, 1, 1 ) )
        {
            auto gridAxis = cvf::StructGridInterface::gridAxisFromFace( faultCellAndFace.m_nativeFace );

            if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_I )
            {
                if ( faultCellAndFace.m_nativeFace == cvf::StructGridInterface::POS_I )
                {
                    shifted_i += refinement.x() - 1;
                }
                for ( size_t refineK = 0; refineK < refinement.z(); ++refineK )
                {
                    for ( size_t refineJ = 0; refineJ < refinement.y(); ++refineJ )
                    {
                        faultCellAndFaces.push_back(
                            std::make_tuple( shifted_i, shifted_j + refineJ, shifted_k + refineK, faultCellAndFace.m_nativeFace ) );
                    }
                }
            }
            else if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_J )
            {
                if ( faultCellAndFace.m_nativeFace == cvf::StructGridInterface::POS_J )
                {
                    shifted_j += refinement.y() - 1;
                }

                for ( size_t refineK = 0; refineK < refinement.z(); ++refineK )
                {
                    for ( size_t refineI = 0; refineI < refinement.x(); ++refineI )
                    {
                        faultCellAndFaces.push_back(
                            std::make_tuple( shifted_i + refineI, shifted_j, shifted_k + refineK, faultCellAndFace.m_nativeFace ) );
                    }
                }
            }
            else if ( gridAxis == cvf::StructGridInterface::GridAxisType::AXIS_K )
            {
                if ( faultCellAndFace.m_nativeFace == cvf::StructGridInterface::POS_K )
                {
                    shifted_k += refinement.z() - 1;
                }

                for ( size_t refineJ = 0; refineJ < refinement.y(); ++refineJ )
                {
                    for ( size_t refineI = 0; refineI < refinement.x(); ++refineI )
                    {
                        faultCellAndFaces.push_back(
                            std::make_tuple( shifted_i + refineI, shifted_j + refineJ, shifted_k, faultCellAndFace.m_nativeFace ) );
                    }
                }
            }
        }
        else
        {
            faultCellAndFaces.push_back( std::make_tuple( shifted_i, shifted_j, shifted_k, faultCellAndFace.m_nativeFace ) );
        }
    }

    // Sort order: i, j, face then k.
    std::sort( faultCellAndFaces.begin(), faultCellAndFaces.end(), RigFault::ordering );

    size_t                             lastI        = std::numeric_limits<size_t>::max();
    size_t                             lastJ        = std::numeric_limits<size_t>::max();
    size_t                             lastK        = std::numeric_limits<size_t>::max();
    size_t                             startK       = std::numeric_limits<size_t>::max();
    cvf::StructGridInterface::FaceType lastFaceType = cvf::StructGridInterface::FaceType::NO_FACE;

    for ( const RigFault::CellAndFace& faultCellAndFace : faultCellAndFaces )
    {
        size_t                             i, j, k;
        cvf::StructGridInterface::FaceType faceType;
        std::tie( i, j, k, faceType ) = faultCellAndFace;

        if ( i != lastI || j != lastJ || lastFaceType != faceType || k != lastK + 1 )
        {
            // No fault should have no face
            if ( lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE )
            {
                writeFaultLine( stream, faultName, lastI, lastJ, startK, lastK, lastFaceType );
            }
            lastI        = i;
            lastJ        = j;
            lastK        = k;
            lastFaceType = faceType;
            startK       = k;
        }
        else
        {
            lastK = k;
        }
    }

    // No fault should have no face
    if ( lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE )
    {
        writeFaultLine( stream, faultName, lastI, lastJ, startK, lastK, lastFaceType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::saveFaults( QTextStream&       stream,
                                           const RigMainGrid* mainGrid,
                                           const cvf::Vec3st& min /*= cvf::Vec3st::ZERO*/,
                                           const cvf::Vec3st& max /*= cvf::Vec3st::UNDEFINED*/,
                                           const cvf::Vec3st& refinement /*= cvf::Vec3st(1, 1, 1)*/ )
{
    stream << "FAULTS" << '\n';

    stream << "-- Name  I1  I2  J1  J2  K1  K2  Face ( I/J/K )" << '\n';

    const cvf::Collection<RigFault>& faults = mainGrid->faults();
    for ( const auto& fault : faults )
    {
        if ( fault->name() != RiaResultNames::undefinedGridFaultName() && fault->name() != RiaResultNames::undefinedGridFaultWithInactiveName() )
        {
            saveFault( stream, mainGrid, fault->faultFaces(), fault->name(), min, max, refinement );
        }
    }
    stream << "/" << '\n';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::importFaultsFromFile( RigEclipseCaseData* eclipseCaseData, const QString& fileName )
{
    cvf::Collection<RigFault> faultCollectionFromFile;
    RifEclipseInputFileTools::parseAndReadFaults( fileName, &faultCollectionFromFile );
    if ( faultCollectionFromFile.empty() )
    {
        return false;
    }

    cvf::Collection<RigFault> faults;
    {
        cvf::Collection<RigFault> faultCollection = eclipseCaseData->mainGrid()->faults();
        for ( size_t i = 0; i < faultCollection.size(); i++ )
        {
            RigFault* f = faultCollection.at( i );
            if ( f->name() == RiaResultNames::undefinedGridFaultName() || f->name() == RiaResultNames::undefinedGridFaultName() )
            {
                // Do not include undefined grid faults, as these are recomputed based on the imported faults from
                // files
                continue;
            }

            faults.push_back( f );
        }
    }

    for ( size_t i = 0; i < faultCollectionFromFile.size(); i++ )
    {
        RigFault* faultFromFile = faultCollectionFromFile.at( i );

        bool existFaultWithSameName = false;
        for ( size_t j = 0; j < faults.size(); j++ )
        {
            RigFault* existingFault = faults.at( j );

            if ( existingFault->name() == faultFromFile->name() )
            {
                existFaultWithSameName = true;
            }
        }

        if ( !existFaultWithSameName )
        {
            faults.push_back( faultFromFile );
        }
    }

    eclipseCaseData->mainGrid()->setFaults( faults );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::parseAndReadPathAliasKeyword( const QString&                            fileName,
                                                             std::vector<std::pair<QString, QString>>* pathAliasDefinitions )
{
    char buf[1024];

    QFile data( fileName );
    data.open( QFile::ReadOnly );

    QString line;

    bool foundPathsKeyword = false;

    do
    {
        qint64 lineLength = data.readLine( buf, sizeof( buf ) );
        if ( lineLength > 0 )
        {
            line = QString::fromLatin1( buf );
            if ( line.size() && ( line[0].isLetter() || foundPathsKeyword ) )
            {
                line = line.trimmed();

                if ( line == gridKeyword )
                {
                    return;
                }
                else if ( line == pathsKeyword )
                {
                    foundPathsKeyword = true;
                }
                else if ( foundPathsKeyword )
                {
                    if ( line.startsWith( "/", Qt::CaseInsensitive ) )
                    {
                        // Detected end of keyword data section
                        return;
                    }
                    else if ( line.startsWith( "--", Qt::CaseInsensitive ) )
                    {
                        continue;
                    }
                    else
                    {
                        // Replace tab with space to be able to split the string using space as splitter
                        line.replace( "\t", " " );

                        // Remove character ' used to mark start and end of fault name, possibly also around face
                        // definition; 'I+'
                        line.remove( "'" );

                        QStringList entries = RiaTextStringTools::splitSkipEmptyParts( line );
                        if ( entries.size() < 2 )
                        {
                            continue;
                        }

                        pathAliasDefinitions->push_back( std::make_pair( entries[0], entries[1] ) );
                    }
                }
            }
        }
    } while ( !data.atEnd() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::findGridKeywordPositions( const std::vector<RifKeywordAndFilePos>& keywords,
                                                         qint64*                                  coordPos,
                                                         qint64*                                  zcornPos,
                                                         qint64*                                  specgridPos,
                                                         qint64*                                  actnumPos,
                                                         qint64*                                  mapaxesPos,
                                                         qint64*                                  gridunitPos )
{
    CVF_ASSERT( coordPos && zcornPos && specgridPos && actnumPos && mapaxesPos && gridunitPos );

    size_t i;
    for ( i = 0; i < keywords.size(); i++ )
    {
        if ( keywords[i].keyword == "COORD" )
        {
            *coordPos = keywords[i].filePos;
        }
        else if ( keywords[i].keyword == "ZCORN" )
        {
            *zcornPos = keywords[i].filePos;
        }
        else if ( keywords[i].keyword == "SPECGRID" )
        {
            *specgridPos = keywords[i].filePos;
        }
        else if ( keywords[i].keyword == "ACTNUM" )
        {
            *actnumPos = keywords[i].filePos;
        }
        else if ( keywords[i].keyword == "MAPAXES" )
        {
            *mapaxesPos = keywords[i].filePos;
        }
        else if ( keywords[i].keyword == "GRIDUNIT" )
        {
            *gridunitPos = keywords[i].filePos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaults( const QString&                           fileName,
                                           const std::vector<RifKeywordAndFilePos>& fileKeywords,
                                           cvf::Collection<RigFault>*               faults )
{
    QFile data( fileName );
    if ( !data.open( QFile::ReadOnly ) )
    {
        return;
    }

    for ( size_t i = 0; i < fileKeywords.size(); i++ )
    {
        if ( fileKeywords[i].keyword.compare( editKeyword, Qt::CaseInsensitive ) == 0 )
        {
            return;
        }
        else if ( fileKeywords[i].keyword.compare( faultsKeyword, Qt::CaseInsensitive ) != 0 )
        {
            continue;
        }

        qint64 filePos = fileKeywords[i].filePos;

        bool isEditKeywordDetected = false;
        readFaults( data, filePos, faults, &isEditKeywordDetected );

        if ( isEditKeywordDetected )
        {
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::parseAndReadFaults( const QString& fileName, cvf::Collection<RigFault>* faults )
{
    QFile data( fileName );
    if ( !data.open( QFile::ReadOnly ) )
    {
        return;
    }

    qint64 filePos = findKeyword( faultsKeyword, data, 0 );

    while ( filePos != -1 )
    {
        readFaults( data, filePos, faults, nullptr );
        filePos = findKeyword( faultsKeyword, data, filePos );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaultsInGridSection( const QString&             fileName,
                                                        cvf::Collection<RigFault>* faults,
                                                        std::vector<QString>*      filenamesWithFaults,
                                                        const QString&             faultIncludeFileAbsolutePathPrefix )
{
    QFile data( fileName );
    if ( !data.open( QFile::ReadOnly ) )
    {
        return;
    }

    // Search for keyword grid
    qint64 gridPos = findKeyword( gridKeyword, data, 0 );
    if ( gridPos < 0 )
    {
        return;
    }

    bool isEditKeywordDetected = false;

    std::vector<std::pair<QString, QString>> pathAliasDefinitions;
    parseAndReadPathAliasKeyword( fileName, &pathAliasDefinitions );

    readFaultsAndParseIncludeStatementsRecursively( data,
                                                    gridPos,
                                                    pathAliasDefinitions,
                                                    faults,
                                                    filenamesWithFaults,
                                                    &isEditKeywordDetected,
                                                    faultIncludeFileAbsolutePathPrefix );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifEclipseInputFileTools::findFaultByName( const cvf::Collection<RigFault>& faults, const QString& name )
{
    for ( size_t i = 0; i < faults.size(); i++ )
    {
        if ( faults.at( i )->name() == name )
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
qint64 RifEclipseInputFileTools::findKeyword( const QString& keyword, QFile& file, qint64 startPos )
{
    QString line;

    file.seek( startPos );

    do
    {
        line = file.readLine();
        line = line.trimmed();

        if ( line.startsWith( "--", Qt::CaseInsensitive ) )
        {
            continue;
        }

        if ( line.startsWith( keyword, Qt::CaseInsensitive ) )
        {
            return file.pos();
        }

    } while ( !file.atEnd() );

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::writeFaultLine( QTextStream&                       stream,
                                               QString                            faultName,
                                               size_t                             i,
                                               size_t                             j,
                                               size_t                             startK,
                                               size_t                             endK,
                                               cvf::StructGridInterface::FaceType faceType )
{
    // Convert indices to eclipse format
    i++;
    j++;
    startK++;
    endK++;

    stream << "'" << faultName << "'"
           << "     " << i << "   " << i << "     " << j << "   " << j << "     " << startK << "   " << endK << "     "
           << faultFaceText( faceType ) << "      / ";
    stream << '\n';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseInputFileTools::faultFaceText( cvf::StructGridInterface::FaceType faceType )
{
    switch ( faceType )
    {
        case cvf::StructGridInterface::POS_I:
            return QString( "I " );
        case cvf::StructGridInterface::NEG_I:
            return QString( "I-" );
        case cvf::StructGridInterface::POS_J:
            return QString( "J " );
        case cvf::StructGridInterface::NEG_J:
            return QString( "J-" );
        case cvf::StructGridInterface::POS_K:
            return QString( "K " );
        case cvf::StructGridInterface::NEG_K:
            return QString( "K-" );
        default:
            CVF_ASSERT( false );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readFaultsAndParseIncludeStatementsRecursively( QFile& file,
                                                                               qint64 startPos,
                                                                               const std::vector<std::pair<QString, QString>>& pathAliasDefinitions,
                                                                               cvf::Collection<RigFault>* faults,
                                                                               std::vector<QString>*      filenamesWithFaults,
                                                                               bool*                      isEditKeywordDetected,
                                                                               const QString& faultIncludeFileAbsolutePathPrefix )
{
    QString line;

    if ( !file.seek( startPos ) )
    {
        return false;
    }

    bool continueParsing = true;

    do
    {
        line = file.readLine();
        line = line.trimmed();

        if ( line.startsWith( "--", Qt::CaseInsensitive ) )
        {
            continue;
        }
        else if ( line.startsWith( editKeyword, Qt::CaseInsensitive ) )
        {
            if ( isEditKeywordDetected )
            {
                *isEditKeywordDetected = true;
            }

            return false;
        }

        if ( line.startsWith( includeKeyword, Qt::CaseInsensitive ) )
        {
            line = file.readLine();
            line = line.trimmed();

            while ( line.startsWith( "--", Qt::CaseInsensitive ) )
            {
                line = file.readLine();
                line = line.trimmed();
            }

            int firstQuote = line.indexOf( "'" );
            int lastQuote  = line.lastIndexOf( "'" );

            if ( firstQuote >= 0 && lastQuote >= 0 && firstQuote != lastQuote )
            {
                QDir currentFileFolder;
                {
                    QFileInfo fi( file.fileName() );
                    currentFileFolder = fi.absoluteDir();
                }

                // Read include file name, and both relative and absolute path is supported
                QString includeFilename = line.mid( firstQuote + 1, lastQuote - firstQuote - 1 );

                for ( auto entry : pathAliasDefinitions )
                {
                    QString textToReplace = "$" + entry.first;
                    includeFilename.replace( textToReplace, entry.second );
                }

#ifdef WIN32
                if ( includeFilename.startsWith( '/' ) )
                {
                    // Absolute UNIX path, prefix on Windows
                    includeFilename = faultIncludeFileAbsolutePathPrefix + includeFilename;
                }
#endif

                QFileInfo fi( currentFileFolder, includeFilename );
                if ( fi.exists() )
                {
                    QString absoluteFilename = fi.canonicalFilePath();
                    QFile   includeFile( absoluteFilename );
                    if ( includeFile.open( QFile::ReadOnly ) )
                    {
                        // qDebug() << "Found include statement, and start parsing of\n  " << absoluteFilename;

                        if ( !readFaultsAndParseIncludeStatementsRecursively( includeFile,
                                                                              0,
                                                                              pathAliasDefinitions,
                                                                              faults,
                                                                              filenamesWithFaults,
                                                                              isEditKeywordDetected,
                                                                              faultIncludeFileAbsolutePathPrefix ) )
                        {
                            qDebug() << "Error when parsing include file : " << absoluteFilename;
                        }
                    }
                }
            }
        }
        else if ( line.startsWith( faultsKeyword, Qt::CaseInsensitive ) )
        {
            if ( !line.contains( "/" ) )
            {
                readFaults( file, file.pos(), faults, isEditKeywordDetected );
                filenamesWithFaults->push_back( file.fileName() );
            }
        }

        if ( isEditKeywordDetected && *isEditKeywordDetected )
        {
            continueParsing = false;
        }

        if ( file.atEnd() )
        {
            continueParsing = false;
        }

    } while ( continueParsing );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( const QString& keyword,
                                                                                const QString& keywordToStopParsing,
                                                                                const std::vector<std::pair<QString, QString>>& pathAliasDefinitions,
                                                                                const QString&        includeStatementAbsolutePathPrefix,
                                                                                QFile&                file,
                                                                                qint64                startPos,
                                                                                QStringList&          keywordDataContent,
                                                                                std::vector<QString>& filenamesContainingKeyword,
                                                                                bool&                 isStopParsingKeywordDetected )
{
    QString line;

    if ( !file.seek( startPos ) )
    {
        return false;
    }

    bool continueParsing = true;

    do
    {
        line = file.readLine();
        line = line.trimmed();

        if ( line.startsWith( "--", Qt::CaseInsensitive ) )
        {
            continue;
        }

        if ( !keywordToStopParsing.isEmpty() && line.startsWith( keywordToStopParsing, Qt::CaseInsensitive ) )
        {
            isStopParsingKeywordDetected = true;

            return false;
        }

        if ( line.startsWith( includeKeyword, Qt::CaseInsensitive ) )
        {
            line = file.readLine();
            line = line.trimmed();

            while ( line.startsWith( "--", Qt::CaseInsensitive ) )
            {
                line = file.readLine();
                line = line.trimmed();
            }

            int firstQuote = line.indexOf( "'" );
            int lastQuote  = line.lastIndexOf( "'" );

            if ( firstQuote >= 0 && lastQuote >= 0 && firstQuote != lastQuote )
            {
                QDir currentFileFolder;
                {
                    QFileInfo fi( file.fileName() );
                    currentFileFolder = fi.absoluteDir();
                }

                // Read include file name, and both relative and absolute path is supported
                QString includeFilename = line.mid( firstQuote + 1, lastQuote - firstQuote - 1 );

                for ( auto entry : pathAliasDefinitions )
                {
                    QString textToReplace = "$" + entry.first;
                    includeFilename.replace( textToReplace, entry.second );
                }

#ifdef WIN32
                if ( includeFilename.startsWith( '/' ) )
                {
                    // Absolute UNIX path, prefix on Windows
                    includeFilename = includeStatementAbsolutePathPrefix + includeFilename;
                }
#endif

                QFileInfo fi( currentFileFolder, includeFilename );
                if ( fi.exists() )
                {
                    QString absoluteFilename = fi.canonicalFilePath();
                    QFile   includeFile( absoluteFilename );
                    if ( includeFile.open( QFile::ReadOnly ) )
                    {
                        if ( !readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                               keywordToStopParsing,
                                                                               pathAliasDefinitions,
                                                                               includeStatementAbsolutePathPrefix,
                                                                               includeFile,
                                                                               0,
                                                                               keywordDataContent,
                                                                               filenamesContainingKeyword,
                                                                               isStopParsingKeywordDetected ) )
                        {
                            qDebug() << "Error when parsing include file : " << absoluteFilename;
                        }
                    }
                }
            }
        }
        else if ( line.startsWith( keyword, Qt::CaseInsensitive ) )
        {
            if ( !line.contains( "/" ) )
            {
                readKeywordDataContent( file, file.pos(), keywordDataContent, isStopParsingKeywordDetected );
                filenamesContainingKeyword.push_back( file.fileName() );
            }
        }

        if ( isStopParsingKeywordDetected )
        {
            continueParsing = false;
        }

        if ( file.atEnd() )
        {
            continueParsing = false;
        }

    } while ( continueParsing );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readKeywordDataContent( QFile& data, qint64 filePos, QStringList& textContent, bool& isStopParsingKeywordDetected )
{
    if ( !data.seek( filePos ) )
    {
        return;
    }

    // This function assumes the keyword is read from file, and the file pointer is pointing to the first line
    // containing data for the keyword

    do
    {
        QString line = data.readLine();
        line         = line.trimmed();

        if ( line.startsWith( "--", Qt::CaseInsensitive ) )
        {
            // Skip comment lines
            continue;
        }
        else if ( line.startsWith( "/", Qt::CaseInsensitive ) )
        {
            // Detected end of keyword data section
            return;
        }
        else if ( line.startsWith( editKeyword, Qt::CaseInsensitive ) )
        {
            // End parsing when edit keyword is detected
            isStopParsingKeywordDetected = true;

            return;
        }
        else if ( line[0].isLetter() )
        {
            // If a letter is starting the line, this is a new keyword
            return;
        }

        if ( !line.isEmpty() )
        {
            textContent.push_back( line );
        }

    } while ( !data.atEnd() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifEclipseInputFileTools::readUnitSystem( QFile& file, qint64 gridunitPos )
{
    bool        stopParsing = false;
    QStringList unitText;
    readKeywordDataContent( file, gridunitPos, unitText, stopParsing );
    for ( QString unitString : unitText )
    {
        if ( unitString.contains( "FEET", Qt::CaseInsensitive ) )
        {
            return RiaDefines::EclipseUnitSystem::UNITS_FIELD;
        }
        else if ( unitString.contains( "CM", Qt::CaseInsensitive ) )
        {
            return RiaDefines::EclipseUnitSystem::UNITS_LAB;
        }
        else if ( unitString.contains( "MET", Qt::CaseInsensitive ) )
        {
            return RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        }
    }
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::StructGridInterface::FaceEnum RifEclipseInputFileTools::faceEnumFromText( const QString& faceString )
{
    QString upperCaseText = faceString.toUpper().trimmed();

    if ( upperCaseText.size() > 1 )
    {
        QString firstTwoChars = upperCaseText.mid( 0, 2 );

        if ( firstTwoChars == "X+" || firstTwoChars == "I+" ) return cvf::StructGridInterface::POS_I;
        if ( firstTwoChars == "Y+" || firstTwoChars == "J+" ) return cvf::StructGridInterface::POS_J;
        if ( firstTwoChars == "Z+" || firstTwoChars == "K+" ) return cvf::StructGridInterface::POS_K;

        if ( firstTwoChars == "X-" || firstTwoChars == "I-" ) return cvf::StructGridInterface::NEG_I;
        if ( firstTwoChars == "Y-" || firstTwoChars == "J-" ) return cvf::StructGridInterface::NEG_J;
        if ( firstTwoChars == "Z-" || firstTwoChars == "K-" ) return cvf::StructGridInterface::NEG_K;
    }

    if ( upperCaseText.size() > 0 )
    {
        QString firstChar = upperCaseText.mid( 0, 1 );

        if ( firstChar == "X" || firstChar == "I" ) return cvf::StructGridInterface::POS_I;
        if ( firstChar == "Y" || firstChar == "J" ) return cvf::StructGridInterface::POS_J;
        if ( firstChar == "Z" || firstChar == "K" ) return cvf::StructGridInterface::POS_K;
    }

    return cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputFileTools::hasGridData( const QString& fileName )
{
    // Look for keyword "COORD" in file
    // NOTE: If readKeywordAndValues is slow for large .GRDECL files, consider function in RifEclipseTextFileReader
    // reading line for line and returns true on first hit of keyword. This prevents reading entire file on large cases

    const auto keywordAndValues = RifEclipseTextFileReader::readKeywordAndValues( fileName.toStdString() );
    auto       coordKeywordItr  = std::find_if( keywordAndValues.begin(),
                                         keywordAndValues.end(),
                                         []( const auto& keywordAndValue ) { return keywordAndValue.keyword == "COORD"; } );

    return coordKeywordItr != keywordAndValues.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseInputFileTools::readKeywordContentFromFile( const QString& keyword, const QString& keywordToStopParsing, QFile& file )
{
    std::vector<std::pair<QString, QString>> pathAliasDefinitions;
    const QString                            includeStatementAbsolutePathPrefix;
    const qint64                             startPositionInFile = 0;
    QStringList                              keywordContent;
    std::vector<QString>                     fileNamesContainingKeyword;
    bool                                     isStopParsingKeywordDetected = false;

    RifEclipseInputFileTools::readKeywordAndParseIncludeStatementsRecursively( keyword,
                                                                               keywordToStopParsing,
                                                                               pathAliasDefinitions,
                                                                               includeStatementAbsolutePathPrefix,
                                                                               file,
                                                                               startPositionInFile,
                                                                               keywordContent,
                                                                               fileNamesContainingKeyword,
                                                                               isStopParsingKeywordDetected );

    return keywordContent;
}

//--------------------------------------------------------------------------------------------------
/// The file pointer is pointing at the line following the FAULTS keyword.
/// Parse content of this keyword until end of file or
/// end of keyword when a single line with '/' is found
//--------------------------------------------------------------------------------------------------
void RifEclipseInputFileTools::readFaults( QFile& data, qint64 filePos, cvf::Collection<RigFault>* faults, bool* isEditKeywordDetected )
{
    if ( !data.seek( filePos ) )
    {
        return;
    }

    // qDebug() << "Reading faults from\n  " << data.fileName();

    std::set<QString> errorMessages;
    std::set<QString> warningMessages;

    RigFault* fault = nullptr;

    do
    {
        QString line = data.readLine();
        line         = line.trimmed();

        if ( line.startsWith( "--", Qt::CaseInsensitive ) )
        {
            // Skip comment lines
            continue;
        }
        else if ( line.startsWith( "/", Qt::CaseInsensitive ) )
        {
            // Detected end of keyword data section
            break;
        }
        else if ( line.startsWith( editKeyword, Qt::CaseInsensitive ) )
        {
            // End parsing when edit keyword is detected

            if ( isEditKeywordDetected )
            {
                *isEditKeywordDetected = true;
            }

            break;
        }

        // Replace tab with space to be able to split the string using space as splitter
        line.replace( "\t", " " );

        QStringList entries;
        bool        insideQuotes = false;
        QString     column;
        for ( int i = 0; i < line.length(); ++i )
        {
            if ( line[i] == '\'' )
            {
                insideQuotes = !insideQuotes;
            }
            else if ( line[i] == ' ' && !insideQuotes )
            {
                if ( column.length() > 0 )
                {
                    entries.push_back( column );
                }
                column.clear();
            }
            else
            {
                column += line[i];
            }
        }

        if ( entries.size() < 8 )
        {
            continue;
        }

        QString faultName = entries[0];

        if ( faultName.contains( ' ' ) )
        {
            errorMessages.insert( QString( "Fault name '%1' contains spaces" ).arg( faultName ) );
            continue;
        }
        else if ( faultName.length() > 8 )
        {
            // Keep going anyway, eclipse files sometimes have longer than
            // the specified 8 characters in the name without Eclipse complaining
            warningMessages.insert( QString( "Fault name '%1' is longer than 8 characters" ).arg( faultName ) );
        }

        int i1, i2, j1, j2, k1, k2;
        i1 = entries[1].toInt();
        i2 = entries[2].toInt();
        j1 = entries[3].toInt();
        j2 = entries[4].toInt();
        k1 = entries[5].toInt();
        k2 = entries[6].toInt();

        QString faceString = entries[7];

        cvf::StructGridInterface::FaceEnum cellFaceEnum = RifEclipseInputFileTools::faceEnumFromText( faceString );

        // Adjust from 1-based to 0-based cell indices
        // Guard against invalid cell ranges by limiting lowest possible range value to zero
        cvf::CellRange cellrange( std::max( i1 - 1, 0 ),
                                  std::max( j1 - 1, 0 ),
                                  std::max( k1 - 1, 0 ),
                                  std::max( i2 - 1, 0 ),
                                  std::max( j2 - 1, 0 ),
                                  std::max( k2 - 1, 0 ) );

        if ( !( fault && fault->name() == faultName ) )
        {
            if ( findFaultByName( *faults, faultName ) == cvf::UNDEFINED_SIZE_T )
            {
                RigFault* newFault = new RigFault;
                newFault->setName( faultName );

                faults->push_back( newFault );
            }

            size_t faultIndex = findFaultByName( *faults, faultName );
            if ( faultIndex == cvf::UNDEFINED_SIZE_T )
            {
                CVF_ASSERT( faultIndex != cvf::UNDEFINED_SIZE_T );
                continue;
            }

            fault = faults->at( faultIndex );
        }

        CVF_ASSERT( fault );

        fault->addCellRangeForFace( cellFaceEnum, cellrange );

    } while ( !data.atEnd() );

    for ( QString errorMessage : errorMessages )
    {
        RiaLogging::error( errorMessage );
    }

    for ( QString warningMessage : warningMessages )
    {
        RiaLogging::warning( warningMessage );
    }
}
