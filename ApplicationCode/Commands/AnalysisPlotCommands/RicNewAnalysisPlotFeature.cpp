/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewAnalysisPlotFeature.h"

#include "RimAnalysisPlot.h"
#include "RimAnalysisPlotCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewAnalysisPlotFeature, "RicNewAnalysisPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewAnalysisPlotFeature::isCommandEnabled()
{
    RimAnalysisPlotCollection* analysisPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( analysisPlotColl );
    }

    if ( analysisPlotColl ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewAnalysisPlotFeature::onActionTriggered( bool isChecked )
{
    RimAnalysisPlotCollection* analysisPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( analysisPlotColl );
    }

    if ( !analysisPlotColl ) return;

    auto newPlot = analysisPlotColl->createAnalysisPlot();
    newPlot->loadDataAndUpdate();

    analysisPlotColl->updateConnectedEditors();

    RiuPlotMainWindowTools::setExpanded( newPlot );
    RiuPlotMainWindowTools::selectAsCurrentItem( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewAnalysisPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Analysis Plot" );
    actionToSetup->setIcon( QIcon( ":/AnalysisPlot16x16.png" ) );
}
