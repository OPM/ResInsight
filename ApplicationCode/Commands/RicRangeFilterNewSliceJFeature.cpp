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

#include "RicRangeFilterNewSliceJFeature.h"

#include "RicRangeFilterFeatureImpl.h"
#include "RicRangeFilterNewExec.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRangeFilterNewSliceJFeature, "RicRangeFilterNewSliceJFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRangeFilterNewSliceJFeature::isCommandEnabled()
{
    return RicRangeFilterFeatureImpl::isRangeFilterCommandAvailable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewSliceJFeature::onActionTriggered( bool isChecked )
{
    RicRangeFilterNewExec* filterExec = RicRangeFilterFeatureImpl::createRangeFilterExecCommand();
    filterExec->m_jSlice              = true;

    caf::CmdExecCommandManager::instance()->processExecuteCommand( filterExec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRangeFilterNewSliceJFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New J-slice range filter" );
}
