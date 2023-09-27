/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewOctaveScriptFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "ApplicationCommands/RicOpenInTextEditorFeature.h"
#include "RicRefreshScriptsFeature.h"
#include "RicScriptFeatureImpl.h"

#include "RimCalcScript.h"
#include "RimScriptCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuTools.h"

#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>

CAF_CMD_SOURCE_INIT( RicNewOctaveScriptFeature, "RicNewOctaveScriptFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewOctaveScriptFeature::isCommandEnabled() const
{
    std::vector<RimScriptCollection*> calcScriptCollections = RicScriptFeatureImpl::selectedScriptCollections();
    if ( calcScriptCollections.empty() ) return false;
    return !calcScriptCollections.front()->directory().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOctaveScriptFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCalcScript*>       calcScripts           = RicScriptFeatureImpl::selectedScripts();
    std::vector<RimScriptCollection*> calcScriptCollections = RicScriptFeatureImpl::selectedScriptCollections();

    RimCalcScript*       calcScript = !calcScripts.empty() ? calcScripts[0] : nullptr;
    RimScriptCollection* scriptColl = !calcScriptCollections.empty() ? calcScriptCollections[0] : nullptr;

    QString fullPathNewScript;

    if ( calcScript )
    {
        QFileInfo existingScriptFileInfo( calcScript->absoluteFileName() );
        fullPathNewScript = existingScriptFileInfo.absolutePath();
    }
    else if ( scriptColl )
    {
        fullPathNewScript = scriptColl->directory();
    }
    else
    {
        return;
    }

    QString fullPathFilenameNewScript;

    fullPathFilenameNewScript = fullPathNewScript + "/untitled.m";
    int num                   = 1;
    while ( caf::Utils::fileExists( fullPathFilenameNewScript ) )
    {
        fullPathFilenameNewScript = fullPathNewScript + "/untitled" + QString::number( num ) + ".m";
        num++;
    }

    bool ok;
    fullPathFilenameNewScript = QInputDialog::getText( nullptr,
                                                       "Specify new script file",
                                                       "File name",
                                                       QLineEdit::Normal,
                                                       fullPathFilenameNewScript,
                                                       &ok,
                                                       RiuTools::defaultDialogFlags() );

    if ( ok && !fullPathFilenameNewScript.isEmpty() )
    {
        QFile file( fullPathFilenameNewScript );
        if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(),
                                           "Script editor",
                                           "Failed to create file\n" + fullPathFilenameNewScript );

            return;
        }

        RicRefreshScriptsFeature::refreshScriptFolders();

        if ( calcScript )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( calcScript );
        }

        RicOpenInTextEditorFeature::openFileInTextEditor( fullPathFilenameNewScript, this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOctaveScriptFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Octave Script" );
}
