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

#include "RicDeleteScriptPathFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicRefreshScriptsFeature.h"
#include "RicScriptFeatureImpl.h"

#include "RimScriptCollection.h"
#include "RiuMainWindow.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteScriptPathFeature, "RicDeleteScriptPathFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteScriptPathFeature::isCommandEnabled()
{
    std::vector<RimScriptCollection*> selection = RicScriptFeatureImpl::selectedScriptCollections();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteScriptPathFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimScriptCollection*> calcScriptCollections = RicScriptFeatureImpl::selectedScriptCollections();
    RimScriptCollection* scriptCollection = calcScriptCollections.size() > 0 ? calcScriptCollections[0] : NULL;
    if (scriptCollection)
    {
        QString toBeRemoved = scriptCollection->directory;

        QString originalFilePathString = RiaApplication::instance()->preferences()->scriptDirectories();
        QString filePathString = originalFilePathString.remove(toBeRemoved);

        // Remove duplicate separators
        QChar separator(';');
        QString regExpString = QString("%1{1,5}").arg(separator);
        filePathString.replace(QRegExp(regExpString), separator);

        // Remove separator at end
        if (filePathString.endsWith(separator))
        {
            filePathString = filePathString.left(filePathString.size() - 1);
        }

        RiaApplication::instance()->preferences()->scriptDirectories = filePathString;
        RiaApplication::instance()->applyPreferences();

        RicRefreshScriptsFeature::refreshScriptFolders();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteScriptPathFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Script Path");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
