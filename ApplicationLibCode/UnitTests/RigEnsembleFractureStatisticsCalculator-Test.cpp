#include "gtest/gtest.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigStimPlanFractureDefinition.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
/// A fracture definition without any conductivity result must not be indexed out of range
//--------------------------------------------------------------------------------------------------
TEST( RigEnsembleFractureStatisticsCalculatorTest, NoConductivityResultNames )
{
    cvf::ref<RigStimPlanFractureDefinition> definition = new RigStimPlanFractureDefinition;
    EXPECT_TRUE( definition->conductivityResultNames().isEmpty() );

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> definitions = { definition };

    for ( auto propertyType : RigEnsembleFractureStatisticsCalculator::propertyTypes() )
    {
        if ( propertyType == RigEnsembleFractureStatisticsCalculator::PropertyType::FORMATION_DIP ) continue;

        auto values = RigEnsembleFractureStatisticsCalculator::calculateProperty( definitions, propertyType );
        EXPECT_TRUE( values.empty() );
    }
}
