/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicSelectCaseOrEnsembleUi.h"

#include "Summary/RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

CAF_PDM_SOURCE_INIT( RicSelectCaseOrEnsembleUi, "RicSelectCaseOrEnsembleUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSelectCaseOrEnsembleUi::RicSelectCaseOrEnsembleUi()
    : m_useEnsembleMode( false )
{
    CAF_PDM_InitObject( "RicSelectCaseOrEnsembleUi" );

    CAF_PDM_InitFieldNoDefault( &m_selectedSummaryCase, "SelectedSummaryCase", "Summary Case" );
    m_selectedSummaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_selectedEnsemble, "SelectedEnsemble", "Ensemble" );
    m_selectedEnsemble.uiCapability()->setAutoAddingOptionFromValue( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectCaseOrEnsembleUi::setEnsembleSelectionMode( bool ensembleMode )
{
    m_useEnsembleMode = ensembleMode;

    RimProject* proj = RimProject::current();

    if ( ensembleMode )
    {
        std::vector<RimSummaryEnsemble*> ensembles = proj->summaryEnsembles();

        for ( RimSummaryEnsemble* ensemble : ensembles )
        {
            if ( ensemble->isEnsemble() )
            {
                m_selectedEnsemble = ensemble;
                break;
            }
        }
    }
    else
    {
        std::vector<RimSummaryCase*> cases = proj->allSummaryCases();
        if ( !cases.empty() ) m_selectedSummaryCase = cases.front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectCaseOrEnsembleUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedSummaryCase )
    {
        options = RiaSummaryTools::optionsForAllSummaryCases();
    }
    else if ( fieldNeedingOptions == &m_selectedEnsemble )
    {
        RimProject*                      proj   = RimProject::current();
        std::vector<RimSummaryEnsemble*> ensembles = proj->summaryEnsembles();

        for ( RimSummaryEnsemble* ensemble : ensembles )
        {
            if ( ensemble->isEnsemble() ) options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectCaseOrEnsembleUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_useEnsembleMode )
        uiOrdering.add( &m_selectedEnsemble );
    else
        uiOrdering.add( &m_selectedSummaryCase );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RicSelectCaseOrEnsembleUi::selectedSummaryCase() const
{
    if ( m_useEnsembleMode ) return nullptr;

    return m_selectedSummaryCase();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicSelectCaseOrEnsembleUi::selectedEnsemble() const
{
    if ( !m_useEnsembleMode ) return nullptr;

    return m_selectedEnsemble();
}
