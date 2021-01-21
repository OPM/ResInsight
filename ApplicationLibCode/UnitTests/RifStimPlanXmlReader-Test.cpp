#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifStimPlanXmlReader.h"
#include "RigStimPlanFractureDefinition.h"

static const QString CASE_REAL_TEST_DATA_DIRECTORY = QString( "%1/RifStimPlanXmlReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifStimPlanXmlReaderTest, LoadFile )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY + "small_fracture.xml";

    double                           conductivityScaleFactor = 1.0;
    double                           halfLengthScaleFactor   = 1.0;
    double                           heightScaleFactor       = 1.0;
    double                           wellPathDepthAtFracture = 100.0;
    RiaDefines::EclipseUnitSystem    unit                    = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    QString                          errorMessage;
    RifStimPlanXmlReader::MirrorMode mode = RifStimPlanXmlReader::MIRROR_AUTO;

    cvf::ref<RigStimPlanFractureDefinition> fractureData;

    fractureData = RifStimPlanXmlReader::readStimPlanXMLFile( fileName,
                                                              conductivityScaleFactor,
                                                              halfLengthScaleFactor,
                                                              heightScaleFactor,
                                                              -wellPathDepthAtFracture,
                                                              mode,
                                                              unit,
                                                              &errorMessage );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( fractureData.notNull() );

    size_t xSamplesIncludingMirrorValues = 7;
    EXPECT_EQ( xSamplesIncludingMirrorValues, fractureData->xCount() );
    EXPECT_EQ( size_t( 5 ), fractureData->yCount() );
    EXPECT_EQ( size_t( 1 ), fractureData->timeSteps().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifStimPlanXmlReaderTest, LoadFileNewFormat )
{
    QString fileName = CASE_REAL_TEST_DATA_DIRECTORY + "contour_Metric.xml";

    double                           conductivityScaleFactor = 1.0;
    double                           halfLengthScaleFactor   = 1.0;
    double                           heightScaleFactor       = 1.0;
    double                           wellPathDepthAtFracture = 100.0;
    RiaDefines::EclipseUnitSystem    unit                    = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    QString                          errorMessage;
    RifStimPlanXmlReader::MirrorMode mode = RifStimPlanXmlReader::MIRROR_AUTO;

    cvf::ref<RigStimPlanFractureDefinition> fractureData;

    fractureData = RifStimPlanXmlReader::readStimPlanXMLFile( fileName,
                                                              conductivityScaleFactor,
                                                              halfLengthScaleFactor,
                                                              heightScaleFactor,
                                                              -wellPathDepthAtFracture,
                                                              mode,
                                                              unit,
                                                              &errorMessage );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( fractureData.notNull() );

    size_t xSamplesIncludingMirrorValues = 49;
    EXPECT_EQ( xSamplesIncludingMirrorValues, fractureData->xCount() );
    EXPECT_EQ( size_t( 23 ), fractureData->yCount() );
    EXPECT_EQ( size_t( 1 ), fractureData->timeSteps().size() );

    EXPECT_DOUBLE_EQ( 2773.680, fractureData->topPerfTvd() );
    EXPECT_DOUBLE_EQ( 2773.680, fractureData->bottomPerfTvd() );
    EXPECT_DOUBLE_EQ( 2804.160, fractureData->topPerfMd() );
    EXPECT_DOUBLE_EQ( 2804.770, fractureData->bottomPerfMd() );
}
