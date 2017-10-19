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

#include "RicReloadFormationNamesFeature.h"
#include "RimFormationNamesCollection.h"
#include "RimFormationNames.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicReloadFormationNamesFeature, "RicReloadFormationNamesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicReloadFormationNamesFeature::isCommandEnabled()
{
    std::vector<RimFormationNames*> selectedFormationNamesObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesObjs);
    std::vector<RimFormationNamesCollection*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    return (selectedFormationNamesObjs.size() > 0 || selectedFormationNamesCollObjs.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicReloadFormationNamesFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimFormationNamesCollection*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    if (selectedFormationNamesCollObjs.size())
    {
        selectedFormationNamesCollObjs[0]->readAllFormationNames();
        for(RimFormationNames* fnames: selectedFormationNamesCollObjs[0]->formationNamesList())
        {
            fnames->updateConnectedViews();
        }

        return;
    }

    std::vector<RimFormationNames*> selectedFormationNamesObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesObjs);
    for (RimFormationNames* fnames:  selectedFormationNamesObjs)
    {
        QString errorMessage;
        fnames->readFormationNamesFile(&errorMessage);
        if ( !errorMessage.isEmpty() )
        {
            QMessageBox::warning(nullptr, "Reload Formation Names", errorMessage);
        }
        
        fnames->updateConnectedViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicReloadFormationNamesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Reload");
    actionToSetup->setIcon(QIcon(":/Refresh-32.png"));
}
