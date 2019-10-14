/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSaveProjectAsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RicSaveProjectFeature.h"
#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QMessageBox>

RICF_SOURCE_INIT( RicSaveProjectAsFeature, "RicSaveProjectAsFeature", "saveProjectAs" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSaveProjectAsFeature::RicSaveProjectAsFeature()
{
    RICF_InitFieldNoDefault( &m_filePath, "filePath", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicSaveProjectAsFeature::execute()
{
    this->disableModelChangeContribution();
    QString errorMessage;
    if ( !RiaApplication::instance()->saveProjectAs( m_filePath(), &errorMessage ) )
    {
        return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, errorMessage );
    }
    return RicfCommandResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveProjectAsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectAsFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( app )
    {
        m_filePath = app->promptForProjectSaveAsFileName();
        if ( m_filePath().isEmpty() )
        {
            return;
        }
        auto response = execute();

        if ( response.status() != RicfCommandResponse::COMMAND_OK )
        {
            QString displayMessage = response.messages().join( "\n" );
            if ( RiaGuiApplication::isRunning() )
            {
                QMessageBox::warning( nullptr, "Error when saving project file", displayMessage );
            }
            RiaLogging::error( displayMessage );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectAsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save Project &As" );
    actionToSetup->setIcon( QIcon( ":/Save.png" ) );
    actionToSetup->setShortcuts( QKeySequence::SaveAs );
}
