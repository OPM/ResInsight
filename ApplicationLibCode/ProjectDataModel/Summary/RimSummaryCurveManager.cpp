////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSummaryCurveManager.h"

#include "RimSummaryPlot.h"
#include "cafPdmObjectHandle.h"
#include "cafSelectionManager.h"

CAF_PDM_SOURCE_INIT( RimSummaryCurveManager, "RimSummaryCurveManager" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveManager::RimSummaryCurveManager()
{
    CAF_PDM_InitObject( "Summary Curve Manager", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    updateFromSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::updateFromSelection()
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimSummaryPlot* summaryPlot = nullptr;
    if ( destinationObject ) destinationObject->firstAncestorOrThisOfType( summaryPlot );

    if ( m_summaryPlot != summaryPlot )
    {
        m_summaryPlot = summaryPlot;
        updateConnectedEditors();
    }
}
