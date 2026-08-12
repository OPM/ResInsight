/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigNestedHybridGridResultTools.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"
#include "RiaStringEncodingTools.h"

#include "RifEclipseKeywordContent.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseTextFileReader.h"
#include "RifInputPropertyLoader.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNestedHybridGridFipnestCodec.h"
#include "RigNestedHybridGridReconstructor.h"
#include "RigTypeSafeIndex.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"

#include "ert/ecl/ecl_file.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <cmath>
#include <limits>
#include <map>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Nested hybrid grid: the grid is a single flat EGRID, and the per-cell nesting level is provided
/// in a sidecar GRDECL file named "<grid-basename>_REFINE.grdecl" next to the grid file.
/// Returns the path to that sidecar if it exists, otherwise an empty string.
//--------------------------------------------------------------------------------------------------
QString RigNestedHybridGridResultTools::refineSidecarFilePath( const QString& gridFileName )
{
    QFileInfo gridFileInfo( gridFileName );
    if ( !gridFileInfo.exists() ) return {};

    QDir          dir      = gridFileInfo.absoluteDir();
    const QString baseName = gridFileInfo.completeBaseName();

    // Filename convention, e.g. DROGON_NESTED.EGRID -> DROGON_NESTED_REFINE.grdecl
    const QString     suffix     = "_" + RiaResultNames::refine();
    const QStringList candidates = { baseName + suffix + ".grdecl", baseName + suffix + ".GRDECL" };
    for ( const QString& candidate : candidates )
    {
        QString path = dir.absoluteFilePath( candidate );
        if ( QFile::exists( path ) ) return path;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Nested hybrid grid: the parent mapping is provided in a sidecar GRDECL file named
/// "<grid-basename>_OLDIJK.grdecl" next to the grid file. It holds, per flat cell, the original
/// coarse cell IJK (OLDI/OLDJ/OLDK) and the local refined coordinates (TMPI/TMPJ/TMPK).
/// Returns its path if it exists.
//--------------------------------------------------------------------------------------------------
QString RigNestedHybridGridResultTools::oldIjkSidecarFilePath( const QString& gridFileName )
{
    QFileInfo gridFileInfo( gridFileName );
    if ( !gridFileInfo.exists() ) return {};

    QDir          dir      = gridFileInfo.absoluteDir();
    const QString baseName = gridFileInfo.completeBaseName();

    const QStringList candidates = { baseName + "_OLDIJK.grdecl", baseName + "_OLDIJK.GRDECL" };
    for ( const QString& candidate : candidates )
    {
        QString path = dir.absoluteFilePath( candidate );
        if ( QFile::exists( path ) ) return path;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Nested hybrid grid: the compact parent-child encoding (FIPNEST/FIPSLOT/REFINE, see
/// RigNestedHybridGridFipnestCodec) in a sidecar GRDECL file named "<grid-basename>_FIPNEST.grdecl".
/// Returns its path if it exists.
//--------------------------------------------------------------------------------------------------
QString RigNestedHybridGridResultTools::fipnestSidecarFilePath( const QString& gridFileName )
{
    QFileInfo gridFileInfo( gridFileName );
    if ( !gridFileInfo.exists() ) return {};

    QDir          dir      = gridFileInfo.absoluteDir();
    const QString baseName = gridFileInfo.completeBaseName();

    const QStringList candidates = { baseName + "_FIPNEST.grdecl", baseName + "_FIPNEST.GRDECL" };
    for ( const QString& candidate : candidates )
    {
        QString path = dir.absoluteFilePath( candidate );
        if ( QFile::exists( path ) ) return path;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Write integer keywords to a GRDECL text file, run-length encoding repeated values as "N*V".
//--------------------------------------------------------------------------------------------------
bool RigNestedHybridGridResultTools::writeIntKeywordsToGrdeclFile( const QString& filePath,
                                                                   const std::vector<std::pair<QString, const std::vector<int>*>>& keywords )
{
    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    QTextStream out( &file );
    out << "-- Nested hybrid grid parent-child arrays exported by ResInsight (#14510)\n";

    const int maxTokensPerLine = 12;

    for ( const auto& [keyword, values] : keywords )
    {
        if ( !values ) continue;

        out << keyword << "\n";

        int  tokensOnLine = 0;
        auto emitToken    = [&]( const QString& token )
        {
            out << ' ' << token;
            if ( ++tokensOnLine == maxTokensPerLine )
            {
                out << "\n";
                tokensOnLine = 0;
            }
        };

        size_t i = 0;
        while ( i < values->size() )
        {
            const int v   = ( *values )[i];
            size_t    run = 1;
            while ( i + run < values->size() && ( *values )[i + run] == v )
                run++;

            // Only run-length encode non-negative values; "N*-V" is not universally parsed.
            if ( run >= 4 && v >= 0 )
            {
                emitToken( QString( "%1*%2" ).arg( run ).arg( v ) );
            }
            else
            {
                for ( size_t r = 0; r < run; r++ )
                    emitToken( QString::number( v ) );
            }
            i += run;
        }

        if ( tokensOnLine != 0 ) out << "\n";
        out << " /\n";
    }

    out.flush();
    return out.status() == QTextStream::Ok && file.error() == QFile::NoError;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridResultTools::importRefineSidecarIfPresent( const QString&                     gridFileName,
                                                                   RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                   RigEclipseCaseData*                eclipseCaseData )
{
    if ( !inputPropertyCollection || !eclipseCaseData ) return;

    // Skip if the REFINE property is already loaded (e.g. restored from a saved project file)
    for ( const RimEclipseInputProperty* prop : inputPropertyCollection->items() )
    {
        if ( prop->resultName() == RiaResultNames::refine() ) return;
    }

    const QString sidecarPath = refineSidecarFilePath( gridFileName );
    if ( sidecarPath.isEmpty() ) return;

    RiaLogging::info( QString( "Nested hybrid grid: loading REFINE property from %1" ).arg( sidecarPath ).toStdString() );

    RifInputPropertyLoader::loadAndSynchronizeInputProperties( inputPropertyCollection, eclipseCaseData, std::vector<QString>{ sidecarPath }, false );
}

//--------------------------------------------------------------------------------------------------
/// Load the OLDIJK sidecar (OLDI/OLDJ/OLDK/TMPI/TMPJ/TMPK) as input properties so the parent-cell
/// mapping is visible and scriptable, mirroring the REFINE property.
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridResultTools::importOldIjkSidecarIfPresent( const QString&                     gridFileName,
                                                                   RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                   RigEclipseCaseData*                eclipseCaseData )
{
    if ( !inputPropertyCollection || !eclipseCaseData ) return;

    // Skip if the OLDIJK properties are already loaded (e.g. restored from a saved project file)
    for ( const RimEclipseInputProperty* prop : inputPropertyCollection->items() )
    {
        if ( prop->resultName().compare( "OLDI", Qt::CaseInsensitive ) == 0 ) return;
    }

    const QString sidecarPath = oldIjkSidecarFilePath( gridFileName );
    if ( sidecarPath.isEmpty() ) return;

    RiaLogging::info( QString( "Nested hybrid grid: loading OLDIJK properties from %1" ).arg( sidecarPath ).toStdString() );

    RifInputPropertyLoader::loadAndSynchronizeInputProperties( inputPropertyCollection, eclipseCaseData, std::vector<QString>{ sidecarPath }, false );
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Auto-export the compact FIPNEST/FIPSLOT/REFINE parent-child encoding next to the grid file after
/// a successful sidecar-based reconstruction (#14510), unless the file already exists. The arrays
/// are computed from the same sidecar input the reconstruction used.
//--------------------------------------------------------------------------------------------------
void exportFipnestSidecarIfAbsent( const QString&                                             gridFileName,
                                   const RigNestedHybridGridReconstructor::NestedHybridInput& input,
                                   const RigEclipseCaseData*                                  eclipseCaseData )
{
    const QString existing = RigNestedHybridGridResultTools::fipnestSidecarFilePath( gridFileName );
    if ( !existing.isEmpty() )
    {
        RiaLogging::info( QString( "Nested hybrid grid: FIPNEST sidecar already present: %1" ).arg( existing ).toStdString() );
        return;
    }

    const RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
    const size_t       nx       = mainGrid->cellCountI();
    const size_t       ny       = mainGrid->cellCountJ();
    const size_t       nz       = mainGrid->cellCountK();

    const auto arrays = RigNestedHybridGridFipnestCodec::computeParentChildArrays( input, nx, ny, nz );
    if ( arrays.fipnest.empty() )
    {
        RiaLogging::warning( "Nested hybrid grid: could not compute FIPNEST arrays; sidecar not exported." );
        return;
    }
    if ( arrays.unresolvedRefinedCells > 0 )
    {
        RiaLogging::warning(
            QString( "Nested hybrid grid: %1 refined cells could not be encoded in FIPNEST." ).arg( arrays.unresolvedRefinedCells ).toStdString() );
    }

    // Note: RifEclipseTextFileReader parses GRDECL values as float, so FIPNEST indices written here
    // stay exact only up to 2^24 cells (~16.7M); the INIT-embedded INTE path has no such limit.
    QFileInfo     gridFileInfo( gridFileName );
    const QString path = gridFileInfo.absoluteDir().absoluteFilePath( gridFileInfo.completeBaseName() + "_FIPNEST.grdecl" );

    const std::vector<std::pair<QString, const std::vector<int>*>> keywords = { { "FIPNEST", &arrays.fipnest },
                                                                                { "FIPSLOT", &arrays.fipslot },
                                                                                { RiaResultNames::refine(), &input.refine } };
    if ( RigNestedHybridGridResultTools::writeIntKeywordsToGrdeclFile( path, keywords ) )
    {
        RiaLogging::info( QString( "Nested hybrid grid: exported FIPNEST sidecar to %1" ).arg( path ).toStdString() );
    }
    else
    {
        RiaLogging::warning( QString( "Nested hybrid grid: failed to write FIPNEST sidecar %1" ).arg( path ).toStdString() );
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridResultTools::reconstructNestedHybridGridIfPresent( const QString& gridFileName, RigEclipseCaseData* eclipseCaseData )
{
    if ( !eclipseCaseData || !eclipseCaseData->mainGrid() ) return;

    const QString refinePath = refineSidecarFilePath( gridFileName );
    const QString oldIjkPath = oldIjkSidecarFilePath( gridFileName );
    if ( refinePath.isEmpty() || oldIjkPath.isEmpty() ) return;

    // Read a single named integer keyword (rounded from the file's float values) from parsed content.
    auto readIntKeyword = []( const std::vector<RifEclipseKeywordContent>& content, const QString& keyword )
    {
        std::vector<int> result;
        for ( const auto& kw : content )
        {
            if ( QString::fromStdString( kw.keyword ).compare( keyword, Qt::CaseInsensitive ) == 0 )
            {
                result.reserve( kw.values.size() );
                for ( float v : kw.values )
                    result.push_back( static_cast<int>( std::lround( v ) ) );
                break;
            }
        }
        return result;
    };

    auto refineContent = RifEclipseTextFileReader::readKeywordAndValues( refinePath.toStdString() );
    auto oldIjkContent = RifEclipseTextFileReader::readKeywordAndValues( oldIjkPath.toStdString() );

    RigNestedHybridGridReconstructor::NestedHybridInput input;
    input.refine = readIntKeyword( refineContent, RiaResultNames::refine() );
    input.oldI   = readIntKeyword( oldIjkContent, "OLDI" );
    input.oldJ   = readIntKeyword( oldIjkContent, "OLDJ" );
    input.oldK   = readIntKeyword( oldIjkContent, "OLDK" );
    input.tmpI   = readIntKeyword( oldIjkContent, "TMPI" );
    input.tmpJ   = readIntKeyword( oldIjkContent, "TMPJ" );
    input.tmpK   = readIntKeyword( oldIjkContent, "TMPK" );

    QString errorMessage;
    if ( RigNestedHybridGridReconstructor::reconstruct( eclipseCaseData, input, &errorMessage ) )
    {
        exportFipnestSidecarIfAbsent( gridFileName, input, eclipseCaseData );
    }

    // The caller computes grid caches (search tree, faults, NNCs) once, after this reconstruction, so
    // that the expensive geometric passes run on the clean grid rather than the flat overlapping one.
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Open the INIT file next to the grid file, or nullptr if there is none. The caller owns the handle.
//--------------------------------------------------------------------------------------------------
ecl_file_type* openInitFileNextToGrid( const QString& gridFileName, QString* initFileNameOut = nullptr )
{
    QStringList fileSet;
    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( gridFileName, &fileSet ) ) return nullptr;

    const QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType( fileSet, ECL_INIT_FILE );
    if ( initFileName.isEmpty() ) return nullptr;

    if ( initFileNameOut ) *initFileNameOut = initFileName;
    return ecl_file_open( RiaStringEncodingTools::toNativeEncoded( initFileName ).data(), ECL_FILE_CLOSE_STREAM );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNestedHybridGridResultTools::initFileHasFipnest( const QString& gridFileName )
{
    ecl_file_type* initFile = openInitFileNextToGrid( gridFileName );
    if ( !initFile ) return false;

    const bool hasFipnest = ecl_file_has_kw( initFile, "FIPNEST" );
    ecl_file_close( initFile );

    return hasFipnest;
}

//--------------------------------------------------------------------------------------------------
/// Reconstruct the nested hybrid grid LGR hierarchy from the FIPNEST/FIPSLOT/REFINE arrays embedded
/// in the INIT file - the sidecar-free import path prototyped in #14510. The REFINE levels are
/// stored as an input-property result before the reconstruction so the per-level result helpers
/// (and the LGR-cell extension) see them, mirroring the sidecar path.
//--------------------------------------------------------------------------------------------------
bool RigNestedHybridGridResultTools::reconstructNestedHybridGridFromInitFile( const QString& gridFileName, RigEclipseCaseData* eclipseCaseData )
{
    if ( !eclipseCaseData || !eclipseCaseData->mainGrid() ) return false;

    QString        initFileName;
    ecl_file_type* initFile = openInitFileNextToGrid( gridFileName, &initFileName );
    if ( !initFile ) return false;

    auto readIntKeyword = [&]( const char* keyword, std::vector<int>* values )
    { return ecl_file_has_kw( initFile, keyword ) && RifEclipseOutputFileTools::keywordData( initFile, keyword, 0, values ); };

    std::vector<int> fipnest, fipslot, refine;
    const bool       haveFipnest = readIntKeyword( "FIPNEST", &fipnest );
    const bool       haveAll     = haveFipnest && readIntKeyword( "FIPSLOT", &fipslot ) &&
                         readIntKeyword( RiaResultNames::refine().toLatin1().data(), &refine );
    ecl_file_close( initFile );

    if ( !haveFipnest ) return false;
    if ( !haveAll )
    {
        RiaLogging::warning( QString( "Nested hybrid grid: %1 contains FIPNEST but not its FIPSLOT/REFINE companions; "
                                      "falling back to sidecar import." )
                                 .arg( initFileName )
                                 .toStdString() );
        return false;
    }

    const RigMainGrid* mainGrid  = eclipseCaseData->mainGrid();
    const size_t       nx        = mainGrid->cellCountI();
    const size_t       ny        = mainGrid->cellCountJ();
    const size_t       nz        = mainGrid->cellCountK();
    const size_t       cellCount = nx * ny * nz;
    if ( fipnest.size() != cellCount || fipslot.size() != cellCount || refine.size() != cellCount )
    {
        RiaLogging::warning( QString( "Nested hybrid grid: FIPNEST/FIPSLOT/REFINE in %1 do not cover all %2 cells; "
                                      "falling back to sidecar import." )
                                 .arg( initFileName )
                                 .arg( cellCount )
                                 .toStdString() );
        return false;
    }

    RiaLogging::info(
        QString( "Nested hybrid grid: reconstructing from the FIPNEST/FIPSLOT/REFINE arrays in %1" ).arg( initFileName ).toStdString() );

    QString    warnings;
    const auto input = RigNestedHybridGridFipnestCodec::buildInputFromParentChildArrays( fipnest, fipslot, refine, nx, ny, nz, &warnings );
    for ( const QString& line : warnings.split( '\n', Qt::SkipEmptyParts ) )
    {
        RiaLogging::warning( ( "Nested hybrid grid: " + line ).toStdString() );
    }

    // Make the refinement levels available as the REFINE result (used by the per-level aggregation
    // helpers) unless the sidecar import already provided it. Created before the reconstruction so
    // the full-length array is extended to cover the new LGR cells.
    if ( RigCaseCellResultsData* results = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        RigEclipseResultAddress refineAddr( RiaDefines::ResultCatType::INPUT_PROPERTY,
                                            RiaDefines::ResultDataType::INTEGER,
                                            RiaResultNames::refine() );
        if ( !results->hasResultEntry( refineAddr ) )
        {
            results->createResultEntry( refineAddr, false );
            if ( auto* timesteps = results->modifiableCellScalarResultTimesteps( refineAddr ) )
            {
                timesteps->push_back( std::vector<double>( refine.begin(), refine.end() ) );
            }
        }
    }

    QString errorMessage;
    return RigNestedHybridGridReconstructor::reconstruct( eclipseCaseData, input, &errorMessage );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridResultTools::extendLgrResults( RigCaseCellResultsData* cellResults )
{
    if ( !cellResults ) return;

    RigMainGrid*       mainGrid       = cellResults->mainGrid();
    RigActiveCellInfo* activeCellInfo = cellResults->activeCellInfo();
    if ( !mainGrid || !activeCellInfo || mainGrid->nestedHybridLgrSourceCells().empty() ) return;

    const size_t activeCellCount = activeCellInfo->reservoirActiveCellCount();

    for ( const RigEclipseResultAddress& addr : cellResults->existingResults() )
    {
        std::vector<std::vector<double>>* timesteps = cellResults->modifiableCellScalarResultTimesteps( addr );
        if ( !timesteps ) continue;

        for ( std::vector<double>& values : *timesteps )
        {
            // Only active-cell-indexed arrays (length below the active-cell count). Full-length
            // (all-cells) arrays are handled separately by the reconstructor.
            if ( !values.empty() && values.size() < activeCellCount )
            {
                assignValuesToLgrs( cellResults, values );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Aggregate a source result onto each refined cell's parent COARSE cell - the pore-volume-weighted
/// average for intensive quantities (falling back to the bulk cell volume as weight if PORV is not
/// available), or the sum for extensive quantities (e.g. FIP) - then broadcast that aggregate back
/// onto every (active) cell of the parent - both the original flat refined cells and the
/// reconstructed LGR cells. Unrefined cells keep their own value. The result is stored as a
/// GENERATED result named "<sourceName>_COARSE" for all time steps.
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RigNestedHybridGridResultTools::computeCoarseAggregate( RigCaseCellResultsData*        cellResults,
                                                                                const RigEclipseResultAddress& sourceAddress,
                                                                                AggregationMode                mode )
{
    RigEclipseResultAddress invalid;
    if ( !cellResults ) return invalid;

    RigMainGrid*       mainGrid       = cellResults->mainGrid();
    RigActiveCellInfo* activeCellInfo = cellResults->activeCellInfo();
    if ( !mainGrid || !activeCellInfo ) return invalid;

    const std::map<size_t, size_t>& coarseParents = mainGrid->nestedHybridCoarseParents();
    const std::map<size_t, size_t>& sourceCells   = mainGrid->nestedHybridLgrSourceCells();
    if ( coarseParents.empty() ) return invalid;

    if ( !cellResults->ensureKnownResultLoaded( sourceAddress ) ) return invalid;

    // Cell volumes (active-cell indexed): the zero-volume mask that excludes the hidden flat
    // duplicates, and the fallback weight if PORV is not available.
    cellResults->computeCellVolumes();
    RigEclipseResultAddress volAddr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    if ( !cellResults->ensureKnownResultLoaded( volAddr ) ) return invalid;

    const size_t activeCellCount = activeCellInfo->reservoirActiveCellCount();

    const size_t tsCount = cellResults->cellScalarResults( sourceAddress ).size();
    if ( tsCount == 0 ) return invalid;

    // Create the output result (GENERATED so the file reader never tries to read it). createResultEntry()
    // pushes onto the backing storage and may reallocate it, invalidating any reference/pointer into the
    // backing storage; bind volumes/sourceTs/porv only afterwards.
    const QString           outName = sourceAddress.resultName() + "_COARSE";
    RigEclipseResultAddress outAddr( RiaDefines::ResultCatType::GENERATED, outName );
    if ( !cellResults->hasResultEntry( outAddr ) ) cellResults->createResultEntry( outAddr, true );
    std::vector<std::vector<double>>* outTs = cellResults->modifiableCellScalarResultTimesteps( outAddr );
    if ( !outTs ) return invalid;
    outTs->resize( tsCount );

    const std::vector<double>&              volumes  = cellResults->cellScalarResults( volAddr, 0 );
    const std::vector<std::vector<double>>& sourceTs = cellResults->cellScalarResults( sourceAddress );

    // Pore volume (active-cell indexed) is the weight for the average; null if PORV is unavailable.
    std::vector<double>        porvTemp;
    const std::vector<double>* porv = nullptr;
    if ( mode == AggregationMode::PORE_VOLUME_WEIGHTED_AVERAGE )
    {
        porv = RigCaseCellResultsData::getResultIndexableStaticResult( activeCellInfo, cellResults, RiaResultNames::porv(), porvTemp );
    }

    auto activeIndex = [&]( size_t reservoirCell ) { return activeCellInfo->cellResultIndex( ReservoirCellIndex( reservoirCell ) ).value(); };

    // The original flat refined cells of an L2/L3 region are hidden (zero volume) once moved into an
    // LGR, so the real geometry/value lives on the LGR cell. Map each flat cell to the cell that
    // carries its geometry: its LGR copy if it has one, otherwise the flat cell itself (e.g. cells
    // that were left un-nested).
    std::map<size_t, size_t> flatToGeometryCell;
    for ( const auto& [lgrCell, flatCell] : sourceCells )
        flatToGeometryCell[flatCell] = lgrCell;
    auto geometryCell = [&]( size_t flatCell )
    {
        auto it = flatToGeometryCell.find( flatCell );
        return it != flatToGeometryCell.end() ? it->second : flatCell;
    };

    for ( size_t ts = 0; ts < tsCount; ts++ )
    {
        const std::vector<double>& src = sourceTs[ts];
        std::vector<double>&       out = ( *outTs )[ts];
        out                            = src; // unrefined cells keep their own value
        if ( out.size() < activeCellCount ) out.resize( activeCellCount, HUGE_VAL );

        // Accumulate per coarse parent, using the geometry-bearing cell. The zero-bulk-volume filter
        // excludes the hidden flat duplicates in both modes (their PORV is a duplicate too), so no
        // cell is counted twice.
        std::map<size_t, std::pair<double, double>> acc; // parent -> (sum value[*weight], sum weight / count)
        for ( const auto& [flatCell, parent] : coarseParents )
        {
            size_t ri = activeIndex( geometryCell( flatCell ) );
            if ( ri == cvf::UNDEFINED_SIZE_T || ri >= src.size() || ri >= volumes.size() ) continue;
            double v = src[ri];
            if ( volumes[ri] <= 0.0 || v == HUGE_VAL ) continue;
            if ( mode == AggregationMode::SUM )
            {
                auto& a = acc[parent];
                a.first += v;
                a.second += 1.0;
            }
            else
            {
                double w = ( porv && ri < porv->size() ) ? ( *porv )[ri] : volumes[ri];
                if ( w <= 0.0 || w == HUGE_VAL ) continue;
                auto& a = acc[parent];
                a.first += v * w;
                a.second += w;
            }
        }

        auto aggregate = [&]( size_t parent, double fallback )
        {
            auto it = acc.find( parent );
            if ( it != acc.end() && it->second.second > 0.0 )
                return ( mode == AggregationMode::SUM ) ? it->second.first : it->second.first / it->second.second;
            return fallback;
        };

        // Broadcast the parent aggregate onto every (active) cell of the parent - both the flat
        // refined cell and its LGR copy - so the aggregate reads correctly on either representation.
        for ( const auto& [flatCell, parent] : coarseParents )
        {
            const size_t gi       = activeIndex( geometryCell( flatCell ) );
            const double fallback = ( gi != cvf::UNDEFINED_SIZE_T && gi < src.size() ) ? src[gi] : HUGE_VAL;
            const double value    = aggregate( parent, fallback );

            for ( size_t cell : { flatCell, geometryCell( flatCell ) } )
            {
                size_t ri = activeIndex( cell );
                if ( ri != cvf::UNDEFINED_SIZE_T && ri < out.size() ) out[ri] = value;
            }
        }
    }

    return outAddr;
}

//--------------------------------------------------------------------------------------------------
/// Per refinement level, compute the aggregate (pore-volume-weighted average or sum) of a source
/// result over the cells of each immediate parent and broadcast it back onto that level's cells. All
/// other cells are left undefined (blank) so each level's result shows only that level. One result
/// "<sourceName>_COARSE_L<level>" is created per level present (stored on the active refined cells;
/// the parent cells are inactive).
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultAddress> RigNestedHybridGridResultTools::computePerLevelAggregate( RigCaseCellResultsData*        cellResults,
                                                                                               const RigEclipseResultAddress& sourceAddress,
                                                                                               AggregationMode                mode )
{
    std::vector<RigEclipseResultAddress> created;
    if ( !cellResults ) return created;

    RigMainGrid*       mainGrid       = cellResults->mainGrid();
    RigActiveCellInfo* activeCellInfo = cellResults->activeCellInfo();
    if ( !mainGrid || !activeCellInfo ) return created;
    if ( mainGrid->nestedHybridLgrSourceCells().empty() ) return created; // not a reconstructed nested hybrid grid

    if ( !cellResults->ensureKnownResultLoaded( sourceAddress ) ) return created;

    cellResults->computeCellVolumes();
    RigEclipseResultAddress volAddr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    if ( !cellResults->ensureKnownResultLoaded( volAddr ) ) return created;

    const size_t activeCellCount = activeCellInfo->reservoirActiveCellCount();
    const size_t totalCellCount  = mainGrid->totalCellCount();

    // Each cell's refinement level. Prefer the REFINE result (authoritative, full-length per cell) so
    // cells of different levels are never combined; fall back to the LGR name only if REFINE is absent.
    RigEclipseResultAddress    refineAddr( RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RiaDefines::ResultDataType::INTEGER,
                                        RiaResultNames::refine() );
    const std::vector<double>* refine = nullptr;
    if ( cellResults->ensureKnownResultLoaded( refineAddr ) )
    {
        const std::vector<std::vector<double>>& ts = cellResults->cellScalarResults( refineAddr );
        if ( !ts.empty() && ts[0].size() == totalCellCount ) refine = &ts[0];
    }

    auto levelFromName = []( const std::string& name )
    {
        const std::string prefix = "LGR_NHG_L";
        if ( name.rfind( prefix, 0 ) != 0 ) return -1;
        int  value = 0;
        bool any   = false;
        for ( size_t i = prefix.size(); i < name.size() && name[i] >= '0' && name[i] <= '9'; i++ )
        {
            value = value * 10 + ( name[i] - '0' );
            any   = true;
        }
        return any ? value : -1;
    };

    auto activeIndex = [&]( size_t reservoirCell ) { return activeCellInfo->cellResultIndex( ReservoirCellIndex( reservoirCell ) ).value(); };

    // Collect every active reconstructed-LGR cell with its refinement level (from REFINE) and its
    // immediate parent cell (from the LGR hierarchy).
    struct CellRef
    {
        size_t resultIndex;
        int    level;
        size_t parentGlobal;
    };
    std::vector<CellRef> cellRefs;
    for ( size_t gi = 1; gi < mainGrid->gridCount(); gi++ )
    {
        RigGridBase* g   = mainGrid->gridByIndex( gi );
        auto*        lgr = dynamic_cast<RigLocalGrid*>( g );
        if ( !lgr || !lgr->isReconstructedGrid() ) continue;
        const int    nameLevel  = levelFromName( g->gridName() );
        RigGridBase* parentGrid = lgr->parentGrid();
        for ( size_t c = 0; c < g->cellCount(); c++ )
        {
            size_t global = g->reservoirCellIndex( c );
            size_t ri     = activeIndex( global );
            if ( ri == cvf::UNDEFINED_SIZE_T ) continue;
            int level = refine ? (int)std::lround( ( *refine )[global] ) : nameLevel;
            if ( level <= 1 ) continue;
            size_t parentGlobal = parentGrid->reservoirCellIndex( g->cell( c ).parentCellIndex() );
            cellRefs.push_back( { ri, level, parentGlobal } );
        }
    }
    if ( cellRefs.empty() ) return created;

    const size_t tsCount = cellResults->cellScalarResults( sourceAddress ).size();
    if ( tsCount == 0 ) return created;

    // One output result per distinct level. Create every entry first: createResultEntry() pushes onto
    // the backing storage and may reallocate it, which would invalidate any reference/pointer into the
    // backing storage (sourceTs, volumes, previously fetched outTs). Only after all entries exist do we
    // resolve the pointers and source references below.
    std::map<int, RigEclipseResultAddress> outAddrByLevel;
    for ( const CellRef& cr : cellRefs )
    {
        if ( outAddrByLevel.count( cr.level ) ) continue;
        RigEclipseResultAddress outAddr( RiaDefines::ResultCatType::GENERATED,
                                         sourceAddress.resultName() + QString( "_COARSE_L%1" ).arg( cr.level ) );
        if ( !cellResults->hasResultEntry( outAddr ) ) cellResults->createResultEntry( outAddr, true );
        outAddrByLevel.emplace( cr.level, outAddr );
    }

    std::map<int, std::vector<std::vector<double>>*> outByLevel;
    for ( const auto& [level, outAddr] : outAddrByLevel )
    {
        std::vector<std::vector<double>>* outTs = cellResults->modifiableCellScalarResultTimesteps( outAddr );
        if ( !outTs ) continue;
        outTs->resize( tsCount );
        outByLevel[level] = outTs;
        created.push_back( outAddr );
    }

    // Safe to bind now that no further entries will be created.
    const std::vector<std::vector<double>>& sourceTs = cellResults->cellScalarResults( sourceAddress );
    const std::vector<double>&              volumes  = cellResults->cellScalarResults( volAddr, 0 );

    // Pore volume (active-cell indexed) is the weight for the average; null if PORV is unavailable.
    std::vector<double>        porvTemp;
    const std::vector<double>* porv = nullptr;
    if ( mode == AggregationMode::PORE_VOLUME_WEIGHTED_AVERAGE )
    {
        porv = RigCaseCellResultsData::getResultIndexableStaticResult( activeCellInfo, cellResults, RiaResultNames::porv(), porvTemp );
    }

    for ( size_t ts = 0; ts < tsCount; ts++ )
    {
        const std::vector<double>& src = sourceTs[ts];

        // Accumulation keyed by (level, immediate parent) - cells of different levels are never
        // accumulated together. The zero-bulk-volume filter excludes hidden duplicates in both modes.
        std::map<int, std::map<size_t, std::pair<double, double>>> acc;
        for ( const CellRef& cr : cellRefs )
        {
            if ( cr.resultIndex >= src.size() || cr.resultIndex >= volumes.size() ) continue;
            double v = src[cr.resultIndex];
            if ( volumes[cr.resultIndex] <= 0.0 || v == HUGE_VAL ) continue;
            if ( mode == AggregationMode::SUM )
            {
                auto& a = acc[cr.level][cr.parentGlobal];
                a.first += v;
                a.second += 1.0;
            }
            else
            {
                double w = ( porv && cr.resultIndex < porv->size() ) ? ( *porv )[cr.resultIndex] : volumes[cr.resultIndex];
                if ( w <= 0.0 || w == HUGE_VAL ) continue;
                auto& a = acc[cr.level][cr.parentGlobal];
                a.first += v * w;
                a.second += w;
            }
        }

        for ( const auto& [level, outTs] : outByLevel )
        {
            std::vector<double>& out = ( *outTs )[ts];
            out.assign( activeCellCount, HUGE_VAL ); // blank everywhere except this level's own cells

            const std::map<size_t, std::pair<double, double>>& accLevel = acc[level];
            for ( const CellRef& cr : cellRefs )
            {
                if ( cr.level != level || cr.resultIndex >= out.size() ) continue;
                auto it = accLevel.find( cr.parentGlobal );
                if ( it != accLevel.end() && it->second.second > 0.0 )
                {
                    out[cr.resultIndex] = ( mode == AggregationMode::SUM ) ? it->second.first : it->second.first / it->second.second;
                }
            }
        }
    }

    return created;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNestedHybridGridResultTools::assignValuesToLgrs( RigCaseCellResultsData* cellResults, std::vector<double>& values )
{
    if ( !cellResults ) return;

    RigMainGrid*       mainGrid       = cellResults->mainGrid();
    RigActiveCellInfo* activeCellInfo = cellResults->activeCellInfo();
    if ( !mainGrid || !activeCellInfo ) return;

    const std::map<size_t, size_t>& sourceCells = mainGrid->nestedHybridLgrSourceCells();
    if ( sourceCells.empty() || values.empty() ) return;

    const size_t totalCellCount = mainGrid->totalCellCount();
    if ( values.size() >= totalCellCount ) return; // full-length array already covering the LGR cells

    // Full-length (all-cells) array loaded after the reconstruction: the file array covers only the
    // original flat cells (the LGR cells are appended at the end of the grid), so it is indexed by
    // global reservoir cell index, not by active-cell result index. Extend it the same way
    // RigNestedHybridGridReconstructor::extendFullLengthResults() extends the already-loaded ones.
    // The original flat cell count is the main grid's own cell count (the LGR cells all live in the
    // appended local grids, including filler cells without a source mapping).
    const size_t origCellCount = mainGrid->cellCount();
    if ( values.size() == origCellCount )
    {
        values.resize( totalCellCount, std::numeric_limits<double>::infinity() );
        for ( const auto& [lgrReservoirCellIndex, flatReservoirCellIndex] : sourceCells )
        {
            values[lgrReservoirCellIndex] = values[flatReservoirCellIndex];
        }
        return;
    }

    const size_t activeCellCount = activeCellInfo->reservoirActiveCellCount();
    if ( values.size() < activeCellCount )
    {
        values.resize( activeCellCount, std::numeric_limits<double>::infinity() );
    }

    for ( const auto& [lgrReservoirCellIndex, flatReservoirCellIndex] : sourceCells )
    {
        size_t lgrResultIndex  = activeCellInfo->cellResultIndex( ReservoirCellIndex( lgrReservoirCellIndex ) ).value();
        size_t flatResultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( flatReservoirCellIndex ) ).value();

        if ( lgrResultIndex != cvf::UNDEFINED_SIZE_T && flatResultIndex != cvf::UNDEFINED_SIZE_T && lgrResultIndex < values.size() &&
             flatResultIndex < values.size() )
        {
            values[lgrResultIndex] = values[flatResultIndex];
        }
    }
}
