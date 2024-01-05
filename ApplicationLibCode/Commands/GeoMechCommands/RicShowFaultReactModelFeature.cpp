/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023    Equinor ASA
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

#include "RicShowFaultReactModelFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RifFaultReactivationModelExporter.h"

#include "RimFaultReactivationModel.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicShowFaultReactModelFeature, "RicShowFaultReactModelFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowFaultReactModelFeature::isCommandEnabled() const
{
    return RiaPreferencesGeoMech::current()->validateFRMSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowFaultReactModelFeature::onActionTriggered( bool isChecked )
{
    RimFaultReactivationModel* model = dynamic_cast<RimFaultReactivationModel*>( caf::SelectionManager::instance()->selectedItem() );
    if ( model == nullptr ) return;

    const QString frmTitle( "Fault Reactivation Modeling" );

    auto exportFile        = QString::fromStdString( model->inputFilename() );
    auto [result, errText] = RifFaultReactivationModelExporter::exportToFile( *model );
    if ( !result )
    {
        QString outErrorText =
            QString( "Failed to export INP model to file %1.\n\n%2" ).arg( exportFile ).arg( QString::fromStdString( errText ) );
        QMessageBox::critical( nullptr, frmTitle, outErrorText );
        return;
    }

    for ( auto gCase : RimProject::current()->geoMechCases() )
    {
        if ( exportFile == gCase->gridFileName() )
        {
            gCase->reloadDataAndUpdate();
            auto& views = gCase->geoMechViews();
            if ( !views.empty() )
            {
                Riu3DMainWindowTools::selectAsCurrentItem( views[0] );
            }
            else
            {
                Riu3DMainWindowTools::selectAsCurrentItem( gCase );
            }
            return;
        }
    }

    RiaApplication* app = RiaApplication::instance();
    if ( !app->openOdbCaseFromFile( exportFile ) )
    {
        QMessageBox::critical( nullptr,
                               frmTitle,
                               "Failed to load INP model from file \"" + exportFile + "\". Check log window for additional information." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowFaultReactModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Show Model in 3D View..." );
}
