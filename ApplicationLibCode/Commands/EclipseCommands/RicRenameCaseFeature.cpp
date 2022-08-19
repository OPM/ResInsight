/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicRenameCaseFeature.h"

#include "RimCase.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QInputDialog>

CAF_CMD_SOURCE_INIT( RicRenameCaseFeature, "RicRenameCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRenameCaseFeature::isCommandEnabled()
{
    auto rimCase = caf::SelectionManager::instance()->selectedItemOfType<RimCase>();
    return ( rimCase != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenameCaseFeature::onActionTriggered( bool isChecked )
{
    auto rimCase = caf::SelectionManager::instance()->selectedItemOfType<RimCase>();
    if ( !rimCase ) return;

    bool    ok;
    QString userDefinedName = QInputDialog::getText( nullptr,
                                                     "Rename Case",
                                                     "Enter new name:",
                                                     QLineEdit::Normal,
                                                     rimCase->caseUserDescription(),
                                                     &ok );

    if ( !ok ) return;

    rimCase->setCustomCaseName( userDefinedName.trimmed() );
    rimCase->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenameCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Rename" );
}
