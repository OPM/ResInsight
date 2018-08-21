/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicDeleteOptionItemFeature.h"

#include "RicCreateMultipleFracturesOptionItemUi.h"
#include "RicCreateMultipleFracturesUi.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteOptionItemFeature, "RicDeleteOptionItemFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteOptionItemFeature::onActionTriggered(bool isChecked)
{
    std::vector<RicCreateMultipleFracturesOptionItemUi*> optionItems;
    caf::SelectionManager::instance()->objectsByType(&optionItems, caf::SelectionManager::FIRST_LEVEL);

    if (!optionItems.empty())
    {
        RiuCreateMultipleFractionsUi* multipleFractionUi = nullptr;
        optionItems[0]->firstAncestorOrThisOfTypeAsserted(multipleFractionUi);

        for (auto optionItem : optionItems)
        {
            multipleFractionUi->deleteOptionItem(optionItem);
        }

        multipleFractionUi->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteOptionItemFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Option Items");
    // actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteOptionItemFeature::isCommandEnabled()
{
    std::vector<RicCreateMultipleFracturesOptionItemUi*> optionItems;
    caf::SelectionManager::instance()->objectsByType(&optionItems, caf::SelectionManager::FIRST_LEVEL);

    return !optionItems.empty();
}
