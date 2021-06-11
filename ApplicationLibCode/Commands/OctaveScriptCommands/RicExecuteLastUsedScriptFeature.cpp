/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RicExecuteLastUsedScriptFeature.h"

#include "RiaApplication.h"

#include "RicExecuteScriptFeature.h"
#include "RimCalcScript.h"
#include "RimProject.h"
#include "RimScriptCollection.h"

#include "cafCmdFeatureManager.h"

#include <QAction>
#include <QFileInfo>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicExecuteLastUsedScriptFeature, "RicExecuteLastUsedScriptFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExecuteLastUsedScriptFeature::lastUsedScriptPathKey()
{
    return "lastUsedScriptPath";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExecuteLastUsedScriptFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteLastUsedScriptFeature::onActionTriggered( bool isChecked )
{
    QSettings settings;

    QString lastUsedScript = settings.value( lastUsedScriptPathKey() ).toString();
    if ( !lastUsedScript.isEmpty() )
    {
        RimScriptCollection* rootScriptCollection = RiaApplication::instance()->project()->scriptCollection();

        std::vector<RimCalcScript*> scripts;
        rootScriptCollection->descendantsIncludingThisOfType( scripts );
        for ( auto c : scripts )
        {
            if ( c->absoluteFileName() == lastUsedScript )
            {
                RicExecuteScriptFeature::executeScript( c );
                return;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExecuteLastUsedScriptFeature::setupActionLook( QAction* actionToSetup )
{
    QString actionText = "Execute Last Used Script";

    QSettings settings;
    QString   lastUsedScript = settings.value( lastUsedScriptPathKey() ).toString();
    if ( !lastUsedScript.isEmpty() )
    {
        QFileInfo fi( lastUsedScript );

        QString fileName = fi.fileName();
        if ( !fileName.isEmpty() )
        {
            actionText = "Execute " + fileName;
        }
    }

    actionToSetup->setText( actionText );
}
