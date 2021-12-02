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

#include "RicSummaryPlotBuilder.h"

#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"

#include "RiuPlotMainWindowTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotBuilder::RicSummaryPlotBuilder()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot* RicSummaryPlotBuilder::createMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimProject*             project        = RimProject::current();
    RimMultiPlotCollection* plotCollection = project->mainPlotCollection()->multiPlotCollection();

    RimMultiPlot* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    for ( auto plot : plots )
    {
        auto copy = dynamic_cast<RimPlot*>( plot->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

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

    project->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    RiuPlotMainWindowTools::setExpanded( plotCollection, true );
    RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );

    return plotWindow;
}
