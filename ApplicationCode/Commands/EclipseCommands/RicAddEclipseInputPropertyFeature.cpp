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

#include "RicAddEclipseInputPropertyFeature.h"

#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseInputCase.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"
 
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>
#include <QStringList>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicAddEclipseInputPropertyFeature, "RicAddEclipseInputPropertyFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAddEclipseInputPropertyFeature::isCommandEnabled()
{
    return selectedInputPropertyCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddEclipseInputPropertyFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Select Eclipse Input Property Files", defaultDir, "All Files (*.* *)");

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
void RicAddEclipseInputPropertyFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Add Input Property");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection* RicAddEclipseInputPropertyFeature::selectedInputPropertyCollection() const
{
    std::vector<RimEclipseInputPropertyCollection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddEclipseInputPropertyFeature::addEclipseInputProperty(const QStringList& fileNames, RimEclipseInputPropertyCollection* inputPropertyCollection)
{
    CVF_ASSERT(inputPropertyCollection);

    RimEclipseInputCase* inputReservoir = dynamic_cast<RimEclipseInputCase*>(inputPropertyCollection->parentField()->ownerObject());
    CVF_ASSERT(inputReservoir);
    if (inputReservoir)
    {
        inputReservoir->openDataFileSet(fileNames);
    }

    inputPropertyCollection->updateConnectedEditors();
}


