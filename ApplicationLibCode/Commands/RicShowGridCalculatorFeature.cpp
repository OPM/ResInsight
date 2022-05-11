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

#include "RicShowGridCalculatorFeature.h"

#include "RicGridCalculatorDialog.h"

#include "RiaGuiApplication.h"

#include "RimGridCalculationCollection.h"
#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"

#include "RiuMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowGridCalculatorFeature, "RicShowGridCalculatorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorDialog* RicShowGridCalculatorFeature::gridCalculatorDialog( bool createIfNotPresent )
{
    RiuMainWindow* mainWindow = RiaGuiApplication::instance()->mainWindow();

    if ( mainWindow )
    {
        return mainWindow->gridCalculatorDialog( createIfNotPresent );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowGridCalculatorFeature::hideGridCalculatorDialog()
{
    auto dialog = RicShowGridCalculatorFeature::gridCalculatorDialog( false );
    if ( dialog ) dialog->hide();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowGridCalculatorFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowGridCalculatorFeature::onActionTriggered( bool isChecked )
{
    RicGridCalculatorDialog* dialog = RicShowGridCalculatorFeature::gridCalculatorDialog( true );

    RimProject*                   proj     = RimProject::current();
    RimGridCalculationCollection* calcColl = proj->gridCalculationCollection();
    if ( calcColl->calculations().empty() )
    {
        calcColl->addCalculation();
    }

    dialog->setCalculationAndUpdateUi( calcColl->calculations()[0] );

    dialog->show();
    dialog->raise();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowGridCalculatorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Grid Property Calculator" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
