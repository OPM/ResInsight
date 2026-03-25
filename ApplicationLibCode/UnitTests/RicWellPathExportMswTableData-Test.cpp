/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaApplication.h"
#include "RiaTestDataDirectory.h"

#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswDataFormatter.h"
#include "CompletionsMsw/RigMswTableData.h"
#include "CompletionsMsw/RigMswTableRows.h"

#include "RifTextDataTableFormatter.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

namespace
{

//--------------------------------------------------------------------------------------------------
/// Write MSW table data for one well to a file, creating parent directories as needed.
//--------------------------------------------------------------------------------------------------
void writeMswTableDataToFile( const RigMswTableData& tableData, const QString& filePath )
{
    QFileInfo fileInfo( filePath );
    QDir().mkpath( fileInfo.absolutePath() );

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;

    QTextStream               stream( &file );
    RifTextDataTableFormatter formatter( stream );
    RigMswDataFormatter::formatMswTables( formatter, tableData );
}

//--------------------------------------------------------------------------------------------------
/// Write COMPSEGL (LGR) table data to a separate file.
//--------------------------------------------------------------------------------------------------
void writeMswLgrTableDataToFile( const RigMswTableData& tableData, const QString& filePath )
{
    if ( !tableData.hasLgrData() ) return;

    QFileInfo fileInfo( filePath );
    QDir().mkpath( fileInfo.absolutePath() );

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;

    QTextStream               stream( &file );
    RifTextDataTableFormatter formatter( stream );
    RigMswDataFormatter::formatCompsegsTable( formatter, tableData, true );
}

//--------------------------------------------------------------------------------------------------
/// Extract {i, j, k, gridName} tuples from COMPSEGS data, sorted for stable comparison.
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<size_t, size_t, size_t, std::string>> extractSortedCells( const RigMswTableData& data )
{
    std::vector<std::tuple<size_t, size_t, size_t, std::string>> cells;
    for ( const auto& row : data.compsegsData() )
    {
        cells.emplace_back( row.i, row.j, row.k, row.gridName );
    }
    std::sort( cells.begin(), cells.end() );
    return cells;
}

} // anonymous namespace

//==================================================================================================
//
// Parameterized integration tests: Tree mode vs Geometry mode for all MSW project files.
//
// The test loads a ResInsight project file, extracts well MSW data using both the tree-based
// (extractSingleWellMswDataTree) and geometry-based (extractSingleWellMswDataGeometry)
// algorithms, and verifies that both produce equivalent results for every well path:
//
//   - The same set of reservoir cells in COMPSEGS (sorted {i,j,k,gridName} tuples)
//   - The same number of WSEGVALV, WSEGAICD, and WSEGSICD valve rows
//
//==================================================================================================

class MswTreeVsGeometryTest : public testing::TestWithParam<std::string>
{
};

TEST_P( MswTreeVsGeometryTest, CompareTreeAndGeometryModes )
{
    const std::string& projectFileName = GetParam();
    QString            projectFilePath =
        QString( "%1/msw-export/project-files/%2" ).arg( TEST_MODEL_DIR ).arg( QString::fromStdString( projectFileName ) );

    // Strip extension for use as folder name, e.g. "base.rsp" -> "base"
    const QString projectStem    = QFileInfo( QString::fromStdString( projectFileName ) ).completeBaseName();
    const QString compareOutBase = QString( "%1/msw-export/compare-output/%2" ).arg( TEST_MODEL_DIR ).arg( projectStem );

    if ( !QFile::exists( projectFilePath ) )
    {
        GTEST_SKIP() << "Project file not found: " << projectFilePath.toStdString();
    }

    bool loaded = RiaApplication::instance()->loadProject( projectFilePath );
    ASSERT_TRUE( loaded ) << "Failed to load project: " << projectFilePath.toStdString();

    RimProject* project = RiaApplication::instance()->project();
    ASSERT_NE( project, nullptr );

    auto eclipseCases = project->eclipseCases();
    ASSERT_FALSE( eclipseCases.empty() ) << "No eclipse cases found in project";

    RimEclipseCase* eclipseCase = eclipseCases[0];
    ASSERT_NE( eclipseCase, nullptr );

    if ( eclipseCase->eclipseCaseData() == nullptr )
    {
        GTEST_SKIP() << "Eclipse case data not loaded — EGRID file may be unavailable";
    }

    auto wellPaths = project->allWellPaths();
    ASSERT_FALSE( wellPaths.empty() ) << "No well paths found in project";

    int wellsWithData = 0;

    for ( auto* wellPath : wellPaths )
    {
        ASSERT_NE( wellPath, nullptr );

        if ( !wellPath->isTopLevelWellPath() ) continue;

        auto treeResult     = RicWellPathExportMswTableData::extractSingleWellMswDataTree( eclipseCase, wellPath );
        auto geometryResult = RicWellPathExportMswTableData::extractSingleWellMswDataGeometry( eclipseCase, wellPath );

        // If one mode fails, the other should fail too (no MSW data for this well path).
        if ( !treeResult.has_value() && !geometryResult.has_value() )
        {
            continue;
        }

        ASSERT_TRUE( treeResult.has_value() ) << "Tree mode failed for well '" << wellPath->name().toStdString()
                                              << "': " << treeResult.error();
        ASSERT_TRUE( geometryResult.has_value() )
            << "Geometry mode failed for well '" << wellPath->name().toStdString() << "': " << geometryResult.error();

        const std::string wellName     = treeResult->wellName();
        const QString     wellFileName = QString::fromStdString( wellName ) + ".txt";

        // Export both modes to files for side-by-side comparison
        writeMswTableDataToFile( *treeResult, compareOutBase + "/tree/" + wellFileName );
        writeMswTableDataToFile( *geometryResult, compareOutBase + "/geometry/" + wellFileName );

        // Export LGR (COMPSEGL) data to separate files if present
        const QString lgrFileName = QString::fromStdString( wellName ) + "_LGR.txt";
        writeMswLgrTableDataToFile( *treeResult, compareOutBase + "/tree/" + lgrFileName );
        writeMswLgrTableDataToFile( *geometryResult, compareOutBase + "/geometry/" + lgrFileName );

        // Both modes must produce data for the same well.
        EXPECT_EQ( wellName, geometryResult->wellName() ) << "Well name mismatch for: " << wellPath->name().toStdString();

        // Both modes must connect to the same set of reservoir cells.
        auto treeCells     = extractSortedCells( *treeResult );
        auto geometryCells = extractSortedCells( *geometryResult );

        EXPECT_EQ( treeCells, geometryCells ) << "COMPSEGS cells differ between Tree and Geometry modes for well '" << wellName << "'";

        // Both modes must produce the same number of valve rows for each valve type.
        EXPECT_EQ( treeResult->wsegvalvData().size(), geometryResult->wsegvalvData().size() )
            << "WSEGVALV row count differs for well '" << wellName << "'";

        EXPECT_EQ( treeResult->wsegaicdData().size(), geometryResult->wsegaicdData().size() )
            << "WSEGAICD row count differs for well '" << wellName << "'";

        EXPECT_EQ( treeResult->wsegsicdData().size(), geometryResult->wsegsicdData().size() )
            << "WSEGSICD row count differs for well '" << wellName << "'";

        ++wellsWithData;
    }

    EXPECT_GT( wellsWithData, 0 ) << "No well paths produced MSW data — check project file and well path MSW parameters";
}

INSTANTIATE_TEST_SUITE_P( MswExportProjectFiles,
                          MswTreeVsGeometryTest,
                          testing::Values( "base.rsp",
                                           "fracture.rsp",
                                           "perf_lateral.rsp",
                                           "perf-lgr.rsp",
                                           "perf_valve.rsp",
                                           "two_wells.rsp",
                                           "perf-lgr-two-wells.rsp",
                                           "perf_aicd.rsp",
                                           "fishbones.rsp",
                                           "multiple_laterals.rsp",
                                           "tie-in-custom-valve-location.rsp",
                                           "standalone-valve.rsp" ) );
