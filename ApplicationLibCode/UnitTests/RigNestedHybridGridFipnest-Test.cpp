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

#include "gtest/gtest.h"

#include "RiaStringEncodingTools.h"
#include "RiaTestDataDirectory.h"

#include "RifEclipseKeywordContent.h"
#include "RifEclipseTextFileReader.h"
#include "RifReaderEclipseOutput.h"

#include "RigEclipseCaseData.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"
#include "RigNestedHybridGridFipnestCodec.h"
#include "RigNestedHybridGridReconstructor.h"
#include "RigNestedHybridGridResultTools.h"

#include "RimEclipseResultCase.h"

#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <set>

//==================================================================================================
// Tests for the FIPNEST/FIPSLOT/REFINE parent-child encoding of a nested hybrid grid (#14510):
// the codec that derives the arrays from the OLDIJK/TMP sidecar description and back, the GRDECL
// sidecar auto-export, and the sidecar-free import path that reads the arrays from the INIT file.
// The requirement backing most of these tests: a grid reconstructed through FIPNEST must equal the
// grid reconstructed through the sidecars.
//==================================================================================================

namespace
{
const size_t NX = 150, NY = 84, NZ = 96;

QString nestedHybridModelDir()
{
    QDir baseFolder( TEST_MODEL_DIR );
    baseFolder.cd( "NestedHybridGrid" );
    return baseFolder.absolutePath();
}

// Read all integer keywords of a GRDECL file in a single pass.
std::map<QString, std::vector<int>> readIntKeywords( const QString& filePath )
{
    std::map<QString, std::vector<int>> result;
    auto                                content = RifEclipseTextFileReader::readKeywordAndValues( filePath.toStdString() );
    for ( const auto& kw : content )
    {
        std::vector<int> values;
        values.reserve( kw.values.size() );
        for ( float v : kw.values )
            values.push_back( (int)std::lround( v ) );
        result[QString::fromStdString( kw.keyword )] = std::move( values );
    }
    return result;
}

RigNestedHybridGridReconstructor::NestedHybridInput buildSidecarInput( const QString& dir )
{
    RigNestedHybridGridReconstructor::NestedHybridInput input;
    auto                                                refine = readIntKeywords( dir + "/DROGON_NESTED_REFINE.grdecl" );
    auto                                                oldIjk = readIntKeywords( dir + "/DROGON_NESTED_OLDIJK.grdecl" );
    input.refine                                               = refine["REFINE"];
    input.oldI                                                 = oldIjk["OLDI"];
    input.oldJ                                                 = oldIjk["OLDJ"];
    input.oldK                                                 = oldIjk["OLDK"];
    input.tmpI                                                 = oldIjk["TMPI"];
    input.tmpJ                                                 = oldIjk["TMPJ"];
    input.tmpK                                                 = oldIjk["TMPK"];
    return input;
}

// A freshly opened (not yet reconstructed) DROGON_NESTED case.
struct OpenedCase
{
    std::unique_ptr<RimEclipseResultCase> resultCase;
    cvf::ref<RigEclipseCaseData>          caseData;
    cvf::ref<RifReaderEclipseOutput>      reader;
};

bool openCase( const QString& gridFile, OpenedCase& openedCase )
{
    openedCase.resultCase = std::make_unique<RimEclipseResultCase>();
    openedCase.caseData   = new RigEclipseCaseData( openedCase.resultCase.get() );
    openedCase.reader     = new RifReaderEclipseOutput;
    if ( !openedCase.reader->open( gridFile, openedCase.caseData.p() ) ) return false;
    openedCase.caseData->mainGrid()->computeCachedData();
    return true;
}

// Compare the reconstructed LGR structure of two cases: same grids (name, dimensions, cell range,
// parent), the same LGR-cell -> source-flat-cell mapping, and coarse-parent maps that partition the
// refined cells identically (the numeric coarse indices may legitimately differ, see the codec).
void expectEqualReconstruction( RigMainGrid* a, RigMainGrid* b )
{
    ASSERT_EQ( a->gridCount(), b->gridCount() );
    ASSERT_EQ( a->totalCellCount(), b->totalCellCount() );

    for ( size_t i = 1; i < a->gridCount(); i++ )
    {
        RigLocalGrid* lgrA = dynamic_cast<RigLocalGrid*>( a->gridByIndex( i ) );
        RigLocalGrid* lgrB = dynamic_cast<RigLocalGrid*>( b->gridByIndex( i ) );
        ASSERT_TRUE( lgrA != nullptr && lgrB != nullptr );
        EXPECT_EQ( lgrA->gridName(), lgrB->gridName() );
        EXPECT_EQ( lgrA->cellCountI(), lgrB->cellCountI() );
        EXPECT_EQ( lgrA->cellCountJ(), lgrB->cellCountJ() );
        EXPECT_EQ( lgrA->cellCountK(), lgrB->cellCountK() );
        EXPECT_EQ( lgrA->reservoirCellIndex( 0 ), lgrB->reservoirCellIndex( 0 ) );
        EXPECT_EQ( lgrA->parentGrid()->gridName(), lgrB->parentGrid()->gridName() );
    }

    EXPECT_EQ( a->nestedHybridLgrSourceCells(), b->nestedHybridLgrSourceCells() );

    // Coarse-parent maps: same key set, and the values partition the keys identically.
    const std::map<size_t, size_t>& cpA = a->nestedHybridCoarseParents();
    const std::map<size_t, size_t>& cpB = b->nestedHybridCoarseParents();
    ASSERT_EQ( cpA.size(), cpB.size() );
    std::map<size_t, size_t> valueMap; // A-parent-value -> B-parent-value
    for ( const auto& [flatCell, parentA] : cpA )
    {
        auto itB = cpB.find( flatCell );
        ASSERT_TRUE( itB != cpB.end() ) << "coarse-parent key sets differ at flat cell " << flatCell;
        auto [mapped, inserted] = valueMap.try_emplace( parentA, itB->second );
        EXPECT_EQ( mapped->second, itB->second ) << "coarse-parent partitions differ at flat cell " << flatCell;
    }
    std::set<size_t> mappedBValues;
    for ( const auto& [va, vb] : valueMap )
        mappedBValues.insert( vb );
    EXPECT_EQ( mappedBValues.size(), valueMap.size() ) << "coarse-parent value mapping is not a bijection";
}

// Append one INTE keyword to an existing unformatted Eclipse output file (e.g. an INIT file).
bool appendIntKeywordToEclipseFile( const QString& filePath, const char* keyword, const std::vector<int>& values )
{
    fortio_type* fortio = fortio_open_append( RiaStringEncodingTools::toNativeEncoded( filePath ).data(), false, ECL_ENDIAN_FLIP );
    if ( !fortio ) return false;

    ecl_kw_type* kw = ecl_kw_alloc_new( keyword, (int)values.size(), ECL_INT, values.data() );
    const bool   ok = ecl_kw_fwrite( kw, fortio );
    ecl_kw_free( kw );
    fortio_fclose( fortio );
    return ok;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Shared fixture: the sidecar input arrays and the codec-derived FIPNEST/FIPSLOT arrays are
/// computed once (array-only work, no grid).
//--------------------------------------------------------------------------------------------------
class RigNestedHybridGridFipnestTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        const QString dir = nestedHybridModelDir();
        if ( !QFile::exists( dir + "/DROGON_NESTED.EGRID" ) )
        {
            s_setupError = "Missing test model: " + dir + "/DROGON_NESTED.EGRID";
            return;
        }

        s_input = buildSidecarInput( dir );
        if ( s_input.refine.size() != NX * NY * NZ )
        {
            s_setupError = "Unexpected sidecar size";
            return;
        }

        s_arrays  = RigNestedHybridGridFipnestCodec::computeParentChildArrays( s_input, NX, NY, NZ );
        s_setupOk = true;
    }

    static void TearDownTestSuite()
    {
        s_input   = {};
        s_arrays  = {};
        s_setupOk = false;
    }

    void SetUp() override { ASSERT_TRUE( s_setupOk ) << s_setupError.toStdString(); }

    static RigNestedHybridGridReconstructor::NestedHybridInput s_input;
    static RigNestedHybridGridFipnestCodec::ParentChildArrays  s_arrays;
    static bool                                                s_setupOk;
    static QString                                             s_setupError;
};

RigNestedHybridGridReconstructor::NestedHybridInput RigNestedHybridGridFipnestTest::s_input;
RigNestedHybridGridFipnestCodec::ParentChildArrays  RigNestedHybridGridFipnestTest::s_arrays;
bool                                                RigNestedHybridGridFipnestTest::s_setupOk = false;
QString                                             RigNestedHybridGridFipnestTest::s_setupError;

//--------------------------------------------------------------------------------------------------
/// Synthetic round-trip with a K refinement factor of 2 where the refined hosts stop below the top
/// coarse-K layer. The reconstructor re-derives the K factor from the maximum OLDK of its input, so
/// the decoder must emit OLD for the unrefined cells too - otherwise the factor comes out as 4 here
/// and every parent lands on the wrong K layer.
//--------------------------------------------------------------------------------------------------
TEST( RigNestedHybridGridFipnestCodecTest, SyntheticKFactorRoundTrip )
{
    // Flat 8x1x8 grid; coarse 4x1x4 embedded at (i, 0, k*2); one level-2 region refining coarse
    // cells (1..2, 1, 1..2) by (2,1,2), its cells appended as a band at i 4..7, k 0..3.
    const size_t nx = 8, ny = 1, nz = 8;

    RigNestedHybridGridReconstructor::NestedHybridInput input;
    input.refine.assign( nx * ny * nz, 1 );
    input.oldI.assign( nx * ny * nz, 1 );
    input.oldJ.assign( nx * ny * nz, 1 );
    input.oldK.assign( nx * ny * nz, 1 );
    input.tmpI.assign( nx * ny * nz, 0 );
    input.tmpJ.assign( nx * ny * nz, 0 );
    input.tmpK.assign( nx * ny * nz, 0 );

    for ( size_t k = 0; k < nz; k++ )
        for ( size_t i = 0; i < 4; i++ )
        {
            const size_t f = i + k * nx * ny;
            input.oldI[f]  = (int)i + 1;
            input.oldK[f]  = (int)k / 2 + 1;
        }

    for ( int tk = 0; tk < 4; tk++ )
        for ( int ti = 0; ti < 4; ti++ )
        {
            const size_t f  = (size_t)( ti + 4 ) + (size_t)tk * nx * ny;
            input.refine[f] = 2;
            input.oldI[f]   = ti / 2 + 1;
            input.oldK[f]   = tk / 2 + 1;
            input.tmpI[f]   = ti;
            input.tmpK[f]   = tk;
        }

    const auto arrays = RigNestedHybridGridFipnestCodec::computeParentChildArrays( input, nx, ny, nz );
    ASSERT_EQ( arrays.fipnest.size(), nx * ny * nz );
    EXPECT_EQ( arrays.unresolvedRefinedCells, 0u );

    QString    warnings;
    const auto translated =
        RigNestedHybridGridFipnestCodec::buildInputFromParentChildArrays( arrays.fipnest, arrays.fipslot, input.refine, nx, ny, nz, &warnings );
    EXPECT_TRUE( warnings.isEmpty() ) << warnings.toStdString();
    ASSERT_EQ( translated.refine.size(), nx * ny * nz );

    // The coarse host (OLD) of every refined cell must be recovered exactly, and the K factor the
    // reconstructor re-derives (NZ / max OLDK) must match the true factor of 2.
    int maxOldK = 0;
    for ( size_t f = 0; f < nx * ny * nz; f++ )
    {
        maxOldK = std::max( maxOldK, translated.oldK[f] );
        if ( input.refine[f] <= 1 ) continue;
        EXPECT_EQ( translated.oldI[f], input.oldI[f] );
        EXPECT_EQ( translated.oldJ[f], input.oldJ[f] );
        EXPECT_EQ( translated.oldK[f], input.oldK[f] );
    }
    ASSERT_GT( maxOldK, 0 );
    ASSERT_EQ( nz % (size_t)maxOldK, 0u );
    EXPECT_EQ( nz / (size_t)maxOldK, 2u );

    // Relative TMP must be reproduced cellwise (the reconstructor is shift-invariant).
    int minOrig[3] = { INT_MAX, INT_MAX, INT_MAX }, minTrans[3] = { INT_MAX, INT_MAX, INT_MAX };
    for ( size_t f = 0; f < nx * ny * nz; f++ )
    {
        if ( input.refine[f] <= 1 ) continue;
        const int orig[3]  = { input.tmpI[f], input.tmpJ[f], input.tmpK[f] };
        const int trans[3] = { translated.tmpI[f], translated.tmpJ[f], translated.tmpK[f] };
        for ( int a = 0; a < 3; a++ )
        {
            minOrig[a]  = std::min( minOrig[a], orig[a] );
            minTrans[a] = std::min( minTrans[a], trans[a] );
        }
    }
    for ( size_t f = 0; f < nx * ny * nz; f++ )
    {
        if ( input.refine[f] <= 1 ) continue;
        EXPECT_EQ( input.tmpI[f] - minOrig[0], translated.tmpI[f] - minTrans[0] );
        EXPECT_EQ( input.tmpJ[f] - minOrig[1], translated.tmpJ[f] - minTrans[1] );
        EXPECT_EQ( input.tmpK[f] - minOrig[2], translated.tmpK[f] - minTrans[2] );
    }
}

//--------------------------------------------------------------------------------------------------
/// T2: structural invariants of the encoded arrays on the DROGON_NESTED dataset.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, ArrayInvariants )
{
    const size_t cellCount = NX * NY * NZ;
    ASSERT_EQ( s_arrays.fipnest.size(), cellCount );
    ASSERT_EQ( s_arrays.fipslot.size(), cellCount );
    EXPECT_EQ( s_arrays.unresolvedRefinedCells, 0u );

    std::map<int, size_t> refinedByLevel;
    for ( size_t f = 0; f < cellCount; f++ )
        if ( s_input.refine[f] > 1 ) refinedByLevel[s_input.refine[f]]++;
    EXPECT_EQ( refinedByLevel[2], 4768u );
    EXPECT_EQ( refinedByLevel[3], 74708u );
    EXPECT_EQ( refinedByLevel[4], 5984u );

    size_t                virtualSlots = 0;
    std::map<int, size_t> level4ChildrenByParent;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        const int link = s_arrays.fipnest[f];
        if ( s_input.refine[f] > 1 )
        {
            // Every refined cell has a parent link and a slot, and its chain ends at an unrefined
            // cell within a few steps.
            ASSERT_GT( link, 0 ) << "refined cell " << f << " has no parent link";
            ASSERT_GT( s_arrays.fipslot[f], 0 );

            size_t cur   = f;
            int    steps = 0;
            while ( s_arrays.fipnest[cur] != 0 )
            {
                ASSERT_LE( ++steps, 4 );
                cur = (size_t)s_arrays.fipnest[cur] - 1;
                ASSERT_LT( cur, cellCount );
            }
            EXPECT_EQ( s_input.refine[cur], 1 );

            if ( s_input.refine[f] == 4 ) level4ChildrenByParent[link]++;
        }
        else if ( link != 0 )
        {
            // An unrefined cell with a link is a refined-away host slot: it must point straight at
            // an unrefined coarse cell with no further link.
            virtualSlots++;
            EXPECT_EQ( s_input.refine[(size_t)link - 1], 1 );
            EXPECT_EQ( s_arrays.fipnest[(size_t)link - 1], 0 );
            EXPECT_GT( s_arrays.fipslot[f], 0 );
        }
    }

    // The 5984 level-4 cells nest in 748 level-3 host slots, 8 children (2x2x2) each; every one of
    // those hosts was refined away and is represented by a virtual slot.
    EXPECT_EQ( level4ChildrenByParent.size(), 748u );
    for ( const auto& [parent, count] : level4ChildrenByParent )
        EXPECT_EQ( count, 8u );
    EXPECT_EQ( virtualSlots, 748u );
}

//--------------------------------------------------------------------------------------------------
/// T1: the translator reproduces the sidecar input from the encoded arrays - REFINE and OLD exactly,
/// TMP up to the per-level per-axis constant shift the reconstructor is invariant to.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, CodecRoundTripReproducesSidecarInput )
{
    QString    warnings;
    const auto translated =
        RigNestedHybridGridFipnestCodec::buildInputFromParentChildArrays( s_arrays.fipnest, s_arrays.fipslot, s_input.refine, NX, NY, NZ, &warnings );
    EXPECT_TRUE( warnings.isEmpty() ) << warnings.toStdString();

    const size_t cellCount = NX * NY * NZ;
    ASSERT_EQ( translated.refine.size(), cellCount );
    EXPECT_EQ( translated.refine, s_input.refine );

    // OLD (the coarse host) must match exactly on every refined cell.
    size_t oldMismatches = 0;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( s_input.refine[f] <= 1 ) continue;
        if ( translated.oldI[f] != s_input.oldI[f] || translated.oldJ[f] != s_input.oldJ[f] || translated.oldK[f] != s_input.oldK[f] )
            oldMismatches++;
    }
    EXPECT_EQ( oldMismatches, 0u );

    // TMP: per level and axis, the level-relative coordinates must match cellwise.
    std::map<int, std::array<int, 6>> tmpMins; // level -> min original tmp[3], min translated tmp[3]
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( s_input.refine[f] <= 1 ) continue;
        const int level    = s_input.refine[f];
        auto      it       = tmpMins.find( level );
        const int orig[3]  = { s_input.tmpI[f], s_input.tmpJ[f], s_input.tmpK[f] };
        const int trans[3] = { translated.tmpI[f], translated.tmpJ[f], translated.tmpK[f] };
        if ( it == tmpMins.end() )
        {
            tmpMins[level] = { orig[0], orig[1], orig[2], trans[0], trans[1], trans[2] };
        }
        else
        {
            for ( int a = 0; a < 3; a++ )
            {
                it->second[a]     = std::min( it->second[a], orig[a] );
                it->second[3 + a] = std::min( it->second[3 + a], trans[a] );
            }
        }
    }

    size_t tmpMismatches = 0;
    for ( size_t f = 0; f < cellCount; f++ )
    {
        if ( s_input.refine[f] <= 1 ) continue;
        const auto& mins     = tmpMins[s_input.refine[f]];
        const int   orig[3]  = { s_input.tmpI[f], s_input.tmpJ[f], s_input.tmpK[f] };
        const int   trans[3] = { translated.tmpI[f], translated.tmpJ[f], translated.tmpK[f] };
        for ( int a = 0; a < 3; a++ )
            if ( orig[a] - mins[a] != trans[a] - mins[3 + a] ) tmpMismatches++;
    }
    EXPECT_EQ( tmpMismatches, 0u );
}

//--------------------------------------------------------------------------------------------------
/// T4: the GRDECL writer round-trips through the standard text file reader, including run-length
/// encoded stretches and large values.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, GrdeclWriterRoundTrip )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    const QString path = tempDir.filePath( "roundtrip_FIPNEST.grdecl" );

    std::vector<int> a = { 0, 0, 0, 0, 0, 7, 7, 7, 7, 1209600, 3, 3, 1, 2 };
    std::vector<int> b( 1000, 42 );
    b[999] = -5;

    const std::vector<std::pair<QString, const std::vector<int>*>> keywords = { { "FIPNEST", &a }, { "FIPSLOT", &b } };
    ASSERT_TRUE( RigNestedHybridGridResultTools::writeIntKeywordsToGrdeclFile( path, keywords ) );

    auto readBack = readIntKeywords( path );
    EXPECT_EQ( readBack["FIPNEST"], a );
    EXPECT_EQ( readBack["FIPSLOT"], b );
}

//--------------------------------------------------------------------------------------------------
/// T5: the stock DROGON_NESTED INIT file has no FIPNEST, so the INIT-based path must not trigger.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, StockInitFileHasNoFipnest )
{
    const QString gridFile = nestedHybridModelDir() + "/DROGON_NESTED.EGRID";
    EXPECT_FALSE( RigNestedHybridGridResultTools::initFileHasFipnest( gridFile ) );
}

//--------------------------------------------------------------------------------------------------
/// T3: a grid reconstructed from the translated FIPNEST arrays equals the grid reconstructed from
/// the sidecar input.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, ReconstructionFromFipnestEqualsSidecarReconstruction )
{
    const QString gridFile = nestedHybridModelDir() + "/DROGON_NESTED.EGRID";

    OpenedCase sidecarCase;
    ASSERT_TRUE( openCase( gridFile, sidecarCase ) );
    QString err;
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( sidecarCase.caseData.p(), s_input, &err ) ) << err.toStdString();

    QString    warnings;
    const auto translated =
        RigNestedHybridGridFipnestCodec::buildInputFromParentChildArrays( s_arrays.fipnest, s_arrays.fipslot, s_input.refine, NX, NY, NZ, &warnings );

    OpenedCase fipnestCase;
    ASSERT_TRUE( openCase( gridFile, fipnestCase ) );
    ASSERT_TRUE( RigNestedHybridGridReconstructor::reconstruct( fipnestCase.caseData.p(), translated, &err ) ) << err.toStdString();

    expectEqualReconstruction( sidecarCase.caseData->mainGrid(), fipnestCase.caseData->mainGrid() );
}

//--------------------------------------------------------------------------------------------------
/// T6: hermetic end-to-end pass through the real files. Phase 1 stages the model with sidecars in a
/// temp dir and verifies the sidecar-based reconstruction auto-exports the FIPNEST GRDECL. Phase 2
/// stages EGRID + INIT only (no sidecars) in a second dir, inserts the FIPNEST/FIPSLOT/REFINE INTE
/// arrays into the INIT copy, imports through the INIT-based path and compares against the
/// sidecar-based reconstruction.
//--------------------------------------------------------------------------------------------------
TEST_F( RigNestedHybridGridFipnestTest, EndToEndInitFileImport )
{
    const QString dir = nestedHybridModelDir();

    // Phase 1: sidecar import in a staged dir -> auto-exported FIPNEST sidecar.
    QTemporaryDir stagedSidecarDir;
    ASSERT_TRUE( stagedSidecarDir.isValid() );
    for ( const QString& f : { QString( "DROGON_NESTED.EGRID" ),
                               QString( "DROGON_NESTED.INIT" ),
                               QString( "DROGON_NESTED_REFINE.grdecl" ),
                               QString( "DROGON_NESTED_OLDIJK.grdecl" ) } )
    {
        ASSERT_TRUE( QFile::copy( dir + "/" + f, stagedSidecarDir.filePath( f ) ) );
    }
    const QString stagedGridFile = stagedSidecarDir.filePath( "DROGON_NESTED.EGRID" );

    OpenedCase sidecarCase;
    ASSERT_TRUE( openCase( stagedGridFile, sidecarCase ) );
    RigNestedHybridGridResultTools::reconstructNestedHybridGridIfPresent( stagedGridFile, sidecarCase.caseData.p() );
    ASSERT_GT( sidecarCase.caseData->mainGrid()->gridCount(), 1u );

    const QString exportedFipnest = RigNestedHybridGridResultTools::fipnestSidecarFilePath( stagedGridFile );
    ASSERT_FALSE( exportedFipnest.isEmpty() ) << "sidecar import did not auto-export the FIPNEST GRDECL";

    auto exportedArrays = readIntKeywords( exportedFipnest );
    ASSERT_EQ( exportedArrays["FIPNEST"].size(), NX * NY * NZ );
    ASSERT_EQ( exportedArrays["FIPSLOT"].size(), NX * NY * NZ );
    ASSERT_EQ( exportedArrays["REFINE"].size(), NX * NY * NZ );

    // Phase 2: EGRID + augmented INIT only - no sidecars anywhere near the grid file.
    QTemporaryDir stagedInitDir;
    ASSERT_TRUE( stagedInitDir.isValid() );
    ASSERT_TRUE( QFile::copy( dir + "/DROGON_NESTED.EGRID", stagedInitDir.filePath( "DROGON_NESTED.EGRID" ) ) );
    ASSERT_TRUE( QFile::copy( dir + "/DROGON_NESTED.INIT", stagedInitDir.filePath( "DROGON_NESTED.INIT" ) ) );

    const QString augmentedInit   = stagedInitDir.filePath( "DROGON_NESTED.INIT" );
    const QString fipnestGridFile = stagedInitDir.filePath( "DROGON_NESTED.EGRID" );
    ASSERT_TRUE( appendIntKeywordToEclipseFile( augmentedInit, "FIPNEST", exportedArrays["FIPNEST"] ) );
    ASSERT_TRUE( appendIntKeywordToEclipseFile( augmentedInit, "FIPSLOT", exportedArrays["FIPSLOT"] ) );
    ASSERT_TRUE( appendIntKeywordToEclipseFile( augmentedInit, "REFINE", exportedArrays["REFINE"] ) );

    EXPECT_TRUE( RigNestedHybridGridResultTools::initFileHasFipnest( fipnestGridFile ) );

    OpenedCase fipnestCase;
    ASSERT_TRUE( openCase( fipnestGridFile, fipnestCase ) );
    ASSERT_TRUE( RigNestedHybridGridResultTools::reconstructNestedHybridGridFromInitFile( fipnestGridFile, fipnestCase.caseData.p() ) );

    expectEqualReconstruction( sidecarCase.caseData->mainGrid(), fipnestCase.caseData->mainGrid() );
}
