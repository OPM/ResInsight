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

#include "RicExportInpFileFeature.h"

#include "RifFaultReactivationModelExporter.h"
#include "RimFaultReactivationModel.h"

#include "RigFaultReactivationModel.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicExportInpFileFeature, "RicExportInpFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportInpFileFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimFaultReactivationModel>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportInpFileFeature::onActionTriggered( bool isChecked )
{
    auto faultReactivationModel = caf::SelectionManager::instance()->selectedItemOfType<RimFaultReactivationModel>();
    if ( faultReactivationModel )
    {
        QString exportFile        = faultReactivationModel->baseDir() + "/faultreactivation.inp";
        auto [isOk, errorMessage] = RifFaultReactivationModelExporter::exportToFile( exportFile.toStdString(), *faultReactivationModel );
        if ( !isOk )
        {
            const QString frmTitle( "Fault Reactivation Modeling" );
            QString outErrorText =
                QString( "Failed to export INP model to file %1.\n\n%2" ).arg( exportFile ).arg( QString::fromStdString( errorMessage ) );
            QMessageBox::critical( nullptr, frmTitle, outErrorText );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportInpFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export INP..." );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}
