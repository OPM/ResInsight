#include "gtest/gtest.h"

#include "RifRoffFileTools.h"

#include "RiaDefines.h"
#include "RiaTestDataDirectory.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "Reader.hpp"

#include "cvfBoundingBox.h"
#include "cvfObject.h"

#include <QDir>
#include <QFile>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsBasic )
{
    // 3 zones spanning 6 K-layers total. ROFF k-order is top→bottom: zone 1 at top,
    // zone 3 at bottom. After the K-flip, reservoir K=0 is the top of the file.
    const std::vector<int> nLayers = { 2, 1, 3 };
    const size_t           nx      = 2;
    const size_t           ny      = 3;
    const size_t           nz      = 6;

    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( nLayers, nx, ny, nz );
    ASSERT_EQ( nx * ny * nz, values.size() );

    // Expected zone per reservoir K (after flipping the ROFF top-down ordering):
    //   ROFF k=0,1 → zone 1 → reservoir K=5,4
    //   ROFF k=2   → zone 2 → reservoir K=3
    //   ROFF k=3,4,5 → zone 3 → reservoir K=2,1,0
    const std::vector<int> expectedZoneByReservoirK = { 3, 3, 3, 2, 1, 1 };

    for ( size_t k = 0; k < nz; ++k )
    {
        for ( size_t j = 0; j < ny; ++j )
        {
            for ( size_t i = 0; i < nx; ++i )
            {
                const size_t idx = i + nx * j + nx * ny * k;
                EXPECT_EQ( static_cast<double>( expectedZoneByReservoirK[k] ), values[idx] );
            }
        }
    }
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsSumMismatch )
{
    // Sum of nLayers (2+1+2 = 5) does not match nz (6) -> empty result.
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( { 2, 1, 2 }, 1, 1, 6 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsNonPositiveLayer )
{
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( { 3, 0, 3 }, 1, 1, 6 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ComputeZoneValuesFromSubgridsRejectsEmptyInput )
{
    const auto values = RifRoffFileTools::computeZoneValuesFromSubgrids( {}, 1, 1, 0 );
    EXPECT_TRUE( values.empty() );
}

TEST( RifRoffFileTools, ReadSubgridsFromRoffFile )
{
    QDir    baseFolder( TEST_DATA_DIR );
    QString filePath = baseFolder.absoluteFilePath( "RifRoffReader/with_subgrids.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    std::ifstream stream( filePath.toStdString(), std::ios::binary );
    ASSERT_TRUE( stream.good() );

    roff::Reader reader( stream );
    reader.parse();

    ASSERT_EQ( 3u, reader.getArrayLength( "subgrids.nLayers" ) );

    const std::vector<int> nLayers = reader.getIntArray( "subgrids.nLayers" );
    ASSERT_EQ( 3u, nLayers.size() );
    EXPECT_EQ( 2, nLayers[0] );
    EXPECT_EQ( 1, nLayers[1] );
    EXPECT_EQ( 3, nLayers[2] );

    ASSERT_EQ( 3u, reader.getArrayLength( "subgrids.names" ) );
    const std::vector<std::string> names = reader.getStringArray( "subgrids.names" );
    ASSERT_EQ( 3u, names.size() );
    EXPECT_EQ( "Top Zone", names[0] );
    EXPECT_EQ( "Middle Zone", names[1] );
    EXPECT_EQ( "Bottom Zone", names[2] );
}

//--------------------------------------------------------------------------------------------------
// computeActiveCellMatrixIndex renumbers the active cells in place: every non-zero entry is replaced
// by its running active index, every zero entry by -1. The return value is the number of active cells.
//--------------------------------------------------------------------------------------------------

TEST( RifRoffFileTools, ComputeActiveCellMatrixIndexMixed )
{
    std::vector<int> activeCells = { 1, 0, 1, 1, 0 };

    const size_t activeCount = RifRoffFileTools::computeActiveCellMatrixIndex( activeCells );

    EXPECT_EQ( 3u, activeCount );
    EXPECT_EQ( std::vector<int>( { 0, -1, 1, 2, -1 } ), activeCells );
}

TEST( RifRoffFileTools, ComputeActiveCellMatrixIndexAllActive )
{
    std::vector<int> activeCells = { 1, 1, 1 };

    EXPECT_EQ( 3u, RifRoffFileTools::computeActiveCellMatrixIndex( activeCells ) );
    EXPECT_EQ( std::vector<int>( { 0, 1, 2 } ), activeCells );
}

TEST( RifRoffFileTools, ComputeActiveCellMatrixIndexAllInactive )
{
    std::vector<int> activeCells = { 0, 0, 0 };

    EXPECT_EQ( 0u, RifRoffFileTools::computeActiveCellMatrixIndex( activeCells ) );
    EXPECT_EQ( std::vector<int>( { -1, -1, -1 } ), activeCells );
}

TEST( RifRoffFileTools, ComputeActiveCellMatrixIndexEmpty )
{
    std::vector<int> activeCells;

    EXPECT_EQ( 0u, RifRoffFileTools::computeActiveCellMatrixIndex( activeCells ) );
    EXPECT_TRUE( activeCells.empty() );
}

TEST( RifRoffFileTools, ComputeActiveCellMatrixIndexTreatsAnyNonZeroAsActive )
{
    // The roff active array is a byte flag, but the check is != 0, so any non-zero value counts.
    std::vector<int> activeCells = { 5, 0, -3 };

    EXPECT_EQ( 2u, RifRoffFileTools::computeActiveCellMatrixIndex( activeCells ) );
    EXPECT_EQ( std::vector<int>( { 0, -1, 1 } ), activeCells );
}

//--------------------------------------------------------------------------------------------------
// hasGridData reports whether the file carries an actual grid, by looking for the cornerLines data.
//--------------------------------------------------------------------------------------------------

TEST( RifRoffFileTools, HasGridDataTrueForGridFile )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/simple_grid.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    EXPECT_TRUE( RifRoffFileTools::hasGridData( filePath ) );
}

TEST( RifRoffFileTools, HasGridDataFalseForParameterOnlyFile )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/facies_info.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    EXPECT_FALSE( RifRoffFileTools::hasGridData( filePath ) );
}

TEST( RifRoffFileTools, HasGridDataFalseForMissingFile )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/this_file_does_not_exist.roff" );
    ASSERT_FALSE( QFile::exists( filePath ) );

    EXPECT_FALSE( RifRoffFileTools::hasGridData( filePath ) );
}

//--------------------------------------------------------------------------------------------------
// openGridFile on a stream is the entry point used when the roff data is not a file on disk, e.g. a
// blob downloaded from Sumo.
//--------------------------------------------------------------------------------------------------

TEST( RifRoffFileTools, OpenGridFileFromStream )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/simple_grid.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    std::ifstream stream( filePath.toStdString(), std::ios::binary );
    ASSERT_TRUE( stream.good() );

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    QString errorMessages;
    ASSERT_TRUE( RifRoffFileTools::openGridFile( stream, caseData.p(), &errorMessages ) );
    EXPECT_TRUE( errorMessages.isEmpty() );

    auto* mainGrid = caseData->mainGrid();
    ASSERT_TRUE( mainGrid != nullptr );

    EXPECT_EQ( 2u, mainGrid->cellCountI() );
    EXPECT_EQ( 2u, mainGrid->cellCountJ() );
    EXPECT_EQ( 2u, mainGrid->cellCountK() );
    EXPECT_EQ( 8u, mainGrid->cellCount() );

    // The fixture marks two of the eight cells inactive.
    auto* activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    ASSERT_TRUE( activeCellInfo != nullptr );
    EXPECT_EQ( 6u, activeCellInfo->reservoirActiveCellCount() );

    // The pillars are vertical and 100 apart, and the translate tag offsets x by 1000 and y by 2000.
    // Depths are negated when the corners are built, so z runs from -100 up to 0. The extent is taken
    // from the nodes rather than from RigMainGrid::boundingBox(), which is only filled in by
    // computeCachedData() - openGridFile leaves that to its caller.
    ASSERT_EQ( 8u * 8u, mainGrid->nodes().size() );

    cvf::BoundingBox bb;
    for ( const auto& node : mainGrid->nodes() )
    {
        bb.add( node );
    }

    EXPECT_NEAR( 1000.0, bb.min().x(), 1e-6 );
    EXPECT_NEAR( 1200.0, bb.max().x(), 1e-6 );
    EXPECT_NEAR( 2000.0, bb.min().y(), 1e-6 );
    EXPECT_NEAR( 2200.0, bb.max().y(), 1e-6 );
    EXPECT_NEAR( -100.0, bb.min().z(), 1e-6 );
    EXPECT_NEAR( 0.0, bb.max().z(), 1e-6 );
}

TEST( RifRoffFileTools, OpenGridFileFromStreamFailsWithoutGridData )
{
    // A property-only roff has no dimensions/translate/scale/cornerLines, so the import must fail
    // cleanly and report why rather than leaving a half-built grid behind.
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/two_properties.roff" );
    ASSERT_TRUE( QFile::exists( filePath ) );

    std::ifstream stream( filePath.toStdString(), std::ios::binary );
    ASSERT_TRUE( stream.good() );

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );

    QString errorMessages;
    EXPECT_FALSE( RifRoffFileTools::openGridFile( stream, caseData.p(), &errorMessages ) );
    EXPECT_FALSE( errorMessages.isEmpty() );
}

//--------------------------------------------------------------------------------------------------
// propertyValuesFromStream reads one property out of an in-memory roff blob. Among the arrays whose
// length matches the grid cell count it prefers the one whose keyword matches the requested name
// (case-insensitively), and it masks inactive cells with HUGE_VAL.
//--------------------------------------------------------------------------------------------------

namespace
{
// Open the 2x2x2 fixture grid, so a property blob has something to be read against.
cvf::ref<RigEclipseCaseData> loadFixtureGrid()
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/simple_grid.roff" );
    if ( !QFile::exists( filePath ) ) return nullptr;

    std::ifstream stream( filePath.toStdString(), std::ios::binary );
    if ( !stream.good() ) return nullptr;

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );
    if ( !RifRoffFileTools::openGridFile( stream, caseData.p(), nullptr ) ) return nullptr;

    return caseData;
}

// The values of the cells the fixture marks active, with the inactive ones dropped. Both the property
// array and the active array are reordered the same way, so the pairing survives and this can be
// compared without depending on the roff-to-reservoir index mapping.
std::vector<double> finiteValuesSorted( const std::vector<double>& values )
{
    std::vector<double> finite;
    for ( double v : values )
    {
        if ( v != HUGE_VAL ) finite.push_back( v );
    }
    std::sort( finite.begin(), finite.end() );
    return finite;
}

std::vector<double> readProperty( const QString& propertyName, RigEclipseCaseData* caseData, bool* ok )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/two_properties.roff" );

    std::ifstream       stream( filePath.toStdString(), std::ios::binary );
    std::vector<double> values;
    *ok = RifRoffFileTools::propertyValuesFromStream( stream, caseData, propertyName, &values );
    return values;
}
} // namespace

TEST( RifRoffFileTools, PropertyValuesFromStreamSelectsRequestedProperty )
{
    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    bool ok     = false;
    auto values = readProperty( "SOIL", caseData.p(), &ok );

    ASSERT_TRUE( ok );
    ASSERT_EQ( 8u, values.size() );

    // The two inactive cells are masked, the remaining six carry the SOIL values, not the SWAT ones.
    EXPECT_EQ( std::vector<double>( { 110.0, 120.0, 130.0, 140.0, 150.0, 160.0 } ), finiteValuesSorted( values ) );
    EXPECT_EQ( 2, std::count( values.begin(), values.end(), HUGE_VAL ) );
}

TEST( RifRoffFileTools, PropertyValuesFromStreamSelectsTheOtherProperty )
{
    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    bool ok     = false;
    auto values = readProperty( "SWAT", caseData.p(), &ok );

    ASSERT_TRUE( ok );
    EXPECT_EQ( std::vector<double>( { 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 } ), finiteValuesSorted( values ) );
}

TEST( RifRoffFileTools, PropertyValuesFromStreamMatchesNameCaseInsensitively )
{
    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    bool ok     = false;
    auto values = readProperty( "soil", caseData.p(), &ok );

    ASSERT_TRUE( ok );
    EXPECT_EQ( std::vector<double>( { 110.0, 120.0, 130.0, 140.0, 150.0, 160.0 } ), finiteValuesSorted( values ) );
}

TEST( RifRoffFileTools, PropertyValuesFromStreamFallsBackToFirstCandidate )
{
    // No array matches the requested name, so the first array of the right length is used. This is the
    // documented fallback, and it keeps a blob whose keyword does not match the result name readable.
    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    bool ok     = false;
    auto values = readProperty( "NO_SUCH_PROPERTY", caseData.p(), &ok );

    ASSERT_TRUE( ok );
    EXPECT_EQ( std::vector<double>( { 10.0, 20.0, 30.0, 40.0, 50.0, 60.0 } ), finiteValuesSorted( values ) );
}

TEST( RifRoffFileTools, PropertyValuesFromStreamRejectsMissingArguments )
{
    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/two_properties.roff" );

    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    std::vector<double> values;

    {
        std::ifstream stream( filePath.toStdString(), std::ios::binary );
        EXPECT_FALSE( RifRoffFileTools::propertyValuesFromStream( stream, nullptr, "SOIL", &values ) );
    }
    {
        std::ifstream stream( filePath.toStdString(), std::ios::binary );
        EXPECT_FALSE( RifRoffFileTools::propertyValuesFromStream( stream, caseData.p(), "SOIL", nullptr ) );
    }
}

TEST( RifRoffFileTools, PropertyValuesFromStreamRejectsMismatchedCellCount )
{
    // facies_info.roff declares a 46x112x242 grid and carries only 4 values, so nothing matches the
    // cell count of the 2x2x2 fixture grid.
    cvf::ref<RigEclipseCaseData> caseData = loadFixtureGrid();
    ASSERT_TRUE( caseData.notNull() );

    QDir baseFolder( TEST_DATA_DIR );
    auto filePath = baseFolder.absoluteFilePath( "RifRoffReader/facies_info.roff" );

    std::ifstream       stream( filePath.toStdString(), std::ios::binary );
    std::vector<double> values;
    EXPECT_FALSE( RifRoffFileTools::propertyValuesFromStream( stream, caseData.p(), "composite", &values ) );
}
