/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimcSummaryPlotCollection.h"

#include "Summary/RiaSummaryPlotTools.h"
#include "SummaryPlotCommands/RicNewDefaultSummaryPlotFeature.h"
#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaTextStringTools.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QStringList>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryPlotCollection, RimcSummaryPlotCollection_newSummaryPlot, "NewSummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryPlotCollection_newSummaryPlot::RimcSummaryPlotCollection_newSummaryPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Summary Plot", "", "", "Create a new Summary Plot" );
    setNullptrValid( false );
    setResultPersistent( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryCases, "SummaryCases", "", "", "", "Summary Cases" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ensemble, "Ensemble", "", "", "", "Ensemble" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address string specifying the plot options" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSummaryPlotCollection_newSummaryPlot::execute()
{
    QStringList addressStrings = RiaTextStringTools::splitSkipEmptyParts( m_addressString(), ";" );

    RimSummaryPlot* newPlot = nullptr;
    if ( m_ensemble )
    {
        if ( !addressStrings.empty() )
        {
            newPlot = RicSummaryPlotFeatureImpl::createSummaryPlotForEnsemble( std::vector<RimSummaryCase*>(), m_ensemble, addressStrings );
        }
        else
        {
            newPlot = RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( { m_ensemble } );
        }
    }
    if ( !m_summaryCases.empty() )
    {
        std::vector<RimSummaryCase*> summaryCases = m_summaryCases.ptrReferencedObjectsByType();
        if ( !addressStrings.empty() )
        {
            newPlot = RicSummaryPlotFeatureImpl::createSummaryPlotForCases( summaryCases, addressStrings );
        }
        else
        {
            newPlot = RicNewDefaultSummaryPlotFeature::createFromSummaryCases( summaryCases );
        }
    }

    if ( newPlot )
    {
        newPlot->loadDataAndUpdate();
        RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( newPlot );
    }

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSummaryPlotCollection_newSummaryPlot::resultIsPersistent_obsolete() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSummaryPlotCollection_newSummaryPlot::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimSummaryPlot );
}
