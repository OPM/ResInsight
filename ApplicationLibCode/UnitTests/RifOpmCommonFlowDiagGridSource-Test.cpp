/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RiaTestDataDirectory.h"

#include "RifOpmCommonFlowDiagGridSource.h"

#include "opm/utility/ECLGraph.hpp"
#include "opm/utility/ECLResultData.hpp"

#include <filesystem>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build the flow graph with both resdata and opm-common, and require identical topology
//--------------------------------------------------------------------------------------------------
void compareGraphs( const std::filesystem::path& gridFile )
{
    auto initFile = gridFile;
    initFile.replace_extension( ".INIT" );

    if ( !std::filesystem::exists( gridFile ) || !std::filesystem::exists( initFile ) )
    {
        GTEST_SKIP() << "Test file not found: " << gridFile;
    }

    const Opm::ECLInitFileData init( initFile.string() );

    const auto resdataGraph = Opm::ECLGraph::load( gridFile.string(), init );

    const RifOpmCommonFlowDiagGridSource opmSource( gridFile.string() );
    const auto                           opmGraph = Opm::ECLGraph::load( opmSource, init );

    EXPECT_EQ( resdataGraph.numGrids(), opmGraph.numGrids() );
    EXPECT_EQ( resdataGraph.activeGrids(), opmGraph.activeGrids() );
    EXPECT_EQ( resdataGraph.numCells(), opmGraph.numCells() );
    EXPECT_EQ( resdataGraph.numConnections(), opmGraph.numConnections() );
    EXPECT_EQ( resdataGraph.neighbours(), opmGraph.neighbours() );
    EXPECT_EQ( resdataGraph.poreVolume(), opmGraph.poreVolume() );
    EXPECT_EQ( resdataGraph.transmissibility(), opmGraph.transmissibility() );
    EXPECT_EQ( resdataGraph.localCellID(), opmGraph.localCellID() );

    auto restartFile = gridFile;
    restartFile.replace_extension( ".UNRST" );
    if ( !std::filesystem::exists( restartFile ) ) return;

    const Opm::ECLRestartData restart( restartFile.string() );
    if ( !restart.selectReportStep( 1 ) ) return;

    for ( const auto phase : resdataGraph.activePhases() )
    {
        EXPECT_EQ( resdataGraph.flux( restart, phase ), opmGraph.flux( restart, phase ) );
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmCommonFlowDiagGridSource, SameGraphAsResdataWithLgrAndNnc )
{
    const auto gridFile = std::filesystem::path( TEST_MODEL_DIR ) / "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID";
    if ( !std::filesystem::exists( gridFile ) ) GTEST_SKIP() << "Test file not found: " << gridFile;

    // Make sure the model exercises LGR and NNC handling
    const RifOpmCommonFlowDiagGridSource opmSource( gridFile.string() );
    EXPECT_GT( opmSource.numGrids(), 1 );
    EXPECT_FALSE( opmSource.nncs().empty() );

    compareGraphs( gridFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifOpmCommonFlowDiagGridSource, SameGraphAsResdata )
{
    compareGraphs( std::filesystem::path( TEST_MODEL_DIR ) / "flow_diagnostics_test/SIMPLE_SUMMARY2.EGRID" );
}
