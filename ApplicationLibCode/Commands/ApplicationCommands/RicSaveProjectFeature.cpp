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

#include "RicSaveProjectFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include <QAction>

RICF_SOURCE_INIT( RicSaveProjectFeature, "RicSaveProjectFeature", "saveProject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSaveProjectFeature::RicSaveProjectFeature()
{
    CAF_PDM_InitFieldNoDefault( &m_filePath, "filePath", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicSaveProjectFeature::execute()
{
    this->disableModelChangeContribution();

    bool    worked = false;
    QString errorMessage;
    if ( !m_filePath().isEmpty() )
    {
        worked = RiaApplication::instance()->saveProjectAs( m_filePath(), &errorMessage );
    }
    else
    {
        worked = RiaApplication::instance()->saveProject( &errorMessage );
    }

    if ( !worked )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }

    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveProjectFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectFeature::onActionTriggered( bool isChecked )
{
    RiaApplication*    app    = RiaApplication::instance();
    RiaGuiApplication* guiApp = dynamic_cast<RiaGuiApplication*>( app );

    if ( guiApp && !guiApp->isProjectSavedToDisc() )
    {
        m_filePath = guiApp->promptForProjectSaveAsFileName();

        if ( m_filePath().isEmpty() )
        {
            return;
        }
    }

    auto response = execute();
    if ( response.status() != caf::PdmScriptResponse::COMMAND_OK )
    {
        QString displayMessage = response.messages().join( "\n" );
        RiaLogging::errorInMessageBox( nullptr, "Error when saving project file", displayMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveProjectFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Save Project" );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );

    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Save );
}

//--------------------------------------------------------------------------------------------------
