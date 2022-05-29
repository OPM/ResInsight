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

#include "RicEclipseShowOnlyFaultFeature.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterNewExec.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "RigFault.h"
#include "RigMainGrid.h"

#include "cafCmdExecCommandManager.h"
#include "cafPdmUiObjectHandle.h"
#include "cvfStructGrid.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEclipseShowOnlyFaultFeature, "RicEclipseShowOnlyFaultFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseShowOnlyFaultFeature::isCommandEnabled()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return false;

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );
    if ( !eclView ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseShowOnlyFaultFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( userData.isNull() || userData.type() != QVariant::String ) return;

    QString faultName = userData.toString();

    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return;

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );
    if ( !eclView ) return;

    RimFaultInViewCollection* coll     = eclView->faultCollection();
    RimFaultInView*           rimFault = coll->findFaultByName( faultName );
    if ( !rimFault ) return;
    if ( !rimFault->parentField() ) return;

    std::vector<caf::PdmObjectHandle*> children;
    rimFault->parentField()->children();

    for ( auto& child : children )
    {
        caf::PdmUiObjectHandle* childUiObject = uiObj( child );
        if ( childUiObject && childUiObject->objectToggleField() )
        {
            caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>( childUiObject->objectToggleField() );

            if ( field ) field->setValueWithFieldChanged( false );
        }
    }

    if ( rimFault )
    {
        rimFault->showFault.setValueWithFieldChanged( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseShowOnlyFaultFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/draw_style_faults_24x24.png" ) );
}
