////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicCreatePlotFromSelectionFeature.h"

#include "RiaGuiApplication.h"

#include "RicSummaryPlotTemplateTools.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimSummaryCase.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreatePlotFromSelectionFeature, "RicCreatePlotFromSelectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreatePlotFromSelectionFeature::isCommandEnabled()
{
    bool anySummaryCases           = !RicSummaryPlotTemplateTools::selectedSummaryCases().empty();
    bool anySummaryCaseCollections = !RicSummaryPlotTemplateTools::selectedSummaryCaseCollections().empty();

    return ( anySummaryCases || anySummaryCaseCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromSelectionFeature::onActionTriggered( bool isChecked )
{
    QString fileName           = RicSummaryPlotTemplateTools::selectPlotTemplatePath();
    auto    sumCases           = RicSummaryPlotTemplateTools::selectedSummaryCases();
    auto    sumCaseCollections = RicSummaryPlotTemplateTools::selectedSummaryCaseCollections();

    RimSummaryPlot* newSummaryPlot = RicSummaryPlotTemplateTools::createPlotFromTemplateFile( fileName );
    RicSummaryPlotTemplateTools::appendSummaryPlotToPlotCollection( newSummaryPlot, sumCases, sumCaseCollections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreatePlotFromSelectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Plot from Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}
