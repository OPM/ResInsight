/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicAddOpmInputPropertyFeature.h"

#include "RiaApplication.h"
#include "RimEclipseInputCaseOpm.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QStringList>

CAF_CMD_SOURCE_INIT(RicAddOpmInputPropertyFeature, "RicAddOpmInputPropertyFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddOpmInputPropertyFeature::isCommandEnabled()
{
    RimEclipseInputPropertyCollection* inputProp = selectedInputPropertyCollection();
    if (inputProp)
    {
        RimEclipseInputCaseOpm* inputCaseOpm = NULL;

        inputProp->firstAnchestorOrThisOfType(inputCaseOpm);
        if (inputCaseOpm)
        {
            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddOpmInputPropertyFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Select Eclipse Input Property Files", defaultDir, "All Files (*.* *)");

    if (fileNames.isEmpty()) return;

    // Remember the directory to next time
    defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setLastUsedDialogDirectory("INPUT_FILES", defaultDir);

    RimEclipseInputPropertyCollection* inputPropertyCollection = selectedInputPropertyCollection();
    if (inputPropertyCollection)
    {
        addEclipseInputProperty(fileNames, inputPropertyCollection);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddOpmInputPropertyFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Add Input Property");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection* RicAddOpmInputPropertyFeature::selectedInputPropertyCollection() const
{
    std::vector<RimEclipseInputPropertyCollection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddOpmInputPropertyFeature::addEclipseInputProperty(const QStringList& fileNames, RimEclipseInputPropertyCollection* inputPropertyCollection)
{
    CVF_ASSERT(inputPropertyCollection);

    RimEclipseInputCaseOpm* inputCaseOpm = NULL;

    inputPropertyCollection->firstAnchestorOrThisOfType(inputCaseOpm);
    if (inputCaseOpm)
    {
        inputCaseOpm->appendPropertiesFromStandaloneFiles(fileNames);
    }

    inputPropertyCollection->updateConnectedEditors();
}


