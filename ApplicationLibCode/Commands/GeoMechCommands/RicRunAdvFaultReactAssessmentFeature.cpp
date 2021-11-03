/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RicRunAdvFaultReactAssessmentFeature.h"

#include "RimFaultInViewCollection.h"
#include "RimFaultRASettings.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRunAdvFaultReactAssessmentFeature, "RicRunAdvFaultReactAssessmentFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunAdvFaultReactAssessmentFeature::isCommandEnabled()
{
    RimFaultInViewCollection* faultColl = faultCollection();

    if ( faultColl )
    {
        return faultColl->faultRAAdvancedEnabled();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunAdvFaultReactAssessmentFeature::onActionTriggered( bool isChecked )
{
    runAdvancedProcessing( selectedFaultID() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunAdvFaultReactAssessmentFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Run Advanced Processing" );
}
