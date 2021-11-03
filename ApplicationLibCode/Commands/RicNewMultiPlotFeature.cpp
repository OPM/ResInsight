/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewMultiPlotFeature.h"

#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindowTools.h"
#include <QAction>

#include "cafSelectionManager.h"

#include "cvfAssert.h"

RICF_SOURCE_INIT( RicNewMultiPlotFeature, "RicNewMultiPlotFeature", "createMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewMultiPlotFeature::RicNewMultiPlotFeature()
{
    CAF_PDM_InitFieldNoDefault( &m_plots, "plots", "Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicNewMultiPlotFeature::execute()
{
    RimProject*             project        = RimProject::current();
    RimMultiPlotCollection* plotCollection = project->mainPlotCollection()->multiPlotCollection();

    RimMultiPlot* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    if ( !m_plots().empty() )
    {
        std::vector<RimQwtPlot*> plots;
        for ( auto ptr : m_plots() )
        {
            plots.push_back( reinterpret_cast<RimQwtPlot*>( ptr ) );
        }

        for ( auto plot : plots )
        {
            auto copy =
                dynamic_cast<RimQwtPlot*>( plot->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

            {
                // TODO: Workaround for fixing the PdmPointer in RimEclipseResultDefinition
                //    caf::PdmPointer<RimEclipseCase> m_eclipseCase;
                // This pdmpointer must be changed to a ptrField

                auto saturationPressurePlotOriginal = dynamic_cast<RimSaturationPressurePlot*>( plot );
                auto saturationPressurePlotCopy     = dynamic_cast<RimSaturationPressurePlot*>( copy );
                if ( saturationPressurePlotCopy && saturationPressurePlotOriginal )
                {
                    RimSaturationPressurePlot::fixPointersAfterCopy( saturationPressurePlotOriginal,
                                                                     saturationPressurePlotCopy );
                }
            }

            plotWindow->addPlot( copy );

            copy->resolveReferencesRecursively();
            copy->revokeMdiWindowStatus();
            copy->setShowWindow( true );

            copy->loadDataAndUpdate();
        }
    }

    project->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    RiuPlotMainWindowTools::setExpanded( plotCollection, true );
    RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );

    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewMultiPlotFeature::isCommandEnabled()
{
    auto plots = selectedPlots();

    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    return !plots.empty() && plots.size() == selectedUiItems.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPlotFeature::onActionTriggered( bool isChecked )
{
    m_plots.v().clear();
    auto plots = selectedPlots();
    for ( RimPlot* plot : plots )
    {
        m_plots.v().push_back( reinterpret_cast<uintptr_t>( plot ) );
    }
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Multi Plot from Selected Plots" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicNewMultiPlotFeature::selectedPlots()
{
    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    std::vector<RimPlot*> plots;
    for ( caf::PdmUiItem* uiItem : uiItems )
    {
        RimPlot* plotInterface = dynamic_cast<RimPlot*>( uiItem );
        // Special case for all well log tracks which currently need to be in Well Log Plot for
        // depth information and cannot be moved into multiplots.
        // TODO: copy depth information into well log tracks to allow their use separately.
        RimWellLogTrack* wellLogTrack = dynamic_cast<RimWellLogTrack*>( plotInterface );
        if ( plotInterface && !wellLogTrack )
        {
            plots.push_back( plotInterface );
        }
    }
    return plots;
}
