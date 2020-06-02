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
}
