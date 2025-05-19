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

#include "RicDeleteOsduTokenFeature.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaOsduDefines.h"

#include <QFile>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicDeleteOsduTokenFeature, "RicDeleteOsduTokenFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteOsduTokenFeature::deleteUserToken()
{
    const auto filename = RiaOsduDefines::tokenPath();
    if ( !QFile::exists( filename ) ) return;

    auto parent = RiaGuiApplication::activeWindow();

    QString question = "Do you want to delete your OSDU token file?";
    auto    reply    = QMessageBox::question( parent, "Delete token?", question, QMessageBox::Yes, QMessageBox::No );
    if ( reply != QMessageBox::Yes ) return;

    QFile::remove( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteOsduTokenFeature::onActionTriggered( bool isChecked )
{
    deleteUserToken();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteOsduTokenFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete OSDU Token" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/Cloud.svg" ) );
}
