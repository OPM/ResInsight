/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicEditSummaryPlotFeature.h"

#include "RiaGuiApplication.h"
#include "RiaSummaryTools.h"

#include "RicSummaryPlotEditorDialog.h"
#include "RicSummaryPlotEditorUi.h"

#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

#include <memory>

CAF_CMD_SOURCE_INIT( RicEditSummaryPlotFeature, "RicEditSummaryPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicEditSummaryPlotFeature::RicEditSummaryPlotFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::closeDialogAndResetTargetPlot()
{
    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    if ( dialog )
    {
        if ( dialog->isVisible() )
        {
            dialog->hide();
        }
        dialog->updateFromSummaryPlot( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorDialog* RicEditSummaryPlotFeature::curveCreatorDialog()
{
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();

    if ( mainPlotWindow )
    {
        return mainPlotWindow->summaryCurveCreatorDialog();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::editSummaryPlot( RimSummaryPlot* plot )
{
    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    if ( !dialog->isVisible() )
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    if ( plot )
    {
        dialog->updateFromSummaryPlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryPlotFeature::isCommandEnabled()
{
    if ( selectedSummaryPlot() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::onActionTriggered( bool isChecked )
{
    editSummaryPlot( selectedSummaryPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Edit Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicEditSummaryPlotFeature::selectedSummaryPlot()
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return sumPlot;
}
