#include "RifVtkReader.h"

#include "RiaTestDataDirectory.h"

#include "RigFemPartCollection.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

static std::filesystem::path testDataDir()
{
    return std::filesystem::path( TEST_DATA_DIR ) / "RifVtkReader";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, OpenValidFile )
{
    RifVtkReader reader;
    EXPECT_FALSE( reader.isOpen() );

    std::string errorMessage;
    bool        opened = reader.openFile( ( testDataDir() / "model.pvd" ).string(), &errorMessage );
    EXPECT_TRUE( opened );
    EXPECT_TRUE( reader.isOpen() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, OpenInvalidFile )
{
    RifVtkReader reader;

    std::string errorMessage;
    bool        opened = reader.openFile( ( testDataDir() / "nonexistent.pvd" ).string(), &errorMessage );
    EXPECT_FALSE( opened );
    EXPECT_FALSE( reader.isOpen() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, StepNames )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    auto stepNames = reader.allStepNames();
    ASSERT_EQ( stepNames.size(), 2u );

    EXPECT_EQ( reader.frameCount( 0 ), 1 );
    EXPECT_EQ( reader.frameCount( 1 ), 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ReadFemParts )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    bool                           result   = reader.readFemParts( femParts.p() );
    ASSERT_TRUE( result );

    ASSERT_EQ( femParts->partCount(), 1 );

    const RigFemPart* part = femParts->part( 0 );
    EXPECT_EQ( part->nodeCount(), 12 );
    EXPECT_EQ( part->elementCount(), 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ReadDisplacements )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    // Step 0: all displacements are zero
    {
        std::vector<cvf::Vec3f> displacements;
        reader.readDisplacements( 0, 0, 0, &displacements );
        ASSERT_EQ( displacements.size(), 12u );
        for ( const auto& d : displacements )
        {
            EXPECT_FLOAT_EQ( d.x(), 0.0f );
            EXPECT_FLOAT_EQ( d.y(), 0.0f );
            EXPECT_FLOAT_EQ( d.z(), 0.0f );
        }
    }

    // Step 1: displacements are (0.1, 0.2, 0.3) in file, z is negated on read
    {
        std::vector<cvf::Vec3f> displacements;
        reader.readDisplacements( 0, 1, 0, &displacements );
        ASSERT_EQ( displacements.size(), 12u );
        for ( const auto& d : displacements )
        {
            EXPECT_NEAR( d.x(), 0.1f, 1e-5f );
            EXPECT_NEAR( d.y(), 0.2f, 1e-5f );
            EXPECT_NEAR( d.z(), -0.3f, 1e-5f );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ScalarFieldNames )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    auto nodeFields = reader.scalarNodeFieldAndComponentNames();
    ASSERT_TRUE( nodeFields.count( "U" ) > 0 );
    auto& uComponents = nodeFields["U"];
    EXPECT_EQ( uComponents.size(), 3u );
    EXPECT_EQ( uComponents[0], "U1" );
    EXPECT_EQ( uComponents[1], "U2" );
    EXPECT_EQ( uComponents[2], "U3" );

    auto elementFields = reader.scalarElementFieldAndComponentNames();
    EXPECT_TRUE( elementFields.count( "Stress" ) > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ReadNodeFieldDisplacement )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    std::vector<float>               u1( 12 ), u2( 12 ), u3( 12 );
    std::vector<float>*              components[3] = { &u1, &u2, &u3 };
    std::vector<std::vector<float>*> resultValues( components, components + 3 );

    reader.readNodeField( "U", 0, 1, 0, &resultValues );

    ASSERT_EQ( u1.size(), 12u );
    for ( size_t i = 0; i < 12; i++ )
    {
        EXPECT_NEAR( u1[i], 0.1f, 1e-5f );
        EXPECT_NEAR( u2[i], 0.2f, 1e-5f );
        EXPECT_NEAR( u3[i], -0.3f, 1e-5f );
    }
}

//--------------------------------------------------------------------------------------------------
/// stress[0..2] are the rows of a symmetric tensor; xz and yz change sign with the negated z
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ReadElementFieldStressTensorRows )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    auto elementFields = reader.scalarElementFieldAndComponentNames();
    ASSERT_TRUE( elementFields.count( "S_VTK" ) > 0 );
    const std::vector<std::string> expectedComponents = { "S11", "S22", "S33", "S12", "S13", "S23" };
    EXPECT_EQ( elementFields["S_VTK"], expectedComponents );

    std::vector<std::vector<float>>  values( 6 );
    std::vector<std::vector<float>*> resultValues;
    for ( auto& v : values )
        resultValues.push_back( &v );

    reader.readElementField( "S_VTK", 0, 1, 0, &resultValues );

    const std::vector<float> expectedCell0 = { 11, 22, 33, 12, -13, -23 };
    const std::vector<float> expectedCell1 = { 111, 122, 133, 112, -113, -123 };
    for ( size_t c = 0; c < 6; c++ )
    {
        ASSERT_EQ( values[c].size(), 2u );
        EXPECT_FLOAT_EQ( values[c][0], expectedCell0[c] );
        EXPECT_FLOAT_EQ( values[c][1], expectedCell1[c] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Point data "pressure" is POR; S and E are the cell values at every element corner, S = POR * I - stress
//--------------------------------------------------------------------------------------------------
TEST( RifVtkReaderTest, ReadElementNodalStressFromCellsAndPor )
{
    RifVtkReader reader;
    reader.openFile( ( testDataDir() / "model.pvd" ).string(), nullptr );

    cvf::ref<RigFemPartCollection> femParts = new RigFemPartCollection();
    reader.readFemParts( femParts.p() );

    EXPECT_TRUE( reader.populateDerivedResultNames() );
    EXPECT_TRUE( reader.scalarNodeFieldAndComponentNames().count( "POR" ) > 0 );
    EXPECT_TRUE( reader.scalarElementNodeFieldAndComponentNames().count( "S" ) > 0 );
    EXPECT_TRUE( reader.scalarElementNodeFieldAndComponentNames().count( "E" ) > 0 );

    std::vector<float>               por;
    std::vector<std::vector<float>*> porValues = { &por };
    reader.readNodeField( "POR", 0, 1, 0, &porValues );
    ASSERT_EQ( por.size(), 12u );
    EXPECT_FLOAT_EQ( por[5], 50.0f );

    std::vector<std::vector<float>>  s( 6 ), e( 6 );
    std::vector<std::vector<float>*> sValues, eValues;
    for ( int c = 0; c < 6; c++ )
    {
        sValues.push_back( &s[c] );
        eValues.push_back( &e[c] );
    }
    reader.readElementNodeField( "S", 0, 1, 0, &sValues );
    reader.readElementNodeField( "E", 0, 1, 0, &eValues );

    // Element 1, local node 2 is node 5 (POR 50). Cell stress rows (111 112 113), (112 122 123), (113 123 133);
    // z is negated, so xz and yz change sign.
    const size_t             idx       = 8 + 2;
    const std::vector<float> expectedS = { 50 - 111, 50 - 122, 50 - 133, -112, 113, 123 };
    const std::vector<float> expectedE = { 1.1f, 1.4f, 1.6f, 1.2f, -1.3f, -1.5f };
    for ( int c = 0; c < 6; c++ )
    {
        ASSERT_EQ( s[c].size(), 16u );
        EXPECT_FLOAT_EQ( s[c][idx], expectedS[c] );
        EXPECT_FLOAT_EQ( e[c][idx], expectedE[c] );
    }
}
