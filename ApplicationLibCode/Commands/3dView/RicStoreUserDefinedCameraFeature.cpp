/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicStoreUserDefinedCameraFeature.h"

#include "RiaApplication.h"

#include "RimGridView.h"
#include "RiuMainWindow.h"

#include "RiuViewer.h"

#include "cvfCamera.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicStoreUserDefinedCameraFeature, "RicStoreUserDefinedCameraFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicStoreUserDefinedCameraFeature::groupName()
{
    return "UserDefinedCamera";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicStoreUserDefinedCameraFeature::eyeName()
{
    return "Eye";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicStoreUserDefinedCameraFeature::viewReferencePointName()
{
    return "ViewReferencePoint";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicStoreUserDefinedCameraFeature::upName()
{
    return "Up";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Camera* RicStoreUserDefinedCameraFeature::activeCamera()
{
    if ( auto activeView = RiaApplication::instance()->activeReservoirView() )
    {
        if ( activeView->viewer() && activeView->viewer()->mainCamera() )
        {
            return activeView->viewer()->mainCamera();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicStoreUserDefinedCameraFeature::isCommandEnabled() const
{
    return activeCamera() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStoreUserDefinedCameraFeature::onActionTriggered( bool isChecked )
{
    if ( auto camera = activeCamera() )
    {
        QSettings settings;
        settings.beginGroup( RicStoreUserDefinedCameraFeature::groupName() );

        cvf::Vec3d eye;
        cvf::Vec3d vrp;
        cvf::Vec3d up;

        camera->toLookAt( &eye, &vrp, &up );

        QVariant eyeVariant = caf::PdmValueFieldSpecialization<cvf::Vec3d>::convert( eye );
        settings.setValue( RicStoreUserDefinedCameraFeature::eyeName(), eyeVariant );

        QVariant vrpVariant = caf::PdmValueFieldSpecialization<cvf::Vec3d>::convert( vrp );
        settings.setValue( RicStoreUserDefinedCameraFeature::viewReferencePointName(), vrpVariant );

        QVariant upVariant = caf::PdmValueFieldSpecialization<cvf::Vec3d>::convert( up );
        settings.setValue( RicStoreUserDefinedCameraFeature::upName(), upVariant );

        RiuMainWindow::instance()->refreshViewActions();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStoreUserDefinedCameraFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Store User Defined View" );
    actionToSetup->setIcon( QIcon( ":/user-defined-view.svg" ) );
}
