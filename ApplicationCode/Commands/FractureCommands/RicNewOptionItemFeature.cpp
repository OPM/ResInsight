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

#include "RicNewOptionItemFeature.h"

#include "RiaApplication.h"

#include "RicCreateMultipleFracturesOptionItemUi.h"
#include "RicCreateMultipleFracturesUi.h"

#include "RimProject.h"
#include "RimDialogData.h"

#include "cafPdmChildArrayField.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewOptionItemFeature, "RicNewOptionItemFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewOptionItemFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOptionItemFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Option Item");
    // actionToSetup->setIcon(QIcon(":/Well.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOptionItemFeature::onActionTriggered(bool isChecked)
{
    RiuCreateMultipleFractionsUi* multipleFractionUi = RiaApplication::instance()->project()->dialogData()->multipleFractionsData();
    RicCreateMultipleFracturesOptionItemUi* selectedOptionItem = nullptr;

    {
        std::vector<RicCreateMultipleFracturesOptionItemUi*> optionItems;
        caf::SelectionManager::instance()->objectsByType(&optionItems, caf::SelectionManager::FIRST_LEVEL);
        if (!optionItems.empty())
        {
            selectedOptionItem = optionItems.front();
            selectedOptionItem->firstAncestorOrThisOfTypeAsserted(multipleFractionUi);
        }

        if (!selectedOptionItem && multipleFractionUi && !multipleFractionUi->options().empty())
        {
            selectedOptionItem = multipleFractionUi->options().back();
        }
    }

    if (multipleFractionUi)
    {
        auto newItem = new RicCreateMultipleFracturesOptionItemUi();
        if (selectedOptionItem)
        {
            newItem->setValues(selectedOptionItem->topKLayer(),
                               selectedOptionItem->baseKLayer(),
                               selectedOptionItem->fractureTemplate(),
                               selectedOptionItem->minimumSpacing());
        }
        multipleFractionUi->insertOptionItem(selectedOptionItem, newItem);
        multipleFractionUi->updateConnectedEditors();
    }
}
