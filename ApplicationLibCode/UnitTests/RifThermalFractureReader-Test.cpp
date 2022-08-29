#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifThermalFractureReader.h"
#include "RigThermalFractureDefinition.h"

#include <QFile>
#include <QTextStream>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_03 = QString( "%1/RifThermalFractureReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifThermalFractureReaderTest, LoadFile )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_03 + "fracture_metric.csv";

    auto [fractureData, errorMessage] = RifThermalFractureReader::readFractureCsvFile( fileName );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( fractureData.get() );

    EXPECT_EQ( "frac01", fractureData->name().toStdString() );

    EXPECT_EQ( fractureData->numNodes(), 81u );

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

    {
        // Sample from center node: LeakoffPressureDrop from last time step
        double expectedValue = 18.8747;
        int    propertyIndex = 18;
        int    nodeIndex     = 0;
        int    timeStepIndex = 28;
        EXPECT_DOUBLE_EQ( expectedValue, fractureData->getPropertyValue( propertyIndex, nodeIndex, timeStepIndex ) );
    }

    {
        // Sample from internal node: EffectiveFracStress from tenth time step
        double expectedValue = 7.72785;
        int    propertyIndex = 17;
        int    nodeIndex     = 2;
        int    timeStepIndex = 10;
        EXPECT_DOUBLE_EQ( expectedValue, fractureData->getPropertyValue( propertyIndex, nodeIndex, timeStepIndex ) );
    }

    {
        // Sample from bottom node: EffectiveResStress from fifth time step
        double expectedValue = 28.5565;
        int    propertyIndex = 16;
        int    nodeIndex     = 57;
        int    timeStepIndex = 6;
        EXPECT_DOUBLE_EQ( expectedValue, fractureData->getPropertyValue( propertyIndex, nodeIndex, timeStepIndex ) );
    }

    {
        // Sample from perimeter node: ResTemperature from eight time step
        double expectedValue = 10.3882;
        int    propertyIndex = 13;
        int    nodeIndex     = 58;
        int    timeStepIndex = 7;
        EXPECT_DOUBLE_EQ( expectedValue, fractureData->getPropertyValue( propertyIndex, nodeIndex, timeStepIndex ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifThermalFractureReaderTest, LoadFileMixedUnits )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_03 + "fracture_mixed_units.csv";

    auto [fractureData, errorMessage] = RifThermalFractureReader::readFractureCsvFile( fileName );

    EXPECT_FALSE( errorMessage.isEmpty() );
    EXPECT_FALSE( fractureData.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifThermalFractureReaderTest, LoadFileNonExistingFiles )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_03 + "this_file_does_not_exist.csv";

    auto [fractureData, errorMessage] = RifThermalFractureReader::readFractureCsvFile( fileName );

    EXPECT_FALSE( errorMessage.isEmpty() );
    EXPECT_FALSE( fractureData.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RifThermalFractureReaderTest, CreateXyzPointCloud )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY_03 + "fracture_OS_metric_units_final.csv";

    auto [fractureData, errorMessage] = RifThermalFractureReader::readFractureCsvFile( fileName );

    auto numNodes     = fractureData->numNodes();
    auto numTimeSteps = fractureData->numTimeSteps();
    auto properties   = fractureData->getPropertyNamesUnits();

    for ( size_t timeStepIndex = 0; timeStepIndex < numTimeSteps; timeStepIndex++ )
    {
        QString exportfileName = QString( CASE_REAL_TEST_DATA_DIRECTORY_03 + "msjtest-%1.xyz" ).arg( timeStepIndex );

        std::vector<double> xs;
        std::vector<double> ys;
        std::vector<double> zs;

        for ( int nodeIndex = 0; nodeIndex < static_cast<int>( numNodes ); nodeIndex++ )
        {
            xs.push_back(
                fractureData->getPropertyValue( 0, static_cast<int>( nodeIndex ), static_cast<int>( timeStepIndex ) ) );
            ys.push_back(
                fractureData->getPropertyValue( 1, static_cast<int>( nodeIndex ), static_cast<int>( timeStepIndex ) ) );
            zs.push_back(
                fractureData->getPropertyValue( 2, static_cast<int>( nodeIndex ), static_cast<int>( timeStepIndex ) ) );
        }

        QFile file( exportfileName );
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            QTextStream out( &file );

            out.setRealNumberPrecision( 16 );

            for ( size_t i = 0; i < xs.size(); i++ )
            {
                out << xs[i] << " " << ys[i] << " " << zs[i] << "\n";
            }
        }
        file.close();
    }
}
