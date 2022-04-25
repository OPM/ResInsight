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

    CAF_PDM_InitFieldNoDefault( &m_calcuationSummaryCase, "CalculationsSummaryCase", "Calculations Summary Case" );
    m_calcuationSummaryCase.xmlCapability()->disableIO();
    m_calcuationSummaryCase = new RimCalculatedSummaryCase;
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
RimSummaryCase* RimSummaryCalculationCollection::calculationSummaryCase()
{
    return m_calcuationSummaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::rebuildCaseMetaData()
{
    ensureCalculationIds();
    m_calcuationSummaryCase->buildMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationCollection::initAfterRead()
{
    rebuildCaseMetaData();
}
