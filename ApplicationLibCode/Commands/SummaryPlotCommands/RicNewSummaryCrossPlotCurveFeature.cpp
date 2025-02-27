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

#include "RicNewSummaryCrossPlotCurveFeature.h"

#include "RiaGuiApplication.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryCrossPlotCurveFeature, "RicNewSummaryCrossPlotCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCrossPlotCurveFeature::isCommandEnabled() const
{
    return ( selectedSummaryPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotCurveFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();

    auto plot = selectedSummaryPlot();
    if ( plot )
    {
        cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plot->curveCount() );

        RimSummaryCase* defaultCase = nullptr;
        if ( project->activeOilField()->summaryCaseMainCollection()->summaryCaseCount() > 0 )
        {
            defaultCase = project->activeOilField()->summaryCaseMainCollection()->summaryCase( 0 );
        }

        RiaSummaryCurveAddress addr( RifEclipseSummaryAddress::fieldAddress( "FOPT" ), RifEclipseSummaryAddress::fieldAddress( "FGOR" ) );
        auto                   newCurve = RiaSummaryPlotTools::addNewSummaryCurve( plot, addr, defaultCase );

        newCurve->setColor( curveColor );
        newCurve->loadDataAndUpdate( true );

        plot->zoomAll();
        plot->updateConnectedEditors();

        RiuPlotMainWindowTools::onObjectAppended( newCurve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCrossPlotCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Cross Plot Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryCrossPlotCurveFeature::selectedSummaryPlot() const
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    return RiaSummaryTools::parentSummaryPlot( selObj );
}
