/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RicNewFaultReactModelingFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"
#include "RiuViewer.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultReactivationModel.h"
#include "RimFaultReactivationModelCollection.h"

#include "RigFault.h"
#include "RigMainGrid.h"

#include "cafCmdExecCommandManager.h"
#include "cafDisplayCoordTransform.h"

#include "cvfCamera.h"
#include "cvfStructGrid.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicNewFaultReactModelingFeature, "RicNewFaultReactModelingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFaultReactModelingFeature::isCommandEnabled() const
{
    return RiaPreferencesGeoMech::current()->validateFRMSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactModelingFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();

    if ( !userData.isNull() && userData.type() == QVariant::List )
    {
        Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
        if ( !view ) return;
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );
        if ( !eclView ) return;

        QVariantList list = userData.toList();
        CAF_ASSERT( list.size() == 2 );

        size_t currentCellIndex = static_cast<size_t>( list[0].toULongLong() );
        int    currentFaceIndex = list[1].toInt();

        auto face = cvf::StructGridInterface::FaceType( currentFaceIndex );

        const RigFault* fault = eclView->mainGrid()->findFaultFromCellIndexAndCellFace( currentCellIndex, face );
        if ( fault )
        {
            QString faultName = fault->name();

            RimFaultInView* rimFault = eclView->faultCollection()->findFaultByName( faultName );
            if ( rimFault )
            {
                RigCell cell = eclView->mainGrid()->cell( currentCellIndex );

                auto normal = cell.faceNormalWithAreaLength( face );
                normal.z()  = normal.z() / eclView->scaleZ() / eclView->scaleZ();
                normal.normalize();
                normal *= eclView->ownerCase()->characteristicCellSize();
                normal *= 3;

                if ( !eclView->mainGrid()->isFaceNormalsOutwards() ) normal = normal * -1.0;

                cvf::Vec3d target1 = cell.faceCenter( face );
                cvf::Vec3d target2 = target1 + normal;

                const QString defaultDirName = "FAULT_REACTIVATION_MODELING";

                // get base directory for our work, should be a new, empty folder somewhere
                QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( defaultDirName );

                QString baseDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Working Directory" ), defaultDir );
                if ( baseDir.isNull() || baseDir.isEmpty() ) return;

                RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirName, baseDir );

                QString errMsg;
                auto    model =
                    eclView->faultReactivationModelCollection()->addNewModel( rimFault, currentCellIndex, face, target1, target2, baseDir, errMsg );
                if ( model != nullptr )
                {
                    model->updateTimeSteps();
                    view->updateAllRequiredEditors();
                    Riu3DMainWindowTools::selectAsCurrentItem( model );
                }
                else
                {
                    QMessageBox::critical( nullptr, "Fault Reactivation Modeling", errMsg );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFaultReactModelingFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
}
