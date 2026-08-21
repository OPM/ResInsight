#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RifMultipleSummaryReaders.h"

#include "RimCalculatedSummaryCurveReader.h"
#include "RimMockSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"

#include <memory>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Minimal stand-in for a native file reader, providing a single address
//--------------------------------------------------------------------------------------------------
class StubNativeSummaryReader : public RifSummaryReaderInterface
{
public:
    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override { return {}; }
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override { return { false, {} }; }
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override { return {}; }
    RiaDefines::EclipseUnitSystem        unitSystem() const override { return RiaDefines::EclipseUnitSystem::UNITS_METRIC; }

    void createAndSetAddresses() override { m_allResultAddresses.insert( RifEclipseSummaryAddress::fieldAddress( "FOPT" ) ); }

    size_t keywordCount() const override { return m_allResultAddresses.size(); }
};

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

size_t calculatedAddressCount( const RifSummaryReaderInterface& reader )
{
    size_t count = 0;
    for ( const auto& adr : reader.allResultAddresses() )
    {
        if ( adr.isCalculated() ) count++;
    }

    return count;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A calculation can be created after the addresses of a summary case have been created. The addresses of the calculated readers must
/// then be refreshed, as done by RifMultipleSummaryReaders::refreshCalculatedAddresses().
///
/// https://github.com/OPM/ResInsight/issues/14559
//--------------------------------------------------------------------------------------------------
TEST( RifMultipleSummaryReaders, RefreshCalculatedAddresses )
{
    RimMockSummaryCase summaryCase;

    RifMultipleSummaryReaders multipleReaders;
    multipleReaders.addReader( std::make_unique<StubNativeSummaryReader>() );
    multipleReaders.addReader( std::make_unique<RifCalculatedSummaryCurveReader>( &summaryCase ) );

    multipleReaders.createAndSetAddresses();

    EXPECT_EQ( 1u, multipleReaders.allResultAddresses().size() );
    EXPECT_EQ( 0u, calculatedAddressCount( multipleReaders ) );

    auto* calculation = createFieldCalculation();

    multipleReaders.refreshCalculatedAddresses();

    EXPECT_EQ( 2u, multipleReaders.allResultAddresses().size() );
    EXPECT_EQ( 1u, calculatedAddressCount( multipleReaders ) );

    RimProject::current()->calculationCollection()->deleteCalculation( calculation );

    multipleReaders.refreshCalculatedAddresses();

    EXPECT_EQ( 1u, multipleReaders.allResultAddresses().size() );
    EXPECT_EQ( 0u, calculatedAddressCount( multipleReaders ) );
}

//--------------------------------------------------------------------------------------------------
/// Addresses are not created up front for the realizations of an ensemble. Refreshing the calculated addresses must not create a partial
/// set of addresses, as the calculated addresses are created along with the native addresses in createAndSetAddresses().
//--------------------------------------------------------------------------------------------------
TEST( RifMultipleSummaryReaders, RefreshCalculatedAddressesForReaderWithNoAddresses )
{
    RimMockSummaryCase summaryCase;

    RifMultipleSummaryReaders multipleReaders;
    multipleReaders.addReader( std::make_unique<StubNativeSummaryReader>() );
    multipleReaders.addReader( std::make_unique<RifCalculatedSummaryCurveReader>( &summaryCase ) );

    auto* calculation = createFieldCalculation();

    multipleReaders.refreshCalculatedAddresses();

    EXPECT_TRUE( multipleReaders.allResultAddresses().empty() );

    // The calculated addresses are created along with the addresses of the native readers
    multipleReaders.createAndSetAddresses();

    EXPECT_EQ( 2u, multipleReaders.allResultAddresses().size() );
    EXPECT_EQ( 1u, calculatedAddressCount( multipleReaders ) );

    RimProject::current()->calculationCollection()->deleteCalculation( calculation );
}
