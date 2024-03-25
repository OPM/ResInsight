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

#include "RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimObservedSummaryData.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"

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
    // Refresh data sources tree
    // Refresh meta data for all summary cases and rebuild AddressNodes in the summary tree
    RimSummaryCaseMainCollection* summaryCaseCollection = RiaSummaryTools::summaryCaseMainCollection();
    auto                          summaryCases          = summaryCaseCollection->allSummaryCases();
    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        if ( !summaryCase ) continue;

        if ( auto reader = summaryCase->summaryReader() )
        {
            reader->buildMetaData();

            if ( summaryCase->showRealizationDataSources() )
            {
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
            reader->buildMetaData();
            obs->onCalculationUpdated();
        }
    }

    auto summaryCaseCollections = summaryCaseCollection->summaryCaseCollections();
    for ( RimSummaryCaseCollection* summaryCaseCollection : summaryCaseCollections )
    {
        summaryCaseCollection->onCalculationUpdated();
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
