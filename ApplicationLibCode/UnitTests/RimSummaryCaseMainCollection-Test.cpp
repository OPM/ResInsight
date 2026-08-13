#include "gtest/gtest.h"

#include "Summary/RiaSummaryTools.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCaseUpdateBatch.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmPointer.h"

#include <algorithm>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Removing a source case makes a delta ensemble rebuild its derived cases. Those derived cases are
/// part of the list of cases to remove, and must stay alive for as long as the caller holds that
/// list. The caller states that span by opening a RimSummaryCaseUpdateBatch.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryCaseMainCollection, RemoveCases_NoDanglingInCallerVector )
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

    auto aliveCount = [&guardedCases]()
    {
        return static_cast<size_t>( std::count_if( guardedCases.begin(),
                                                   guardedCases.end(),
                                                   []( const caf::PdmPointer<RimSummaryCase>& guardedCase )
                                                   { return guardedCase.notNull(); } ) );
    };

    {
        RimSummaryCaseUpdateBatch updateBatch;

        mainCollection->removeCases( cases );

        // The two derived cases are detached by the delta ensemble, but not destroyed while the batch is open
        EXPECT_EQ( cases.size(), aliveCount() );

        for ( auto* summaryCase : cases )
        {
            delete summaryCase;
        }
    }

    // Everything the caller handed over has been destroyed exactly once
    EXPECT_EQ( size_t( 0 ), aliveCount() );

    mainCollection->removeEnsemble( deltaEnsemble );
    mainCollection->removeEnsemble( ensemble1 );
    mainCollection->removeEnsemble( ensemble2 );
    delete deltaEnsemble;
    delete ensemble1;
    delete ensemble2;
}
