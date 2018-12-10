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
#include "Riu3DMainWindowTools.h"
 
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
    RimEclipseInputPropertyCollection* inputPropertyCollection = selectedInputPropertyCollection();
    if (!inputPropertyCollection) return;

    QString casePath;
    {
        RimEclipseInputCase* inputReservoir = nullptr;
        inputPropertyCollection->firstAncestorOrThisOfTypeAsserted(inputReservoir);

        QFileInfo fi(inputReservoir->gridFileName());
        casePath = fi.absolutePath();
    }

    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectoryWithFallback("INPUT_FILES", casePath);
    QStringList fileNames = QFileDialog::getOpenFileNames(Riu3DMainWindowTools::mainWindowWidget(), "Select Eclipse Input Property Files", defaultDir, "All Files (*.* *)");

    if (fileNames.isEmpty()) return;

    // Remember the directory to next time
    defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setLastUsedDialogDirectory("INPUT_FILES", defaultDir);

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
    return caf::SelectionManager::instance()->selectedItemOfType<RimEclipseInputPropertyCollection>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAddEclipseInputPropertyFeature::addEclipseInputProperty(const QStringList& fileNames, RimEclipseInputPropertyCollection* inputPropertyCollection)
{
    CVF_ASSERT(inputPropertyCollection);

    RimEclipseInputCase* inputReservoir = nullptr;
    inputPropertyCollection->firstAncestorOrThisOfTypeAsserted(inputReservoir);
    inputReservoir->openDataFileSet(fileNames);

    inputPropertyCollection->updateConnectedEditors();
}


