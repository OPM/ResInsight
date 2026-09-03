#include "gtest/gtest.h"

#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RigCaseRealizationParameters.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

//--------------------------------------------------------------------------------------------------
/// The summary case main collection is a shared global object, so every test must leave it empty to
/// keep the tests order independent.
//--------------------------------------------------------------------------------------------------
class RimEnsembleCurveFilterPerformanceTest : public ::testing::TestWithParam<int>
{
protected:
    RimSummaryCaseMainCollection* mainCollection() const { return RiaSummaryTools::summaryCaseMainCollection(); }

    //--------------------------------------------------------------------------------------------------
    /// Creates an ensemble of mock summary cases. Each case has a realization number, an ensemble
    /// parameter ("PORO") and a summary vector (FOPT) with a synthetic time series so both
    /// FilterMode::ENSEMBLE_PARAMETER and FilterMode::SUMMARY_VALUE can be exercised.
    //--------------------------------------------------------------------------------------------------
    RimSummaryEnsemble* createEnsemble( const QString& name, int caseCount, size_t timeStepCount ) const
    {
        std::vector<RimSummaryCase*> cases;
        for ( int i = 0; i < caseCount; i++ )
        {
            auto* summaryCase = static_cast<RimMockSummaryCase*>( createMockCase( i ) );

            summaryCase->caseRealizationParameters()->addParameter( "PORO", static_cast<double>( i ) );

            std::vector<time_t> timeSteps;
            std::vector<double> values;
            timeSteps.reserve( timeStepCount );
            values.reserve( timeStepCount );
            for ( size_t t = 0; t < timeStepCount; t++ )
            {
                timeSteps.push_back( static_cast<time_t>( t * 86400 ) );
                values.push_back( static_cast<double>( i ) + static_cast<double>( t ) * 0.001 );
            }
            summaryCase->addVector( RifEclipseSummaryAddress::fieldAddress( "FOPT" ), timeSteps, values );

            cases.push_back( summaryCase );
        }

        return mainCollection()->addEnsemble( cases, name, true );
    }

    void TearDown() override
    {
        for ( auto* ensemble : mainCollection()->summaryEnsembles() )
        {
            mainCollection()->removeEnsemble( ensemble );
            delete ensemble;
        }

        for ( auto* summaryCase : mainCollection()->topLevelSummaryCases() )
        {
            mainCollection()->removeCase( summaryCase, false );
            delete summaryCase;
        }
    }
};

//--------------------------------------------------------------------------------------------------
/// Not run as part of the normal test suite. Parameterized over case counts (10, 30, 60 realizations,
/// see INSTANTIATE_TEST_SUITE_P below), each with 15000 time steps, so the O(N) vs O(N^2) scaling of
/// applyFilter() is visible when comparing the printed timings between runs.
///
/// Run all three case counts:
///   ResInsight-tests.exe --gtest_filter=*ApplyFilterBySummaryValue* --gtest_also_run_disabled_tests
///
/// Run a single case count (index 0 = 10 cases, 1 = 30 cases, 2 = 60 cases), e.g. for profiling:
///   ResInsight-tests.exe --gtest_filter=*ApplyFilterBySummaryValue/2 --gtest_also_run_disabled_tests
///
/// The same filter can be launched from Visual Studio using the
/// "resinsight-tests.exe - RimEnsembleCurveFilterPerformanceTest" entry in .vs/launch.vs.json.
//--------------------------------------------------------------------------------------------------
TEST_P( RimEnsembleCurveFilterPerformanceTest, DISABLED_ApplyFilterBySummaryValue )
{
    const int    caseCount      = GetParam();
    const size_t timeStepCount  = 15000;
    const int    iterationCount = 3;

    auto* ensemble = createEnsemble( "PerformanceEnsemble", caseCount, timeStepCount );
    auto  allCases = ensemble->allSummaryCases();

    auto curveSet = std::make_unique<RimEnsembleCurveSet>();
    curveSet->setSummaryEnsemble( ensemble );
    // The curve set's own Y address drives the available/selected time step range used by the filter's
    // time config. Without it, the selected time range collapses to a single time step.
    curveSet->setSummaryAddressY( RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );

    auto* filter = curveSet->filterCollection()->addFilter();
    filter->setSummaryAddresses( { RifEclipseSummaryAddress::fieldAddress( "FOPT" ) } );

    // The SUMMARY_VALUE filter compares against the value at the last time step inside the selected
    // time range, i.e. the last synthetic value of each case: value(i) = i + (timeStepCount-1)*0.001.
    // Split exactly in the middle of that value range so about half of the realizations are filtered
    // out, forcing applyFilter() to read and scan the full summary vector of every case.
    double lastStepOffset = static_cast<double>( timeStepCount - 1 ) * 0.001;
    double minFinalValue  = 0.0 + lastStepOffset;
    double maxFinalValue  = static_cast<double>( caseCount - 1 ) + lastStepOffset;
    double midValue       = 0.5 * ( minFinalValue + maxFinalValue );

    {
        auto* valueRangeField = dynamic_cast<caf::PdmField<std::pair<double, double>>*>( filter->findField( "ValueRange" ) );
        ASSERT_TRUE( valueRangeField != nullptr );
        valueRangeField->setValue( { midValue, maxFinalValue } );
    }

    std::vector<RimSummaryCase*> filteredCases;

    auto start = std::chrono::high_resolution_clock::now();
    for ( int iteration = 0; iteration < iterationCount; iteration++ )
    {
        filteredCases = filter->applyFilter( allCases );
    }
    auto                          end  = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "RimEnsembleCurveFilter::applyFilter (SUMMARY_VALUE) : " << caseCount << " cases x " << timeStepCount << " time steps, "
              << iterationCount << " iterations : total " << std::setw( 9 ) << diff.count() << " s, avg " << std::setw( 9 )
              << ( diff.count() / iterationCount ) << " s/iteration\n";

    EXPECT_GT( filteredCases.size(), size_t( 0 ) );
    EXPECT_LT( filteredCases.size(), allCases.size() );
}

//--------------------------------------------------------------------------------------------------
/// ::testing::Values( ... ) lists the case counts to instantiate the test with. gtest creates one test
/// instance per value, named CaseCounts/RimEnsembleCurveFilterPerformanceTest.DISABLED_ApplyFilterBySummaryValue/<index>,
/// where <index> is the 0-based position of the value in the list (0 -> 10, 1 -> 30, 2 -> 60). Inside
/// the test body, GetParam() returns the value for that instance. Add, remove, or change entries here
/// to test other case counts, e.g. ::testing::Values( 10, 30, 60, 120 ) adds a fourth instance (/3).
//--------------------------------------------------------------------------------------------------
INSTANTIATE_TEST_SUITE_P( CaseCounts, RimEnsembleCurveFilterPerformanceTest, ::testing::Values( 10, 30, 60 ) );

//--------------------------------------------------------------------------------------------------
/// Not run as part of the normal test suite, run manually (e.g. attached to a profiler) using
/// --gtest_filter=*RimEnsembleCurveFilterPerformanceTest* --gtest_also_run_disabled_tests
//--------------------------------------------------------------------------------------------------
TEST_F( RimEnsembleCurveFilterPerformanceTest, DISABLED_ApplyFilterByEnsembleParameter )
{
    const int caseCount      = 500;
    const int iterationCount = 20;

    auto* ensemble = createEnsemble( "PerformanceEnsemble", caseCount, 10 );
    auto  allCases = ensemble->allSummaryCases();

    auto curveSet = std::make_unique<RimEnsembleCurveSet>();
    curveSet->setSummaryEnsemble( ensemble );

    auto* filter = curveSet->filterCollection()->addFilter( "PORO" );

    // addFilter() only sets the ensemble parameter name, the filter mode still defaults to SUMMARY_VALUE.
    {
        using FilterModeField = caf::PdmField<caf::AppEnum<RimEnsembleCurveFilter::FilterMode>>;
        auto* filterModeField = dynamic_cast<FilterModeField*>( filter->findField( "FilterMode" ) );
        ASSERT_TRUE( filterModeField != nullptr );
        filterModeField->setValue( RimEnsembleCurveFilter::FilterMode::ENSEMBLE_PARAMETER );
    }

    filter->updateMaxMinAndDefaultValuesFromParent();
    ASSERT_LT( filter->minValue(), filter->maxValue() );
    double midValue = 0.5 * ( filter->minValue() + filter->maxValue() );

    {
        auto* valueRangeField = dynamic_cast<caf::PdmField<std::pair<double, double>>*>( filter->findField( "ValueRange" ) );
        ASSERT_TRUE( valueRangeField != nullptr );
        valueRangeField->setValue( { midValue, filter->maxValue() } );
    }

    std::vector<RimSummaryCase*> filteredCases;

    auto start = std::chrono::high_resolution_clock::now();
    for ( int iteration = 0; iteration < iterationCount; iteration++ )
    {
        filteredCases = filter->applyFilter( allCases );
    }
    auto                          end  = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "RimEnsembleCurveFilter::applyFilter (ENSEMBLE_PARAMETER) : " << caseCount << " cases, " << iterationCount
              << " iterations : total " << std::setw( 9 ) << diff.count() << " s, avg " << std::setw( 9 )
              << ( diff.count() / iterationCount ) << " s/iteration\n";

    EXPECT_GT( filteredCases.size(), size_t( 0 ) );
    EXPECT_LT( filteredCases.size(), allCases.size() );
}
