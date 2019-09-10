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

#include "RicRangeFilterNewFeature.h"

#include "RicRangeFilterFeatureImpl.h"
#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRangeFilterNewFeature, "RicRangeFilterNewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNewFeature::isCommandEnabled()
{
    return RicRangeFilterFeatureImpl::isRangeFilterCommandAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewFeature::onActionTriggered( bool isChecked )
{
    RicRangeFilterNewExec* filterExec = RicRangeFilterFeatureImpl::createRangeFilterExecCommand();

    caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Range.png" ) );
    actionToSetup->setText( "New Range Filter" );
}
