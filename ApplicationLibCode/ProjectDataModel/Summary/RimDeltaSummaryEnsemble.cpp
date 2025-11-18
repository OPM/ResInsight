/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaQDateTimeTools.h"

#include "Summary/RiaSummaryTools.h"

#include "SummaryPlotCommands/RicNewDerivedEnsembleFeature.h"

#include "RifSummaryReaderInterface.h"

#include "RimDeltaSummaryCase.h"
#include "RimDeltaSummaryEnsemble.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDateTime>

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<RimDeltaSummaryEnsemble::FixedTimeStepMode>::setUp()
{
    addItem( RimDeltaSummaryEnsemble::FixedTimeStepMode::FIXED_TIME_STEP_NONE, "FIXED_TIME_STEP_NONE", "None" );
    addItem( RimDeltaSummaryEnsemble::FixedTimeStepMode::FIXED_TIME_STEP_CASE_1, "FIXED_TIME_STEP_CASE_1", "Ensemble 1" );
    addItem( RimDeltaSummaryEnsemble::FixedTimeStepMode::FIXED_TIME_STEP_CASE_2, "FIXED_TIME_STEP_CASE_2", "Ensemble 2" );
    setDefault( RimDeltaSummaryEnsemble::FixedTimeStepMode::FIXED_TIME_STEP_NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimDeltaSummaryEnsemble, "RimDeltaSummaryEnsemble", "RimDerivedEnsembleCaseCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeltaSummaryEnsemble::RimDeltaSummaryEnsemble()
{
    CAF_PDM_InitObject( "Delta Ensemble", ":/SummaryEnsemble.svg" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble1, "Ensemble1", "Ensemble 1" );
    m_ensemble1.uiCapability()->setUiTreeChildrenHidden( true );
    m_ensemble1.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_ensemble2, "Ensemble2", "Ensemble 2" );
    m_ensemble1.uiCapability()->setUiTreeChildrenHidden( true );
    m_ensemble2.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_operator, "Operator", "Operator" );

    CAF_PDM_InitField( &m_swapEnsemblesButton, "SwapEnsembles", false, "SwapEnsembles" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_swapEnsemblesButton );

    CAF_PDM_InitField( &m_caseCount, "CaseCount", QString( "" ), "Matching Cases" );
    m_caseCount.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_matchOnParameters, "MatchOnParameters", false, "Match On Parameters" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_matchOnParameters );

    CAF_PDM_InitField( &m_discardMissingOrIncompleteRealizations,
                       "DiscardMissingOrIncompleteRealizations",
                       true,
                       "Discard Missing or Incomplete Realizations" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_discardMissingOrIncompleteRealizations );

    CAF_PDM_InitFieldNoDefault( &m_useFixedTimeStep, "UseFixedTimeStep", "Use Fixed Time Step" );
    CAF_PDM_InitField( &m_fixedTimeStepIndex, "FixedTimeStepIndex", 0, "Time Step" );
    m_fixedTimeStepIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_fixedTimeStepIndex.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    setNameTemplate( "Delta Ensemble" );

    setAsEnsemble( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeltaSummaryEnsemble::~RimDeltaSummaryEnsemble()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::setEnsemble1( RimSummaryEnsemble* ensemble )
{
    m_ensemble1 = ensemble;

    RiaSummaryTools::updateSummaryEnsembleNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::setEnsemble2( RimSummaryEnsemble* ensemble )
{
    m_ensemble2 = ensemble;

    RiaSummaryTools::updateSummaryEnsembleNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimDeltaSummaryEnsemble::allSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;
    for ( auto sumCase : allDerivedCases( true ) )
        cases.push_back( sumCase );
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimDeltaSummaryEnsemble::ensembleSummaryAddresses() const
{
    std::set<RifEclipseSummaryAddress> addresses;
    if ( !m_ensemble1 || !m_ensemble2 ) return addresses;

    addresses   = m_ensemble1->ensembleSummaryAddresses();
    auto addrs2 = m_ensemble2->ensembleSummaryAddresses();
    addresses.insert( addrs2.begin(), addrs2.end() );
    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::createDerivedEnsembleCases()
{
    if ( !m_ensemble1 || !m_ensemble2 ) return;

    setAllCasesNotInUse();

    const auto cases1 = m_ensemble1->allSummaryCases();
    const auto cases2 = m_ensemble2->allSummaryCases();

    for ( auto& sumCase1 : cases1 )
    {
        auto crp = sumCase1->caseRealizationParameters();
        if ( !crp ) continue;

        RimSummaryCase* summaryCase2 = nullptr;
        if ( m_matchOnParameters )
        {
            summaryCase2 = findCaseByParametersHash( cases2, crp->parametersHash() );
        }
        else
        {
            summaryCase2 = findCaseByRealizationNumber( cases2, crp->realizationNumber() );
        }
        if ( !summaryCase2 ) continue;

        auto derivedCase = firstCaseNotInUse();
        derivedCase->setSummaryCases( sumCase1, summaryCase2 );
        derivedCase->setOperator( m_operator() );

        int fixedTimeStepCase1 = -1;
        int fixedTimeStepCase2 = -1;
        if ( m_useFixedTimeStep == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 )
        {
            fixedTimeStepCase1 = m_fixedTimeStepIndex;
        }
        else if ( m_useFixedTimeStep == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 )
        {
            fixedTimeStepCase2 = m_fixedTimeStepIndex;
        }

        derivedCase->setFixedTimeSteps( fixedTimeStepCase1, fixedTimeStepCase2 );
        derivedCase->createSummaryReaderInterface();
        derivedCase->setCaseRealizationParameters( crp );
        derivedCase->setInUse( true );
        derivedCase->updateDisplayNameFromCases();
    }

    // If other derived ensembles are referring to this ensemble, update their cases as well
    for ( auto referring : findReferringEnsembles() )
    {
        referring->createDerivedEnsembleCases();
    }

    deleteCasesNoInUse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDeltaSummaryEnsemble::discardMissingOrIncompleteRealizations() const
{
    return m_discardMissingOrIncompleteRealizations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> RimDeltaSummaryEnsemble::nameKeys() const
{
    QString nameCase1 = "None";

    if ( m_ensemble1 )
    {
        nameCase1 = m_ensemble1->name();
    }

    QString nameCase2 = "None";
    if ( m_ensemble2 )
    {
        nameCase2 = m_ensemble2->name();
    }

    return std::make_pair( nameCase1.toStdString(), nameCase2.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDeltaSummaryEnsemble::nameTemplateText() const
{
    return "Delta: " + RiaDefines::key1VariableName() + " - " + RiaDefines::key2VariableName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDeltaSummaryEnsemble::hasCaseReference( const RimSummaryCase* sumCase ) const
{
    if ( m_ensemble1 )
    {
        for ( auto currCase : m_ensemble1->allSummaryCases() )
        {
            if ( currCase == sumCase ) return true;
        }
    }

    if ( m_ensemble2 )
    {
        for ( auto currCase : m_ensemble2->allSummaryCases() )
        {
            if ( currCase == sumCase ) return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::onLoadDataAndUpdate()
{
    updateDerivedEnsembleCases();
    updateReferringCurveSets();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimDeltaSummaryEnsemble::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensemble1 || fieldNeedingOptions == &m_ensemble2 )
    {
        for ( auto ensemble : allEnsembles() )
        {
            if ( ensemble != this ) options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
        }
    }
    else if ( fieldNeedingOptions == &m_caseCount )
    {
        size_t caseCount1 = m_ensemble1 ? m_ensemble1->allSummaryCases().size() : 0;
        size_t caseCount2 = m_ensemble2 ? m_ensemble2->allSummaryCases().size() : 0;

        m_caseCount = QString( "%1 / %2" ).arg( (int)m_cases.size() ).arg( std::max( caseCount1, caseCount2 ) );
    }

    if ( fieldNeedingOptions == &m_fixedTimeStepIndex )
    {
        RimSummaryEnsemble* sourceEnsemble = nullptr;
        if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 )
        {
            sourceEnsemble = m_ensemble1;
        }
        else if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 )
        {
            sourceEnsemble = m_ensemble2;
        }

        if ( sourceEnsemble && !sourceEnsemble->allSummaryCases().empty() )
        {
            auto firstCase     = sourceEnsemble->allSummaryCases().front();
            auto summaryReader = firstCase->summaryReader();

            const std::vector<time_t>& timeSteps = summaryReader->timeSteps( RifEclipseSummaryAddress() );

            options = RiaQDateTimeTools::createOptionItems( timeSteps );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto nameGroup = uiOrdering.addNewGroup( "Name" );
    RimSummaryEnsemble::defineUiOrdering( uiConfigName, *nameGroup );

    auto caseGroup = uiOrdering.addNewGroup( "Delta Configuration" );
    caseGroup->add( &m_caseCount );
    caseGroup->add( &m_ensemble1 );
    caseGroup->add( &m_operator );
    caseGroup->add( &m_ensemble2 );
    caseGroup->add( &m_swapEnsemblesButton );

    caseGroup->add( &m_useFixedTimeStep );
    if ( m_useFixedTimeStep() != RimDeltaSummaryEnsemble::FixedTimeStepMode::FIXED_TIME_STEP_NONE )
    {
        caseGroup->add( &m_fixedTimeStepIndex );
    }

    caseGroup->add( &m_matchOnParameters );
    caseGroup->add( &m_discardMissingOrIncompleteRealizations );

    uiOrdering.skipRemainingFields( true );

    if ( !isValid() ) m_caseCount = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    bool doUpdate      = false;
    bool doUpdateCases = false;
    bool doShowDialog  = false;

    if ( changedField == &m_ensemble1 || changedField == &m_ensemble2 || changedField == &m_matchOnParameters )
    {
        doUpdate      = true;
        doUpdateCases = true;
        doShowDialog  = true;
    }
    else if ( changedField == &m_operator || changedField == &m_useFixedTimeStep || changedField == &m_fixedTimeStepIndex ||
              changedField == &m_discardMissingOrIncompleteRealizations )
    {
        doUpdate      = true;
        doUpdateCases = true;
        doShowDialog  = false;
    }
    else if ( changedField == &m_swapEnsemblesButton )
    {
        m_swapEnsemblesButton = false;
        auto temp             = m_ensemble1();
        m_ensemble1           = m_ensemble2();
        m_ensemble2           = temp;

        doUpdate      = true;
        doUpdateCases = true;
        doShowDialog  = false;
    }

    if ( doUpdate )
    {
        RiaSummaryTools::updateSummaryEnsembleNames();

        if ( doUpdateCases )
        {
            createDerivedEnsembleCases();
            updateConnectedEditors();

            if ( doShowDialog && m_ensemble1 != nullptr && m_ensemble2 != nullptr && allSummaryCases().empty() )
            {
                RicNewDerivedEnsembleFeature::showWarningDialog();
            }
        }

        updateReferringCurveSetsZoomAll();

        // If other derived ensembles are referring to this ensemble, update their cases as well
        for ( auto referring : findReferringEnsembles() )
        {
            refering->updateReferringCurveSetsZoomAll();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_swapEnsemblesButton )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Swap Ensembles";
        }
    }
    if ( &m_fixedTimeStepIndex == field )
    {
        auto a = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( a )
        {
            a->singleSelectionMode = true;
            a->showTextFilter      = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::setAllCasesNotInUse()
{
    for ( auto derCase : allDerivedCases( true ) )
        derCase->setInUse( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::deleteCasesNoInUse()
{
    std::vector<RimDeltaSummaryCase*> inactiveCases;
    auto                              allCases = allDerivedCases( false );
    std::copy_if( allCases.begin(),
                  allCases.end(),
                  std::back_inserter( inactiveCases ),
                  []( RimDeltaSummaryCase* derCase ) { return !derCase->isInUse(); } );

    for ( auto derCase : inactiveCases )
    {
        removeCase( derCase );
        delete derCase;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeltaSummaryCase* RimDeltaSummaryEnsemble::firstCaseNotInUse()
{
    auto allCases = allDerivedCases( false );
    auto itr      = std::find_if( allCases.begin(), allCases.end(), []( RimDeltaSummaryCase* derCase ) { return !derCase->isInUse(); } );
    if ( itr != allCases.end() )
    {
        return *itr;
    }

    // If no active case was found, add a new case to the collection
    auto newCase = new RimDeltaSummaryCase();

    // Show realization data source for the first case. If we create for all, the performance will be bad
    newCase->setShowVectorItemsInProjectTree( m_cases.empty() );

    m_cases.push_back( newCase );
    return newCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDeltaSummaryCase*> RimDeltaSummaryEnsemble::allDerivedCases( bool activeOnly ) const
{
    std::vector<RimDeltaSummaryCase*> activeCases;
    for ( auto sumCase : RimSummaryEnsemble::allSummaryCases() )
    {
        auto derivedCase = dynamic_cast<RimDeltaSummaryCase*>( sumCase );
        if ( derivedCase && ( !activeOnly || derivedCase->isInUse() ) )
        {
            activeCases.push_back( derivedCase );
        }
    }
    return activeCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryEnsemble::updateDerivedEnsembleCases()
{
    for ( auto& derivedCase : allDerivedCases( true ) )
    {
        derivedCase->createSummaryReaderInterface();

        auto crp = derivedCase->summaryCase1()->caseRealizationParameters();
        if ( !crp ) continue;
        derivedCase->setCaseRealizationParameters( crp );
    }

    // If other derived ensembles are referring to this ensemble, update their cases as well
    for ( auto referring : findReferringEnsembles() )
    {
        referring->updateDerivedEnsembleCases();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDeltaSummaryEnsemble::isValid() const
{
    return m_ensemble1 != nullptr && m_ensemble2 != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDeltaSummaryEnsemble::findCaseByParametersHash( const std::vector<RimSummaryCase*>& cases, size_t hash )
{
    for ( auto sumCase : cases )
    {
        auto ensembleParameters = sumCase->caseRealizationParameters();
        if ( ensembleParameters && ensembleParameters->parametersHash() == hash ) return sumCase;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDeltaSummaryEnsemble::findCaseByRealizationNumber( const std::vector<RimSummaryCase*>& cases, int realizationNumber )
{
    for ( auto sumCase : cases )
    {
        auto ensembleParameters = sumCase->caseRealizationParameters();
        if ( ensembleParameters && ensembleParameters->realizationNumber() == realizationNumber ) return sumCase;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDeltaSummaryEnsemble*> RimDeltaSummaryEnsemble::findReferringEnsembles() const
{
    std::vector<RimDeltaSummaryEnsemble*> referringEnsembles;

    auto mainColl = firstAncestorOrThisOfType<RimSummaryCaseMainCollection>();
    if ( mainColl )
    {
        for ( auto ensemble : mainColl->summaryEnsembles() )
        {
            auto derivedEnsemble = dynamic_cast<RimDeltaSummaryEnsemble*>( ensemble );
            if ( derivedEnsemble )
            {
                if ( derivedEnsemble->m_ensemble1() == this || derivedEnsemble->m_ensemble2() == this )
                {
                    referringEnsembles.push_back( derivedEnsemble );
                }
            }
        }
    }
    return referringEnsembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> RimDeltaSummaryEnsemble::allEnsembles() const
{
    std::vector<RimSummaryEnsemble*> ensembles;

    auto project = RimProject::current();

    for ( auto ensemble : project->summaryEnsembles() )
    {
        if ( ensemble == this ) continue;

        if ( !ensemble->isEnsemble() ) continue;

        auto derivedEnsemble = dynamic_cast<const RimDeltaSummaryEnsemble*>( ensemble );
        if ( derivedEnsemble && !derivedEnsemble->isValid() ) continue;

        ensembles.push_back( ensemble );
    }
    return ensembles;
}
