#include "gtest/gtest.h"

#include "Summary/RiaSummaryTools.h"

#include "RigCaseRealizationParameters.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmPointer.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace
{
RimSummaryCase* createMockCase( int realizationNumber )
{
    auto* summaryCase = new RimMockSummaryCase();

    auto parameters = std::make_shared<RigCaseRealizationParameters>();
    parameters->setRealizationNumber( realizationNumber );
    summaryCase->setCaseRealizationParameters( parameters );

    return summaryCase;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Removing a source case makes a delta ensemble recreate and delete derived cases. The derived cases
/// are part of the list of cases to remove, and must not be left as dangling pointers in that list.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryCaseMainCollection, RemoveCasesReferredByDeltaEnsemble )
{
    RimSummaryCaseMainCollection* mainCollection = RiaSummaryTools::summaryCaseMainCollection();

    auto* ensemble1 = mainCollection->addEnsemble( { createMockCase( 0 ), createMockCase( 1 ) }, "Ensemble 1", true );
    auto* ensemble2 = mainCollection->addEnsemble( { createMockCase( 0 ), createMockCase( 1 ) }, "Ensemble 2", true );

    auto* deltaEnsemble = new RimDeltaSummaryEnsemble();
    mainCollection->addEnsemble( deltaEnsemble );
    deltaEnsemble->setEnsemble1( ensemble1 );
    deltaEnsemble->setEnsemble2( ensemble2 );
    deltaEnsemble->createDerivedEnsembleCases();

    EXPECT_EQ( size_t( 2 ), deltaEnsemble->allSummaryCases().size() );

    auto cases = mainCollection->allSummaryCases();
    EXPECT_EQ( size_t( 6 ), cases.size() );

    // Guarded pointers are set to null when the object is deleted
    std::vector<caf::PdmPointer<RimSummaryCase>> guardedCases( cases.begin(), cases.end() );

    mainCollection->removeCases( cases );

    size_t aliveCount = 0;
    for ( const auto& guardedCase : guardedCases )
    {
        if ( guardedCase.notNull() ) aliveCount++;
    }

    // The two derived cases are deleted by the delta ensemble during removal
    EXPECT_EQ( size_t( 4 ), aliveCount );
    EXPECT_EQ( aliveCount, cases.size() );

    // No deleted case is left behind in the list
    for ( auto* summaryCase : cases )
    {
        auto isAlive = [summaryCase]( const caf::PdmPointer<RimSummaryCase>& guardedCase )
        { return guardedCase.notNull() && guardedCase.p() == summaryCase; };
        EXPECT_TRUE( std::any_of( guardedCases.begin(), guardedCases.end(), isAlive ) );
    }

    for ( auto* summaryCase : cases )
    {
        delete summaryCase;
    }

    mainCollection->removeEnsemble( deltaEnsemble );
    mainCollection->removeEnsemble( ensemble1 );
    mainCollection->removeEnsemble( ensemble2 );
    delete deltaEnsemble;
    delete ensemble1;
    delete ensemble2;
}
