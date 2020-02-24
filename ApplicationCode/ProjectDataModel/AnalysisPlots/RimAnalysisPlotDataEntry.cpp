/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimAnalysisPlotDataEntry.h"

#include "RiaSummaryCurveDefinition.h"

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryCaseCollection.h"

#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"

CAF_PDM_SOURCE_INIT( RimAnalysisPlotDataEntry, "AnalysisPlotDataEntry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotDataEntry::RimAnalysisPlotDataEntry()
{
    CAF_PDM_InitObject( "Data Entry", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Case", "", "", "" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address", "", "", "" );
    m_summaryAddress.uiCapability()->setUiHidden( true );
    m_summaryAddress.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryAddress = new RimSummaryAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlotDataEntry::~RimAnalysisPlotDataEntry()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlotDataEntry::setFromCurveDefinition( const RiaSummaryCurveDefinition& curveDef )
{
    m_summaryAddress->setAddress( curveDef.summaryAddress() );
    m_ensemble    = curveDef.ensemble();
    m_summaryCase = curveDef.summaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition RimAnalysisPlotDataEntry::curveDefinition() const
{
    return RiaSummaryCurveDefinition( m_summaryCase(), m_summaryAddress->address(), m_ensemble() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimAnalysisPlotDataEntry::summaryCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimAnalysisPlotDataEntry::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimAnalysisPlotDataEntry::summaryAddress() const
{
    return m_summaryAddress->address();
}
