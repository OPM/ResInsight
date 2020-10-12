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

#include "RicDeleteCustomObjectiveFunctionWeightFeature.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionWeight.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteCustomObjectiveFunctionWeightFeature, "RicDeleteCustomObjectiveFunctionWeightFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteCustomObjectiveFunctionWeightFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomObjectiveFunctionWeightFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    std::vector<RimCustomObjectiveFunction*> func;
    selObj->descendantsIncludingThisOfType( func );
    std::vector<RimCustomObjectiveFunctionWeight*> weight;
    selObj->descendantsIncludingThisOfType( weight );

    if ( func.size() == 1 && weight.size() == 1 )
    {
        func[0]->removeWeight( weight[0] );
        func[0]->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( func.front() );
    }

    selObj->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomObjectiveFunctionWeightFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
