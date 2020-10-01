/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicShowPlotWindowFeature.h"

#include "RiaGuiApplication.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowPlotWindowFeature, "RicShowPlotWindowFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowPlotWindowFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotWindowFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowPlotWindowFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Open Plot Window" );
    actionToSetup->setToolTip( "Open Plot Window (Ctrl+Shift+P)" );
    actionToSetup->setIcon( QIcon( ":/PlotWindow.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Shift+P" ) ) );
}
