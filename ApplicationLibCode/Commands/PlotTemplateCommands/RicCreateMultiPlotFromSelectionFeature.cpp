////////////////////////////////////////////////////////////////////////////////
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

#include "RicCreateMultiPlotFromSelectionFeature.h"

#include "RimSummaryMultiPlot.h"
#include "Summary/RiaSummaryPlotTemplateTools.h"

#include "RiuPlotMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateMultiPlotFromSelectionFeature, "RicCreateMultiPlotFromSelectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultiPlotFromSelectionFeature::onActionTriggered( bool isChecked )
{
    QString fileName = RicSummaryPlotTemplateTools::selectPlotTemplatePath();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::create( fileName );

    RiuPlotMainWindowTools::selectAsCurrentItem( newSummaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultiPlotFromSelectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary Plot from Template" );
    actionToSetup->setIcon( QIcon( ":/plot-template-standard.svg" ) );
}
