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

#include "RicNewWellPathLateralFeature.h"

#include "RicNewWellPathLateralAtDepthFeature.h"

#include "RigWellPath.h"
#include "RimWellPath.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellPathLateralFeature, "RicNewWellPathLateralFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathLateralFeature::isCommandEnabled()
{
    RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>();

    return wellPath != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathLateralFeature::onActionTriggered( bool isChecked )
{
    RimWellPath* parentWellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>();
    if ( !parentWellPath ) return;

    double measuredDepth = 0.0;

    if ( parentWellPath->wellPathGeometry() && parentWellPath->wellPathGeometry()->measuredDepths().size() > 2 )
    {
        // Create new well close to the total depth of parent well
        double mdCandidate = parentWellPath->wellPathGeometry()->measuredDepths().back() - 100.0;
        measuredDepth      = std::max( 0.0, mdCandidate );
    }

    RicNewWellPathLateralAtDepthFeature::createLateralAtMeasuredDepth( parentWellPath, measuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathLateralFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Path Lateral" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
