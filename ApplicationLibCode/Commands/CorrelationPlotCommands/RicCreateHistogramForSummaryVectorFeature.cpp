/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicCreateHistogramForSummaryVectorFeature.h"

#include "Histogram/RimEnsembleSummaryVectorHistogramDataSource.h"
#include "RicHistogramPlotTools.h"

#include "Histogram/RimEnsembleParameterHistogramDataSource.h"
#include "RimCorrelationPlotCollection.h"
#include "RimCorrelationReportPlot.h"
#include "RimParameterResultCrossPlot.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateHistogramForSummaryVectorFeature, "RicCreateHistogramForSummaryVectorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateHistogramForSummaryVectorFeature::isCommandEnabled() const
{
    return selectedCrossPlot() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateHistogramForSummaryVectorFeature::onActionTriggered( bool isChecked )
{
    if ( auto crossPlot = selectedCrossPlot() )
    {
        auto multiplot     = RicHistogramPlotTools::addNewHistogramMultiplot();
        auto histogramPlot = RicHistogramPlotTools::addNewHistogramPlot( multiplot );

        auto ensembles = crossPlot->ensembles();
        auto timeStep  = crossPlot->timeStep();
        auto addresses = crossPlot->addresses();
        for ( RimSummaryEnsemble* ensemble : ensembles )
        {
            for ( auto address : addresses )
            {
                auto dataSource = new RimEnsembleSummaryVectorHistogramDataSource();
                dataSource->setEnsemble( ensemble );
                dataSource->setSummaryAddress( address );
                dataSource->setTimeStep( timeStep );
                RicHistogramPlotTools::createHistogramCurve( histogramPlot, dataSource );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateHistogramForSummaryVectorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
    actionToSetup->setText( "Create Histogram Plot For Summary Vector" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RicCreateHistogramForSummaryVectorFeature::selectedCrossPlot()
{
    auto selectedPlots = caf::selectedObjectsByTypeStrict<RimParameterResultCrossPlot*>();
    if ( selectedPlots.size() == 1 ) return selectedPlots.front();

    if ( auto reportPlot = caf::firstAncestorOfTypeFromSelectedObject<RimCorrelationReportPlot>() )
    {
        return reportPlot->crossPlot();
    }

    return nullptr;
}
