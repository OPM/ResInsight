/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicNewPythonScriptFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

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

CAF_CMD_SOURCE_INIT( RicNewPythonScriptFeature, "RicNewPythonScriptFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPythonScriptFeature::isCommandEnabled()
{
    std::vector<RimCalcScript*>       calcScripts           = RicScriptFeatureImpl::selectedScripts();
    std::vector<RimScriptCollection*> calcScriptCollections = RicScriptFeatureImpl::selectedScriptCollections();
    if ( calcScripts.size() == 1u && calcScripts.front()->scriptType() == RimCalcScript::PYTHON )
    {
        return true;
    }
    else if ( calcScriptCollections.size() == 1u && !calcScriptCollections.front()->directory().isEmpty() )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPythonScriptFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCalcScript*>       calcScripts           = RicScriptFeatureImpl::selectedScripts();
    std::vector<RimScriptCollection*> calcScriptCollections = RicScriptFeatureImpl::selectedScriptCollections();

    RimCalcScript*       calcScript = calcScripts.size() > 0 ? calcScripts[0] : nullptr;
    RimScriptCollection* scriptColl = calcScriptCollections.size() > 0 ? calcScriptCollections[0] : nullptr;

    QString fullPathNewScript;

    if ( calcScript )
    {
        QFileInfo existingScriptFileInfo( calcScript->absoluteFileName() );
        fullPathNewScript = existingScriptFileInfo.absolutePath();
        calcScript->firstAncestorOrThisOfTypeAsserted( scriptColl );
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

    fullPathFilenameNewScript = fullPathNewScript + "/untitled.py";
    int num                   = 1;
    while ( caf::Utils::fileExists( fullPathFilenameNewScript ) )
    {
        fullPathFilenameNewScript = fullPathNewScript + "/untitled" + QString::number( num ) + ".py";
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
        else
        {
            QTextStream stream( &file );
            stream << "# Load ResInsight Processing Server Client Library\nimport rips\n# Connect to ResInsight "
                      "instance\nresinsight = rips.Instance.find()\n# Example code\nprint(\"ResInsight version: \" + "
                      "resinsight.version_string())\n";
        }

        scriptColl->readContentFromDisc();
        scriptColl->updateConnectedEditors();

        if ( calcScript )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( calcScript );
        }

        RiaApplication* app          = RiaApplication::instance();
        QString         scriptEditor = app->scriptEditorPath();
        if ( !scriptEditor.isEmpty() )
        {
            QStringList arguments;
            arguments << fullPathFilenameNewScript;

            QProcess* myProcess = new QProcess( this );
            myProcess->start( scriptEditor, arguments );

            if ( !myProcess->waitForStarted( 1000 ) )
            {
                RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(),
                                               "Script editor",
                                               "Failed to start script editor executable\n" + scriptEditor );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPythonScriptFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Python Script" );
}
