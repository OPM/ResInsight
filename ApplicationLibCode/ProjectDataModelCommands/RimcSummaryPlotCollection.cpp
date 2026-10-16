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
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RifSummaryReaderInterface.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QStringList>

#include <algorithm>
#include <iterator>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryPlotCollection, RimcSummaryPlotCollection_newSummaryPlot, "NewSummaryPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryPlotCollection_newSummaryPlot::RimcSummaryPlotCollection_newSummaryPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Summary Plot", "", "", "Create a new Summary Plot" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryCases, "SummaryCases", "", "", "", "Summary Cases" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ensemble, "Ensemble", "", "", "", "Ensemble" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ensembles, "Ensembles", "", "", "", "Ensembles plotted together in the same plot" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address string specifying the plot options" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSummaryPlotCollection_newSummaryPlot::execute()
{
    auto ensembles = allEnsembles( m_ensemble(), m_ensembles );
    if ( !m_summaryCases().empty() && !ensembles.empty() )
    {
        return std::unexpected( "Specify either summary cases or ensembles, not both." );
    }

    RimSummaryPlot* newPlot = createSummaryPlot( m_summaryCases.ptrReferencedObjectsByType(), ensembles, m_addressString() );

    // Plots created without an address are already added to a multi plot
    if ( newPlot && !newPlot->firstAncestorOrThisOfType<RimSummaryMultiPlot>() )
    {
        newPlot->loadDataAndUpdate();
        RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( newPlot );
    }

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*>
    RimcSummaryPlotCollection_newSummaryPlot::allEnsembles( RimSummaryEnsemble*                               ensemble,
                                                            const caf::PdmPtrArrayField<RimSummaryEnsemble*>& ensembleArray )
{
    std::vector<RimSummaryEnsemble*> ensembles = ensembleArray.ptrReferencedObjectsByType();
    if ( ensemble && std::find( ensembles.begin(), ensembles.end(), ensemble ) == ensembles.end() )
    {
        ensembles.insert( ensembles.begin(), ensemble );
    }
    return ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimcSummaryPlotCollection_newSummaryPlot::createSummaryPlot( const std::vector<RimSummaryCase*>&     summaryCases,
                                                                             const std::vector<RimSummaryEnsemble*>& ensembleCandidates,
                                                                             const QString&                          address )
{
    QStringList addressStrings = RiaTextStringTools::splitSkipEmptyParts( address, ";" );

    // References that cannot be resolved are returned as nullptr
    std::vector<RimSummaryEnsemble*> ensembles;
    std::copy_if( ensembleCandidates.begin(),
                  ensembleCandidates.end(),
                  std::back_inserter( ensembles ),
                  []( auto e ) { return e != nullptr; } );

    RimSummaryPlot* newPlot = nullptr;

    if ( !ensembles.empty() )
    {
        if ( !addressStrings.empty() )
        {
            // Summary addresses are created on demand, make sure they are available before applying address filters
            for ( auto ensemble : ensembles )
            {
                for ( auto summaryCase : ensemble->allSummaryCases() )
                {
                    if ( auto reader = summaryCase ? summaryCase->summaryReader() : nullptr ) reader->createAddressesIfRequired();
                }
            }

            newPlot = RicSummaryPlotFeatureImpl::createSummaryPlotForEnsembles( ensembles, addressStrings );
        }
        else
        {
            newPlot = RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( ensembles );
        }
    }
    if ( !summaryCases.empty() )
    {
        if ( !addressStrings.empty() )
        {
            newPlot = RicSummaryPlotFeatureImpl::createSummaryPlotForCases( summaryCases, addressStrings );
        }
        else
        {
            newPlot = RicNewDefaultSummaryPlotFeature::createFromSummaryCases( summaryCases );
        }
    }

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSummaryPlotCollection_newSummaryPlot::classKeywordReturnedType() const
{
    return RimSummaryPlot::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryPlotCollection, RimcSummaryPlotCollection_newSummaryMultiPlot, "NewSummaryMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryPlotCollection_newSummaryMultiPlot::RimcSummaryPlotCollection_newSummaryMultiPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Summary Multi Plot", "", "", "Create a new multi plot with one subplot for each address" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryCases, "SummaryCases", "", "", "", "Summary Cases" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ensembles, "Ensembles", "", "", "", "Ensembles plotted together in each subplot" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressStrings, "Addresses", "", "", "", "One formatted address string for each subplot" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSummaryPlotCollection_newSummaryMultiPlot::execute()
{
    if ( m_addressStrings().empty() ) return std::unexpected( "Specify at least one address." );

    if ( !m_summaryCases().empty() && !m_ensembles().empty() )
    {
        return std::unexpected( "Specify either summary cases or ensembles, not both." );
    }

    // A subplot without an address is created in a separate multi plot
    for ( const QString& address : m_addressStrings() )
    {
        if ( RiaTextStringTools::splitSkipEmptyParts( address, ";" ).isEmpty() ) return std::unexpected( "Addresses cannot be empty." );
    }

    std::vector<RimSummaryPlot*> plots;
    for ( const QString& address : m_addressStrings() )
    {
        RimSummaryPlot* plot = RimcSummaryPlotCollection_newSummaryPlot::createSummaryPlot( m_summaryCases.ptrReferencedObjectsByType(),
                                                                                            m_ensembles.ptrReferencedObjectsByType(),
                                                                                            address );
        if ( plot )
        {
            plot->loadDataAndUpdate();
            plots.push_back( plot );
        }
    }

    if ( plots.empty() ) return std::unexpected( "Unable to create subplots. Specify summary cases or ensembles." );

    return RiaSummaryPlotTools::createAndAppendSummaryMultiPlot( plots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSummaryPlotCollection_newSummaryMultiPlot::classKeywordReturnedType() const
{
    return RimSummaryMultiPlot::classKeywordStatic();
}
