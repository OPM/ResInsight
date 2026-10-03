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
