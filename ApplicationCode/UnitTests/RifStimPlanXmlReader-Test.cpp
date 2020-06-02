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
    RiaEclipseUnitTools::UnitSystem  unit                    = RiaEclipseUnitTools::UnitSystem::UNITS_METRIC;
    QString                          errorMessage;
    RifStimPlanXmlReader::MirrorMode mode = RifStimPlanXmlReader::MIRROR_AUTO;

    cvf::ref<RigStimPlanFractureDefinition> m_stimPlanFractureDefinitionData;

    m_stimPlanFractureDefinitionData = RifStimPlanXmlReader::readStimPlanXMLFile( fileName,
                                                                                  conductivityScaleFactor,
                                                                                  halfLengthScaleFactor,
                                                                                  heightScaleFactor,
                                                                                  -wellPathDepthAtFracture,
                                                                                  mode,
                                                                                  unit,
                                                                                  &errorMessage );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( m_stimPlanFractureDefinitionData.notNull() );

    size_t xSamplesIncludingMirrorValues = 7;
    EXPECT_EQ( xSamplesIncludingMirrorValues, m_stimPlanFractureDefinitionData->xCount() );
    EXPECT_EQ( 5, m_stimPlanFractureDefinitionData->yCount() );
    EXPECT_EQ( 1, m_stimPlanFractureDefinitionData->timeSteps().size() );
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
    RiaEclipseUnitTools::UnitSystem  unit                    = RiaEclipseUnitTools::UnitSystem::UNITS_METRIC;
    QString                          errorMessage;
    RifStimPlanXmlReader::MirrorMode mode = RifStimPlanXmlReader::MIRROR_AUTO;

    cvf::ref<RigStimPlanFractureDefinition> m_stimPlanFractureDefinitionData;

    m_stimPlanFractureDefinitionData = RifStimPlanXmlReader::readStimPlanXMLFile( fileName,
                                                                                  conductivityScaleFactor,
                                                                                  halfLengthScaleFactor,
                                                                                  heightScaleFactor,
                                                                                  -wellPathDepthAtFracture,
                                                                                  mode,
                                                                                  unit,
                                                                                  &errorMessage );

    EXPECT_TRUE( errorMessage.isEmpty() );
    EXPECT_TRUE( m_stimPlanFractureDefinitionData.notNull() );

    size_t xSamplesIncludingMirrorValues = 49;
    EXPECT_EQ( xSamplesIncludingMirrorValues, m_stimPlanFractureDefinitionData->xCount() );
    EXPECT_EQ( 23, m_stimPlanFractureDefinitionData->yCount() );
    EXPECT_EQ( 1, m_stimPlanFractureDefinitionData->timeSteps().size() );

    EXPECT_DOUBLE_EQ( 2773.680, m_stimPlanFractureDefinitionData->topPerfTvd() );
    EXPECT_DOUBLE_EQ( 2773.680, m_stimPlanFractureDefinitionData->bottomPerfTvd() );
    EXPECT_DOUBLE_EQ( 2804.160, m_stimPlanFractureDefinitionData->topPerfMd() );
    EXPECT_DOUBLE_EQ( 2804.770, m_stimPlanFractureDefinitionData->bottomPerfMd() );
}
