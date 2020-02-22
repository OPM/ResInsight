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

#include "RicRangeFilterInsertFeature.h"

#include "RicRangeFilterFeatureImpl.h"
#include "RicRangeFilterInsertExec.h"

#include "RimCellRangeFilter.h"

#include "cafSelectionManager.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRangeFilterInsertFeature, "RicRangeFilterInsertFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterInsertFeature::isCommandEnabled()
{
    return RicRangeFilterFeatureImpl::isRangeFilterCommandAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterInsertFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCellRangeFilter*> selection             = selectedCellRangeFilters();
    RimCellRangeFilterCollection*    rangeFilterCollection = RicRangeFilterFeatureImpl::findRangeFilterCollection();

    RicRangeFilterInsertExec* filterExec = new RicRangeFilterInsertExec( rangeFilterCollection, selection[0] );
    caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterInsertFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Insert Range Filter" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCellRangeFilter*> RicRangeFilterInsertFeature::selectedCellRangeFilters()
{
    std::vector<RimCellRangeFilter*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    return selection;
}
