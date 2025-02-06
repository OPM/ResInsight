/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimAutomationSettings.h"

#include "Summary/RiaSummaryTools.h"

#include "RimSummaryMultiPlotCollection.h"
#include "Summary/RimSummaryPlot.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimAutomationSettings, "RimAutomationSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAutomationSettings::RimAutomationSettings()
{
    CAF_PDM_InitObject( "Automation Settings", ":/gear.svg" );

    CAF_PDM_InitFieldNoDefault( &m_cellSelectionDestination,
                                "CellSelectionDestination",
                                "Cell Selection Destination",
                                "",
                                "Add curves to the selected Summary Plot when clicking on cells in a 3D view." );

    CAF_PDM_InitFieldNoDefault( &m_createSummaryPlot, "CreateSummaryPlot", "Create Summary Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_createSummaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimAutomationSettings::cellSelectionDestination() const
{
    return m_cellSelectionDestination();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAutomationSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_cellSelectionDestination )
    {
        if ( m_cellSelectionDestination )
        {
            m_cellSelectionDestination->setUiIconFromResourceString( ":/SummaryPlot16x16.png" );
        }
    }
    if ( changedField == &m_createSummaryPlot )
    {
        if ( m_createSummaryPlot )
        {
            RimSummaryPlot* newPlot = new RimSummaryPlot();
            RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( newPlot );

            RiuPlotMainWindowTools::selectAsCurrentItem( this );

            m_cellSelectionDestination = newPlot;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAutomationSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_cellSelectionDestination, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
    uiOrdering.add( &m_createSummaryPlot, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAutomationSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_cellSelectionDestination )
    {
        if ( auto summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection() )
        {
            summaryPlotColl->summaryPlotItemInfos( &options );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAutomationSettings::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_createSummaryPlot )
    {
        if ( auto* attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attr->m_buttonText = "New";
        }
    }
}
