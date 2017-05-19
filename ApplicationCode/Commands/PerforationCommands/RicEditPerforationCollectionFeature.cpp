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

#include "RicEditPerforationCollectionFeature.h"

#include "RiuEditPerforationCollectionWidget.h"

#include "RimPerforationCollection.h"
#include "RimWellPath.h"

#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicEditPerforationCollectionFeature, "RicEditPerforationCollectionFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditPerforationCollectionFeature::isCommandEnabled()
{
    return selectedWellPath() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditPerforationCollectionFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    RimWellPath* wellPath = selectedWellPath();
    
    if (wellPath == nullptr) return;

    RiuEditPerforationCollectionWidget dlg(nullptr, wellPath->perforationIntervalCollection());
    dlg.exec();
    wellPath->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditPerforationCollectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Perforations");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicEditPerforationCollectionFeature::selectedWellPath() const
{
    RimWellPath* wellPath = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(wellPath);
    }

    return wellPath;
}
