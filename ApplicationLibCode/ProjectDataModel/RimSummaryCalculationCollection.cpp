/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryCalculationCollection.h"

#include "Summary/RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimObservedSummaryData.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

CAF_PDM_SOURCE_INIT( RimSummaryCalculationCollection, "RimSummaryCalculationCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationCollection::RimSummaryCalculationCollection()
{
    CAF_PDM_InitObject( "Calculation Collection", ":/chain.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RimSummaryCalculationCollection::createCalculation() const
{
    return new RimSummaryCalculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::updateDataDependingOnCalculations()
{
    // One more update is required after the last calculation is deleted, to remove the data created by that calculation.
    if ( calculations().empty() && !m_hasDataFromCalculations ) return;

    m_hasDataFromCalculations = !calculations().empty();

    // Refresh data sources tree
    // Refresh meta data for all summary cases and rebuild AddressNodes in the summary tree
    if ( auto summaryCaseCollection = RiaSummaryTools::summaryCaseMainCollection() )
    {
        auto summaryCases = summaryCaseCollection->allSummaryCases();
        for ( RimSummaryCase* summaryCase : summaryCases )
        {
            if ( !summaryCase ) continue;

            if ( summaryCase->showTreeNodes() )
            {
                if ( auto reader = summaryCase->summaryReader() )
                {
                    reader->createAndSetAddresses();
                }
                summaryCase->onCalculationUpdated();
            }
        }
    }

    RimObservedDataCollection* observedDataCollection = RiaSummaryTools::observedDataCollection();
    auto                       observedData           = observedDataCollection->allObservedSummaryData();
    for ( auto obs : observedData )
    {
        if ( !obs ) continue;

        if ( auto reader = obs->summaryReader() )
        {
            reader->createAndSetAddresses();
            obs->onCalculationUpdated();
        }
    }

    if ( auto summaryCaseCollection = RiaSummaryTools::summaryCaseMainCollection() )
    {
        // A delta ensemble derives its addresses from the source ensembles, and must be updated after the source ensembles. The delta
        // ensembles are stored after the source ensembles, as an ensemble must exist before it can be used as a source.
        std::vector<RimSummaryEnsemble*> deltaEnsembles;

        for ( auto ensemble : summaryCaseCollection->summaryEnsembles() )
        {
            if ( dynamic_cast<RimDeltaSummaryEnsemble*>( ensemble ) )
            {
                deltaEnsembles.push_back( ensemble );
            }
            else
            {
                ensemble->onCalculationUpdated();
            }
        }

        for ( auto ensemble : deltaEnsembles )
        {
            ensemble->onCalculationUpdated();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::rebuildCaseMetaData()
{
    ensureValidCalculationIds();
    updateDataDependingOnCalculations();
}
