/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RicRunCommandFileFeature.h"

#include "RiaApplication.h"
#include "RicfCommandFileExecutor.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicRunCommandFileFeature, "RicRunCommandFileFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunCommandFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunCommandFileFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("COMMAND_FILE");

    QString fileName = QFileDialog::getOpenFileName(nullptr, "Open ResInsight Command File", defaultDir, "ResInsight Command File (*.txt);;All files(*.*)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);

            QString applicationPath = QDir::currentPath();

            QFileInfo fi(fileName);
            QDir::setCurrent(fi.absolutePath());

            RicfCommandFileExecutor::instance()->executeCommands(in);

            QDir::setCurrent(applicationPath);
        
            app->setLastUsedDialogDirectory("COMMAND_FILE", QFileInfo(fileName).absolutePath());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunCommandFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Run Command File");
}
