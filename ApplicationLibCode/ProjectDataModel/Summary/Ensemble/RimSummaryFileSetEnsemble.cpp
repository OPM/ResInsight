/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimSummaryFileSetEnsemble.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetCollection.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "RimSummaryCase.h"

CAF_PDM_SOURCE_INIT( RimSummaryFileSetEnsemble, "RimSummaryFileSetEnsemble" );

namespace internal
{
const QString pathPatternPlaceholder = "*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryFileSetEnsemble::RimSummaryFileSetEnsemble()
{
    CAF_PDM_InitObject( "Path Pattern Ensemble", ":/SummaryCase.svg" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFileSet, "EnsembleFileSet", "Ensemble File Set" );

    m_cases.xmlCapability()->disableIO();

    setAsEnsemble( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSet* RimSummaryFileSetEnsemble::ensembleFileSet()
{
    return m_ensembleFileSet();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::setEnsembleFileSet( RimEnsembleFileSet* ensembleFileSet )
{
    m_ensembleFileSet = ensembleFileSet;
    setGroupingMode( m_ensembleFileSet->groupingMode() );
    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::updateName( const std::set<QString>& existingEnsembleNames )
{
    QString candidateName = m_ensembleFileSet() ? m_ensembleFileSet()->name() : "Path Pattern Ensemble";

    if ( m_name == candidateName ) return;

    m_name = candidateName;
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::cleanupBeforeDelete()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->fileSetChanged.disconnect( this );
        m_ensembleFileSet()->nameChanged.disconnect( this );

        auto fileSet      = m_ensembleFileSet();
        m_ensembleFileSet = nullptr;

        if ( auto coll = fileSet->firstAncestorOrThisOfType<RimEnsembleFileSetCollection>() )
        {
            coll->deleteFileSetIfPossible( fileSet );
        }
    }
    RimSummaryEnsemble::cleanupBeforeDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::reloadCases()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->reload();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryFileSetEnsemble::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_ensembleFileSet )
    {
        return RimEnsembleFileSetTools::ensembleFileSetOptions();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onFileSetChanged( const caf::SignalEmitter* emitter )
{
    bool notifyChange = true;
    createSummaryCasesFromEnsembleFileSet( notifyChange );
    RiaSummaryTools::updateConnectedPlots( this );
    buildChildNodes();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onFileSetNameChanged( const caf::SignalEmitter* emitter )
{
    updateName( {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::createSummaryCasesFromEnsembleFileSet( bool notifyChange )
{
    m_cases.deleteChildrenAsync();

    if ( m_ensembleFileSet() )
    {
        auto paths = m_ensembleFileSet()->createPaths( ".SMSPEC" );

        RiaDefines::FileType fileType = RiaDefines::FileType::SMSPEC;

        RiaEnsembleImportTools::CreateConfig createConfig{ .fileType              = fileType,
                                                           .ensembleOrGroup       = false,
                                                           .allowDialogs          = false,
                                                           .buildSummaryAddresses = false };

        auto newCases = RiaEnsembleImportTools::createSummaryCasesFromFiles( paths, createConfig );
        if ( newCases.empty() )
        {
            RiaLogging::warning( "No new cases are created." );
            return;
        }

        replaceCases( newCases, notifyChange );

        // Update name of cases and ensemble after all cases are added
        for ( auto summaryCase : newCases )
        {
            summaryCase->setShowVectorItemsInProjectTree( false );
            summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
            summaryCase->updateAutoShortName();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::initAfterRead()
{
    RimSummaryEnsemble::initAfterRead();

    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::connectSignals()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->fileSetChanged.connect( this, &RimSummaryFileSetEnsemble::onFileSetChanged );
        m_ensembleFileSet()->nameChanged.connect( this, &RimSummaryFileSetEnsemble::onFileSetNameChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_ensembleFileSet() )
    {
        auto group = uiOrdering.addNewGroup( "Ensemble Definition" );
        m_ensembleFileSet()->uiOrdering( uiConfigName, *group );
    }

    auto developersGroup = uiOrdering.addNewGroup( " -- Developers --" );
    developersGroup->setCollapsedByDefault();
    developersGroup->add( &m_ensembleFileSet );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSummaryEnsemble::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_ensembleFileSet )
    {
        connectSignals();
        bool notifyChange = true;
        createSummaryCasesFromEnsembleFileSet( notifyChange );
        RiaSummaryTools::updateConnectedPlots( this );
        caseNameChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onLoadDataAndUpdate()
{
    if ( m_cases.empty() )
    {
        bool notifyChange = false;
        createSummaryCasesFromEnsembleFileSet( notifyChange );
    }

    RimSummaryEnsemble::onLoadDataAndUpdate();
}
