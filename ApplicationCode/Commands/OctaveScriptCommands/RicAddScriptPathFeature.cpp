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

#include "RicAddScriptPathFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicRefreshScriptsFeature.h"
#include "RicScriptFeatureImpl.h"

#include "RimScriptCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicAddScriptPathFeature, "RicAddScriptPathFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddScriptPathFeature::isCommandEnabled()
{
    std::vector<RimScriptCollection*> selection = RicScriptFeatureImpl::selectedScriptCollections();
    return selection.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddScriptPathFeature::onActionTriggered(bool isChecked)
{
    QString selectedFolder = QFileDialog::getExistingDirectory(Riu3DMainWindowTools::mainWindowWidget(), "Select script folder");
    if (!selectedFolder.isEmpty())
    {
        QString filePathString = RiaApplication::instance()->preferences()->scriptDirectories();

        QChar separator(';');
        if (!filePathString.isEmpty() && !filePathString.endsWith(separator, Qt::CaseInsensitive))
        {
            filePathString += separator;
        }

        filePathString += selectedFolder;

        RiaApplication::instance()->preferences()->scriptDirectories = filePathString;
        RiaApplication::instance()->applyPreferences();

        RicRefreshScriptsFeature::refreshScriptFolders();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddScriptPathFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Add Script Path");
}
