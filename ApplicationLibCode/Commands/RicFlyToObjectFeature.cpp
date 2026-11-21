/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicFlyToObjectFeature.h"

#include "RiaApplication.h"

#include "Rim3dPropertiesInterface.h"
#include "Rim3dView.h"
#include "Rim3dViewCameraOperations.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicFlyToObjectFeature, "RicFlyToObjectFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicFlyToObjectFeature::isCommandEnabled() const
{
    return RicFlyToObjectFeature::boundingBoxForSelectedObjects().isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::onActionTriggered( bool isChecked )
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( !activeView ) return;

    Rim3dViewCameraOperations::flyToBoundingBox( *activeView, RicFlyToObjectFeature::boundingBoxForSelectedObjects() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Fly to Object" );
    // actionToSetup->setIcon(QIcon(":/3DView16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RicFlyToObjectFeature::boundingBoxForSelectedObjects()
{
    cvf::BoundingBox bb;

    for ( auto obj : caf::SelectionManager::instance()->objectsByType<Rim3dPropertiesInterface>() )
    {
        if ( obj )
        {
            auto candidate = obj->boundingBoxInDomainCoords();
            if ( candidate.isValid() )
            {
                bb.add( candidate );
            }
        }
    }

    return bb;
}
