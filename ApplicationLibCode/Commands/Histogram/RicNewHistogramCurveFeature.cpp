/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicNewHistogramCurveFeature.h"

#include "RiaGuiApplication.h"

#include "RicHistogramPlotTools.h"

#include "Histogram/RimEnsembleFractureHistogramDataSource.h"
#include "Histogram/RimEnsembleParameterHistogramDataSource.h"
#include "Histogram/RimEnsembleSummaryVectorHistogramDataSource.h"
#include "Histogram/RimGridStatisticsHistogramDataSource.h"
#include "Histogram/RimHistogramCurve.h"
#include "Histogram/RimHistogramCurveCollection.h"
#include "Histogram/RimHistogramPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewHistogramCurveFeature, "RicNewHistogramCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewHistogramCurveFeature::isCommandEnabled() const
{
    return selectedHistogramPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramCurveFeature::onActionTriggered( bool isChecked )
{
    RimHistogramPlot* plot = selectedHistogramPlot();
    if ( plot )
    {
        auto dataSourceType = caf::AppEnum<RicHistogramPlotTools::DataSourceType>::fromText( userData().toString() );
        RicHistogramPlotTools::createDefaultHistogramCurve( plot, dataSourceType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Histogram Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot* RicNewHistogramCurveFeature::selectedHistogramPlot() const
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        return selObj->firstAncestorOrThisOfType<RimHistogramPlot>();
    }

    return nullptr;
}
