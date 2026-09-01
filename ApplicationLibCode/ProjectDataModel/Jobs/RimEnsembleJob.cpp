/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimEnsembleJob.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"
#include "RimTools.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimEnsembleJob, "EnsembleJob" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleJob::RimEnsembleJob()
{
    CAF_PDM_InitObject( "Ensemble Job", ":/opm.png" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    m_ensemble = nullptr;
    m_ensemble.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedRealizations, "SelectedRealizations", "Selected Realizations" );
    m_selectedRealizations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedRealizations.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleJob::~RimEnsembleJob()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::execute()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::stop()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleJob::percentageDone() const
{
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList RimEnsembleJob::jobLog() const
{
    return QStringList();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::matchesKeyValue( const QString& key, const QString& value ) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::setFinished( bool runOk )
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setStarted()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setEnsemble( RimReservoirGridEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleJob::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedRealizations )
    {
        for ( auto* realization : m_ensemble->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( realization->uiName(), realization ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensemble )
    {
        RimTools::reservoirGridEnsembleOptionItems( &options );
    }

    return options;
}
