/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicAddFieldToQuickAccessFeature.h"

#include "QuickAccess/RimFieldReference.h"
#include "QuickAccess/RimQuickAccessCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAddFieldToQuickAccessFeature, "RicAddFieldToQuickAccessFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddFieldToQuickAccessFeature::onActionTriggered( bool isChecked )
{
    auto objects = caf::selectedObjectsByType<caf::PdmObject*>();
    if ( objects.empty() ) return;

    auto firstObject = objects.front();
    if ( !firstObject ) return;

    RimFieldReference fieldRef;
    fieldRef.setObject( firstObject );

    caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(), &fieldRef, "Select Field for Quick Access", "" );
    propertyDialog.setWindowIcon( QIcon( ":/pin.svg" ) );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        RimQuickAccessCollection::instance()->addQuickAccessField( fieldRef );
        RimQuickAccessCollection::instance()->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddFieldToQuickAccessFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Field To Quick Access" );
    actionToSetup->setIcon( QIcon( ":/pin.svg" ) );
}
