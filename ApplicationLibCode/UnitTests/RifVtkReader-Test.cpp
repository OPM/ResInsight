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
