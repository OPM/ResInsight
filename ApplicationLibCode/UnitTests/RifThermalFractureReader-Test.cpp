#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifThermalFractureReader.h"
#include "RigThermalFractureDefinition.h"

static const QString CASE_REAL_TEST_DATA_DIRECTORY = QString( "%1/RifThermalFractureReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifThermalFractureReaderTest, LoadFile )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY + "fracture_metric.csv";

    auto [fractureData, errorMessage] = RifThermalFractureReader::readFractureCsvFile( fileName );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( fractureData.get() );

    EXPECT_EQ( "frac01", fractureData->name().toStdString() );

    EXPECT_EQ( fractureData->numNodes(), 57u );

    EXPECT_EQ( fractureData->numTimeSteps(), 29u );

    auto properties = fractureData->getPropertyNamesUnits();
    EXPECT_EQ( properties.size(), 19u );
    EXPECT_EQ( properties[0].first.toStdString(), "XCoord" );
    EXPECT_EQ( properties[0].second.toStdString(), "m" );

    // The location of the center node is the same for all timesteps
    double centerNodeX = 459352.0;
    double centerNodeY = -7.32599e+06;
    double centerNodeZ = 2735.0;

    int nodeIndex = 0;
    for ( size_t timeStepIndex = 0; timeStepIndex < fractureData->numTimeSteps(); timeStepIndex++ )
    {
        EXPECT_DOUBLE_EQ( centerNodeX, fractureData->getPropertyValue( 0, nodeIndex, static_cast<int>( timeStepIndex ) ) );
        EXPECT_DOUBLE_EQ( centerNodeY, fractureData->getPropertyValue( 1, nodeIndex, static_cast<int>( timeStepIndex ) ) );
        EXPECT_DOUBLE_EQ( centerNodeZ, fractureData->getPropertyValue( 2, nodeIndex, static_cast<int>( timeStepIndex ) ) );
    }
}
