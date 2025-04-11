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

#include "RicApplyUserDefinedCameraFeature.h"
#include "RicStoreUserDefinedCameraFeature.h"

#include "RiaApplication.h"

#include "RimGridView.h"

#include "RiuViewer.h"

#include "cvfCamera.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicApplyUserDefinedCameraFeature, "RicApplyUserDefinedCameraFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicApplyUserDefinedCameraFeature::isCommandEnabled() const
{
    if ( !RicStoreUserDefinedCameraFeature::activeCamera() ) return false;

    cvf::Vec3d eye = cvf::Vec3d::UNDEFINED;
    cvf::Vec3d vrp = cvf::Vec3d::UNDEFINED;
    cvf::Vec3d up  = cvf::Vec3d::UNDEFINED;

    readCameraFromSettings( eye, vrp, up );
    return !( eye.isUndefined() || vrp.isUndefined() || up.isUndefined() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicApplyUserDefinedCameraFeature::onActionTriggered( bool isChecked )
{
    if ( auto camera = RicStoreUserDefinedCameraFeature::activeCamera() )
    {
        cvf::Vec3d eye = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d vrp = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d up  = cvf::Vec3d::UNDEFINED;

        readCameraFromSettings( eye, vrp, up );
        if ( eye.isUndefined() || vrp.isUndefined() || up.isUndefined() ) return;

        RicStoreUserDefinedCameraFeature::activeCamera()->setFromLookAt( eye, vrp, up );
        RiaApplication::instance()->activeReservoirView()->updateDisplayModelForCurrentTimeStepAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicApplyUserDefinedCameraFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "User Defined View" );
    actionToSetup->setToolTip( "User Defined View (Ctrl+Alt+U)" );
    actionToSetup->setIcon( QIcon( ":/user-defined-view.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Alt+U" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicApplyUserDefinedCameraFeature::readCameraFromSettings( cvf::Vec3d& eye, cvf::Vec3d& vrp, cvf::Vec3d& up )
{
    eye = cvf::Vec3d::UNDEFINED;
    vrp = cvf::Vec3d::UNDEFINED;
    up  = cvf::Vec3d::UNDEFINED;

    QSettings settings;
    settings.beginGroup( RicStoreUserDefinedCameraFeature::groupName() );

    QVariant eyeVariant = settings.value( RicStoreUserDefinedCameraFeature::eyeName() );
    QVariant vrpVariant = settings.value( RicStoreUserDefinedCameraFeature::viewReferencePointName() );
    QVariant upVariant  = settings.value( RicStoreUserDefinedCameraFeature::upName() );
    if ( eyeVariant.isNull() || vrpVariant.isNull() || upVariant.isNull() ) return;

    caf::PdmValueFieldSpecialization<cvf::Vec3d>::setFromVariant( eyeVariant, eye );
    caf::PdmValueFieldSpecialization<cvf::Vec3d>::setFromVariant( vrpVariant, vrp );
    caf::PdmValueFieldSpecialization<cvf::Vec3d>::setFromVariant( upVariant, up );
}
