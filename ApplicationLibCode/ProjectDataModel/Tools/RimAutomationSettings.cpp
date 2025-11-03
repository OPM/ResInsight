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

#include "RimSummaryMultiPlot.h"

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

    CAF_PDM_InitField( &m_caseReloadIntervalSeconds,
                       "CaseReloadIntervalSeconds",
                       15,
                       "Case Reload Interval (seconds)",
                       "Interval in seconds for automatic reload of cases when enabled." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimAutomationSettings::summaryPlots() const
{
    if ( auto summaryPlot = dynamic_cast<RimSummaryPlot*>( m_cellSelectionDestination() ) )
    {
        return { summaryPlot };
    }

    if ( auto multiSummaryPlot = dynamic_cast<RimSummaryMultiPlot*>( m_cellSelectionDestination() ) )
    {
        return multiSummaryPlot->summaryPlots();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAutomationSettings::setDestinationPlot( RimPlotWindow* plotWindow )
{
    m_cellSelectionDestination = plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAutomationSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( "Destination Plot for Cell Selection" );

    group->add( &m_cellSelectionDestination );
    m_cellSelectionDestination.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    auto refreshGrp = uiOrdering.addNewGroup( "Plot Update" );
    refreshGrp->add( &m_caseReloadIntervalSeconds );
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
            for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
            {
                auto mainPlotName = multiPlot->description();

                options.push_back( caf::PdmOptionItemInfo( mainPlotName, multiPlot, false, multiPlot->uiIconProvider() ) );

                for ( RimSummaryPlot* plot : multiPlot->summaryPlots() )
                {
                    QString displayName = mainPlotName + " : ";
                    displayName += plot->userDescriptionField()->uiCapability()->uiValue().toString();
                    options.push_back( caf::PdmOptionItemInfo( displayName, plot, false, plot->uiIconProvider() ) );
                }
            }
        }

        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAutomationSettings::caseReloadIntervalMs() const
{
    return m_caseReloadIntervalSeconds() * 1000;
}
