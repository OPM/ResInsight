/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewCustomObjectiveFunctionFeature.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimObjectiveFunctionTools.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewCustomObjectiveFunctionFeature, "RicNewCustomObjectiveFunctionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewCustomObjectiveFunctionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomObjectiveFunctionFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selObj ) return;

    std::vector<RimCustomObjectiveFunctionCollection*> coll;
    selObj->descendantsIncludingThisOfType( coll );

    if ( coll.size() == 1 )
    {
        RimCustomObjectiveFunction*       newFunc   = coll[0]->addObjectiveFunction();
        RimCustomObjectiveFunctionWeight* newWeight = RimObjectiveFunctionTools::addWeight( newFunc );
        coll[0]->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( newFunc );
        RiuPlotMainWindowTools::setExpanded( coll.front() );
        RiuPlotMainWindowTools::setExpanded( newFunc );
    }

    selObj->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewCustomObjectiveFunctionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Custom Objective Function" );
    actionToSetup->setIcon( QIcon( ":/ObjectiveFunction.svg" ) );
}
