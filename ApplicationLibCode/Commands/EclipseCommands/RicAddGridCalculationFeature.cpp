/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicAddGridCalculationFeature.h"

#include "cafSelectionManager.h"

#include "RicShowGridCalculatorFeature.h"
#include "RimEclipseResultAddress.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicAddGridCalculationFeature, "RicAddGridCalculationFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAddGridCalculationFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddGridCalculationFeature::onActionTriggered( bool isChecked )
{
    auto address = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseResultAddress>();
    if ( address )
    {
        RicShowGridCalculatorFeature::addCalculationAndShowDialog( *address );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddGridCalculationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Grid Calculation" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
