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

#include "RicGeoMechCopyCaseFeature.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicGeoMechCopyCaseFeature, "RicGeoMechCopyCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicGeoMechCopyCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechCopyCaseFeature::onActionTriggered( bool isChecked )
{
    RimGeoMechModels* coll = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGeoMechModels>();
    if ( coll )
    {
        // get the cases

        std::vector<RimGeoMechCase*> cases = caf::selectedObjectsByTypeStrict<RimGeoMechCase*>();

        RimGeoMechCase* caseToSelect = coll->copyCases( cases );

        if ( caseToSelect )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( caseToSelect );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGeoMechCopyCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Copy.png" ) );
    actionToSetup->setText( "Create Copy" );
}
