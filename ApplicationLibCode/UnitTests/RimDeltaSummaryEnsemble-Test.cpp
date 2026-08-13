#include "gtest/gtest.h"

#include "Summary/RiaSummaryTools.h"

#include "RigCaseRealizationParameters.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"

#include "cafPdmPtrField.h"

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

size_t countOf( const std::vector<RimDeltaSummaryEnsemble*>& ensembles, const RimDeltaSummaryEnsemble* ensemble )
{
    return static_cast<size_t>( std::count( ensembles.begin(), ensembles.end(), ensemble ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a source ensemble without going through setEnsemble1()/setEnsemble2(). The setters trigger
/// an ensemble name update, and auto generated names never converge for a cyclic dependency.
//--------------------------------------------------------------------------------------------------
void forceSourceEnsemble( RimDeltaSummaryEnsemble* deltaEnsemble, const QString& fieldKeyword, RimSummaryEnsemble* sourceEnsemble )
{
    auto* field = dynamic_cast<caf::PdmPtrField<RimSummaryEnsemble*>*>( deltaEnsemble->findField( fieldKeyword ) );
    ASSERT_TRUE( field != nullptr );

    field->setValue( sourceEnsemble );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// The summary case main collection is a shared global object, so every test must leave it empty to
/// keep the tests order independent.
//--------------------------------------------------------------------------------------------------
class RimDeltaSummaryEnsembleTest : public ::testing::Test
{
protected:
    RimSummaryCaseMainCollection* mainCollection() const { return RiaSummaryTools::summaryCaseMainCollection(); }

    RimSummaryEnsemble* createEnsemble( const QString& name, const std::vector<int>& realizationNumbers ) const
    {
        std::vector<RimSummaryCase*> cases;
        for ( auto realizationNumber : realizationNumbers )
        {
            cases.push_back( createMockCase( realizationNumber ) );
        }

        return mainCollection()->addEnsemble( cases, name, true );
    }

    RimDeltaSummaryEnsemble* createDeltaEnsemble( RimSummaryEnsemble* ensemble1, RimSummaryEnsemble* ensemble2 ) const
    {
        auto* deltaEnsemble = new RimDeltaSummaryEnsemble();
        mainCollection()->addEnsemble( deltaEnsemble );
        deltaEnsemble->setEnsemble1( ensemble1 );
        deltaEnsemble->setEnsemble2( ensemble2 );

        return deltaEnsemble;
    }

    void TearDown() override
    {
        auto ensembles = mainCollection()->summaryEnsembles();

        // Delete the delta ensembles first, they refer to the other ensembles
        std::stable_partition( ensembles.begin(),
                               ensembles.end(),
                               []( RimSummaryEnsemble* ensemble ) { return dynamic_cast<RimDeltaSummaryEnsemble*>( ensemble ) != nullptr; } );

        for ( auto* ensemble : ensembles )
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
/// objectsWithReferringPtrFields() returns one entry per referring field, so an ensemble used as both
/// sources of the same delta ensemble is reported twice by the raw PDM call.
//--------------------------------------------------------------------------------------------------
TEST_F( RimDeltaSummaryEnsembleTest, DependentDeltaEnsembles_Deduplicated )
{
    auto* ensemble      = createEnsemble( "Ensemble", { 0, 1 } );
    auto* deltaEnsemble = createDeltaEnsemble( ensemble, ensemble );

    EXPECT_EQ( size_t( 2 ), ensemble->objectsWithReferringPtrFieldsOfType<RimDeltaSummaryEnsemble>().size() );

    auto dependents = RimSummaryEnsembleTools::dependentDeltaEnsembles( ensemble );
    ASSERT_EQ( size_t( 1 ), dependents.size() );
    EXPECT_EQ( deltaEnsemble, dependents.front() );
}

//--------------------------------------------------------------------------------------------------
/// A delta ensemble must be regenerated before the delta ensembles using it as a source.
//--------------------------------------------------------------------------------------------------
TEST_F( RimDeltaSummaryEnsembleTest, DependencyOrder_ChainedDeltaEnsembles )
{
    auto* ensemble1 = createEnsemble( "Ensemble 1", { 0, 1 } );
    auto* ensemble2 = createEnsemble( "Ensemble 2", { 0, 1 } );
    auto* ensemble3 = createEnsemble( "Ensemble 3", { 0, 1 } );

    auto* deltaA = createDeltaEnsemble( ensemble1, ensemble2 );
    auto* deltaB = createDeltaEnsemble( deltaA, ensemble3 );

    auto orderFromEnsemble1 = RimSummaryEnsembleTools::deltaEnsemblesInUpdateOrder( { ensemble1 } );
    ASSERT_EQ( size_t( 2 ), orderFromEnsemble1.size() );
    EXPECT_EQ( deltaA, orderFromEnsemble1[0] );
    EXPECT_EQ( deltaB, orderFromEnsemble1[1] );

    // Ensemble 3 is only a source of the second delta ensemble
    auto orderFromEnsemble3 = RimSummaryEnsembleTools::deltaEnsemblesInUpdateOrder( { ensemble3 } );
    ASSERT_EQ( size_t( 1 ), orderFromEnsemble3.size() );
    EXPECT_EQ( deltaB, orderFromEnsemble3[0] );
}

//--------------------------------------------------------------------------------------------------
/// A cycle is constructible through the UI. The traversal must terminate and report each delta
/// ensemble once.
//--------------------------------------------------------------------------------------------------
TEST_F( RimDeltaSummaryEnsembleTest, DependencyOrder_CycleTerminates )
{
    auto* ensemble1 = createEnsemble( "Ensemble 1", { 0, 1 } );
    auto* ensemble2 = createEnsemble( "Ensemble 2", { 0, 1 } );

    auto* deltaA = createDeltaEnsemble( ensemble1, ensemble2 );
    auto* deltaB = createDeltaEnsemble( deltaA, ensemble2 );

    // Close the cycle, A now refers to B and B refers to A
    forceSourceEnsemble( deltaA, "Ensemble1", deltaB );

    auto order = RimSummaryEnsembleTools::deltaEnsemblesInUpdateOrder( { ensemble2 } );
    ASSERT_EQ( size_t( 2 ), order.size() );
    EXPECT_EQ( size_t( 1 ), countOf( order, deltaA ) );
    EXPECT_EQ( size_t( 1 ), countOf( order, deltaB ) );

    EXPECT_TRUE( RimSummaryEnsembleTools::wouldCreateDependencyCycle( deltaB, deltaA ) );

    // Break the cycle before tear down
    forceSourceEnsemble( deltaA, "Ensemble1", ensemble1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RimDeltaSummaryEnsembleTest, WouldCreateDependencyCycle )
{
    auto* ensemble1 = createEnsemble( "Ensemble 1", { 0, 1 } );
    auto* ensemble2 = createEnsemble( "Ensemble 2", { 0, 1 } );

    auto* deltaA = createDeltaEnsemble( ensemble1, ensemble2 );
    auto* deltaB = createDeltaEnsemble( deltaA, ensemble2 );

    EXPECT_TRUE( RimSummaryEnsembleTools::wouldCreateDependencyCycle( deltaA, deltaA ) );
    EXPECT_TRUE( RimSummaryEnsembleTools::wouldCreateDependencyCycle( deltaA, deltaB ) );

    EXPECT_FALSE( RimSummaryEnsembleTools::wouldCreateDependencyCycle( deltaB, deltaA ) );
    EXPECT_FALSE( RimSummaryEnsembleTools::wouldCreateDependencyCycle( deltaA, ensemble1 ) );
}
