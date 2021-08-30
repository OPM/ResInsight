/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewIntersectionBandFeature.h"

/*
#include "RiaApplication.h"

#include "RimBoxIntersection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafCmdExecCommandManager.h"

#include "cvfAssert.h"
*/
#include "cafSelectionManager.h"

#include "RimCurveIntersectionBand.h"
#include "RimExtrudedCurveIntersection.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewIntersectionBandFeature, "RicNewIntersectionBandFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewIntersectionBandFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionBandFeature::onActionTriggered( bool isChecked )
{
    auto container = caf::SelectionManager::instance()->selectedItemOfType<RimExtrudedCurveIntersection>();
    if ( container )
    {
        auto object = container->appendNewIntersectionBand();
        container->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionBandFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
    actionToSetup->setText( "Append New Intersection Band" );
}
