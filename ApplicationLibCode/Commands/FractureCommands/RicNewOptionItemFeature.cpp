/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicCreateMultipleFracturesOptionItemUi.h"
#include "RicCreateMultipleFracturesUi.h"

#include "RimDialogData.h"
#include "RimProject.h"

#include "cafPdmChildArrayField.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewOptionItemFeature, "RicNewOptionItemFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOptionItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Option Item" );
    // actionToSetup->setIcon(QIcon(":/Well.svg"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewOptionItemFeature::onActionTriggered( bool isChecked )
{
    RiuCreateMultipleFractionsUi*           multipleFractionUi = RimProject::current()->dialogData()->multipleFractionsData();
    RicCreateMultipleFracturesOptionItemUi* selectedOptionItem = nullptr;

    {
        const auto optionItems =
            caf::SelectionManager::instance()->objectsByType<RicCreateMultipleFracturesOptionItemUi>( caf::SelectionManager::FIRST_LEVEL );
        if ( !optionItems.empty() )
        {
            selectedOptionItem = optionItems.front();
            multipleFractionUi = selectedOptionItem->firstAncestorOrThisOfTypeAsserted<RiuCreateMultipleFractionsUi>();
        }

        if ( !selectedOptionItem && multipleFractionUi && !multipleFractionUi->options().empty() )
        {
            selectedOptionItem = multipleFractionUi->options().back();
        }
    }

    if ( multipleFractionUi )
    {
        auto newItem = new RicCreateMultipleFracturesOptionItemUi();

        if ( selectedOptionItem )
        {
            newItem->setValues( selectedOptionItem->baseKLayer() + 1,
                                selectedOptionItem->baseKLayer() + 1,
                                selectedOptionItem->fractureTemplate(),
                                selectedOptionItem->minimumSpacing() );
        }
        multipleFractionUi->insertOptionItem( selectedOptionItem, newItem );
        multipleFractionUi->updateButtonsEnableState();
        multipleFractionUi->updateConnectedEditors();
    }
}
