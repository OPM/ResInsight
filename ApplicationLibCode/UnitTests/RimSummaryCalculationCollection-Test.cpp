#include "gtest/gtest.h"

#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RimMockSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include <vector>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Create a calculation producing a single field address
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* createFieldCalculation()
{
    auto* calculation = dynamic_cast<RimSummaryCalculation*>( RimProject::current()->calculationCollection()->addCalculation() );

    calculation->setExpression( "MY_CALCULATION := x + 1" );
    calculation->parseExpression();

    auto* variable = dynamic_cast<RimSummaryCalculationVariable*>( calculation->variables()->at( 0 ) );

    RimSummaryAddress address;
    address.setAddress( RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    variable->setSummaryAddress( address );

    return calculation;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// The summary case main collection is a shared global object, so every test must leave it empty to
/// keep the tests order independent.
//--------------------------------------------------------------------------------------------------
class RimSummaryCalculationCollectionTest : public ::testing::Test
{
protected:
    RimSummaryCaseMainCollection* mainCollection() const { return RiaSummaryTools::summaryCaseMainCollection(); }

    void TearDown() override
    {
        for ( auto* ensemble : mainCollection()->summaryEnsembles() )
        {
            mainCollection()->removeEnsemble( ensemble );
            delete ensemble;
        }
    }
};

//--------------------------------------------------------------------------------------------------
/// The addresses of the realizations of an ensemble are not created up front, and the calculated addresses of the realizations must be
/// refreshed both when a calculation is created and when the last calculation is deleted.
///
/// https://github.com/OPM/ResInsight/issues/14559
//--------------------------------------------------------------------------------------------------
TEST_F( RimSummaryCalculationCollectionTest, RefreshCalculatedAddressesForAllRealizations )
{
    std::vector<RimMockSummaryCase*> mockCases;
    std::vector<RimSummaryCase*>     summaryCases;
    for ( int realizationNumber = 0; realizationNumber < 3; realizationNumber++ )
    {
        auto* summaryCase = createMockCase( realizationNumber );
        mockCases.push_back( dynamic_cast<RimMockSummaryCase*>( summaryCase ) );
        summaryCases.push_back( summaryCase );
    }

    mainCollection()->addEnsemble( summaryCases, "Ensemble", true );

    auto* calculation = createFieldCalculation();

    std::vector<int> refreshCountAfterCreate;
    for ( auto* mockCase : mockCases )
    {
        EXPECT_GT( mockCase->refreshCalculatedAddressesCount(), 0 );
        refreshCountAfterCreate.push_back( mockCase->refreshCalculatedAddressesCount() );
    }

    RimProject::current()->calculationCollection()->deleteCalculation( calculation );

    // Deleting the last calculation must refresh the addresses, to discard the addresses created by the calculation
    for ( size_t i = 0; i < mockCases.size(); i++ )
    {
        EXPECT_GT( mockCases[i]->refreshCalculatedAddressesCount(), refreshCountAfterCreate[i] );
    }
}
