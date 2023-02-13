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

#include "RimCalculatedSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCalculation.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryCalculationCollection, "RimSummaryCalculationCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationCollection::RimSummaryCalculationCollection()
{
    CAF_PDM_InitObject( "Calculation Collection", ":/chain.png" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "" );
    m_cases.uiCapability()->setUiTreeHidden( true );
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
RimCalculatedSummaryCase* RimSummaryCalculationCollection::calculationSummaryCase( RimSummaryCase* summaryCase )
{
    for ( RimCalculatedSummaryCase* c : m_cases )
    {
        if ( c->summaryCase() == summaryCase ) return c;
    }

    // Calculated case was not found: create it.
    auto calculationSummaryCase = new RimCalculatedSummaryCase;
    calculationSummaryCase->setSummaryCase( summaryCase );
    m_cases.push_back( calculationSummaryCase );

    calculationSummaryCase->buildMetaData();

    return calculationSummaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCalculatedSummaryCase*> RimSummaryCalculationCollection::calculationSummaryCases() const
{
    return m_cases.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::rebuildCaseMetaData()
{
    ensureValidCalculationIds();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::initAfterRead()
{
    rebuildCaseMetaData();
}
