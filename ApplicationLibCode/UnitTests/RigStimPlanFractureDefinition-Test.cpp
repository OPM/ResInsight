#include "gtest/gtest.h"

#include "RigStimPlanFractureDefinition.h"

//--------------------------------------------------------------------------------------------------
/// minY/maxY (and minDepth/maxDepth derived from them) must not crash when the StimPlan fracture
/// definition has no Y samples, e.g. when the "ys" element is missing from the XML file.
//--------------------------------------------------------------------------------------------------
TEST( RigStimPlanFractureDefinitionTest, MinMaxYWithEmptyYsDoesNotCrash )
{
    cvf::ref<RigStimPlanFractureDefinition> fractureData = new RigStimPlanFractureDefinition;

    EXPECT_EQ( size_t( 0 ), fractureData->yCount() );

    EXPECT_NO_FATAL_FAILURE( fractureData->minDepth() );
    EXPECT_NO_FATAL_FAILURE( fractureData->maxDepth() );
}

//--------------------------------------------------------------------------------------------------
/// Data for a time step that is not in the time step list is ignored
//--------------------------------------------------------------------------------------------------
TEST( RigStimPlanFractureDefinitionTest, SetDataAtUnknownTimeValueIsIgnored )
{
    cvf::ref<RigStimPlanFractureDefinition> fractureData = new RigStimPlanFractureDefinition;

    const std::vector<std::vector<double>> data = { { 1.0, 2.0 } };
    EXPECT_FALSE( fractureData->setDataAtTimeValue( "CONDUCTIVITY", "md-m", data, 10.0 ) );
    EXPECT_TRUE( fractureData->getDataAtTimeIndex( "CONDUCTIVITY", "md-m", 0 ).empty() );

    fractureData->addTimeStep( 10.0 );
    EXPECT_TRUE( fractureData->setDataAtTimeValue( "CONDUCTIVITY", "md-m", data, 10.0 ) );
    EXPECT_FALSE( fractureData->setDataAtTimeValue( "CONDUCTIVITY", "md-m", data, 20.0 ) );
    EXPECT_EQ( data, fractureData->getDataAtTimeIndex( "CONDUCTIVITY", "md-m", 0 ) );
}
