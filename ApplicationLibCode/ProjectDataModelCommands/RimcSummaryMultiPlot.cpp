/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimcSummaryMultiPlot.h"

#include "RiaTextStringTools.h"
#include "Summary/RiaSummaryPlotTools.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimcSummaryPlotCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryMultiPlot, RimcSummaryMultiPlot_appendSubplot, "AppendSubplot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryMultiPlot_appendSubplot::RimcSummaryMultiPlot_appendSubplot( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Append Subplot", "", "", "Append a new subplot to the summary plot" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryCases, "SummaryCases", "", "", "", "Summary Cases" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ensembles, "Ensembles", "", "", "", "Ensembles plotted together in the subplot" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address string specifying the plot options" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSummaryMultiPlot_appendSubplot::execute()
{
    // A plot without an address is created in a separate multi plot
    if ( RiaTextStringTools::splitSkipEmptyParts( m_addressString(), ";" ).isEmpty() ) return std::unexpected( "Specify an address." );

    auto multiPlot = self<RimSummaryMultiPlot>();

    if ( !m_summaryCases().empty() && !m_ensembles().empty() )
    {
        return std::unexpected( "Specify either summary cases or ensembles, not both." );
    }

    RimSummaryPlot* newPlot = RimcSummaryPlotCollection_newSummaryPlot::createSummaryPlot( m_summaryCases.ptrReferencedObjectsByType(),
                                                                                           m_ensembles.ptrReferencedObjectsByType(),
                                                                                           m_addressString() );
    if ( !newPlot ) return std::unexpected( "Unable to create a subplot. Specify summary cases or ensembles." );

    newPlot->loadDataAndUpdate();
    RiaSummaryPlotTools::appendPlotsToSummaryMultiPlot( multiPlot, { newPlot } );

    multiPlot->loadDataAndUpdate();
    multiPlot->updateAllRequiredEditors();

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSummaryMultiPlot_appendSubplot::classKeywordReturnedType() const
{
    return RimSummaryPlot::classKeywordStatic();
}
