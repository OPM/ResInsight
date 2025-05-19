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

#include "RicDeleteSumoTokenFeature.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "Tools/Cloud/RiaSumoDefines.h"

#include <QFile>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicDeleteSumoTokenFeature, "RicDeleteSumoTokenFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSumoTokenFeature::deleteUserToken()
{
    const auto filename = RiaSumoDefines::tokenPath();
    if ( !QFile::exists( filename ) ) return;

    auto parent = RiaGuiApplication::activeWindow();

    QString question = "Do you want to delete your SUMO token file?";
    auto    reply    = QMessageBox::question( parent, "Delete token?", question, QMessageBox::Yes, QMessageBox::No );
    if ( reply != QMessageBox::Yes ) return;

    QFile::remove( filename );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSumoTokenFeature::onActionTriggered( bool isChecked )
{
    deleteUserToken();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSumoTokenFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete SUMO Token" + RiaDefines::betaFeaturePostfix() );
    actionToSetup->setIcon( QIcon( ":/Cloud.svg" ) );
}
