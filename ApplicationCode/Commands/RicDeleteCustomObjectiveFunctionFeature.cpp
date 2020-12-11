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

#include "RicDeleteCustomObjectiveFunctionFeature.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEnsembleCurveSet.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteCustomObjectiveFunctionFeature, "RicDeleteCustomObjectiveFunctionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteCustomObjectiveFunctionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomObjectiveFunctionFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    std::vector<RimCustomObjectiveFunction*> func;
    selObj->allAncestorsOrThisOfType( func );
    std::vector<RimCustomObjectiveFunctionCollection*> coll;
    selObj->allAncestorsOrThisOfType( coll );

    if ( func.size() == 1 && coll.size() == 1 )
    {
        coll[0]->removeObjectiveFunction( func[0] );
        coll[0]->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( coll.front() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteCustomObjectiveFunctionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete" );
    actionToSetup->setShortcut( QKeySequence( QKeySequence::Delete ) );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
