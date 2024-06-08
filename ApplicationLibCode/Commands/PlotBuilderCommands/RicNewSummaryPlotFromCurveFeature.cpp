/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicNewSummaryPlotFromCurveFeature.h"

#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "RicSummaryPlotBuilder.h"

#include <QAction>
#include <QVariant>

CAF_CMD_SOURCE_INIT( RicNewSummaryPlotFromCurveFeature, "RicNewSummaryPlotFromCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromCurveFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( !userData.isNull() && userData.canConvert<void*>() )
    {
        RimSummaryCurve* curve = static_cast<RimSummaryCurve*>( userData.value<void*>() );

        auto curveCopy = curve->copyObject<RimSummaryCurve>();
        curveCopy->setShowInLegend( true );

        RimSummaryPlot* plot = RicSummaryPlotBuilder::createPlot( { curveCopy } );

        std::vector<RimSummaryPlot*> plots = { plot };

        RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( plots );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot from Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
