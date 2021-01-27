/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RicNewWellPathLateralAtDepthFeature.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "RigWellPath.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimModeledWellPathLateral.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGroup.h"
#include "RimWellPathLateralGeometryDef.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <cmath>

CAF_CMD_SOURCE_INIT( RicNewWellPathLateralAtDepthFeature, "RicNewWellPathLateralAtDepthFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathLateralAtDepthFeature::isCommandEnabled()
{
    if ( wellPathSelectionItem() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathLateralAtDepthFeature::onActionTriggered( bool isChecked )
{
    RiuWellPathSelectionItem* wellPathSelItem = wellPathSelectionItem();
    CVF_ASSERT( wellPathSelItem );

    RimWellPath* wellPath = wellPathSelItem->m_wellpath;
    CVF_ASSERT( wellPath );
    RimWellPathGroup* wellPathGroup = nullptr;
    wellPath->firstAncestorOrThisOfType( wellPathGroup );

    RimProject* project = RimProject::current();
    if ( project && RimProject::current()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();

        if ( wellPathCollection )
        {
            auto newModeledWellPath = new RimModeledWellPathLateral();

            auto [pointVector, measuredDepths] =
                wellPath->wellPathGeometry()->clippedPointSubset( wellPath->wellPathGeometry()->measuredDepths().front(),
                                                                  wellPathSelItem->m_measuredDepth );
            if ( pointVector.size() < 2u ) return;

            newModeledWellPath->geometryDefinition()->setParentGeometry( wellPath->wellPathGeometry() );
            newModeledWellPath->geometryDefinition()->setMdAtConnection( wellPathSelItem->m_measuredDepth );
            newModeledWellPath->geometryDefinition()->createTargetAtConnectionPoint(
                pointVector[pointVector.size() - 1u] - pointVector[pointVector.size() - 2u] );

            newModeledWellPath->geometryDefinition()->enableTargetPointPicking( true );
            newModeledWellPath->createWellPathGeometry();
            if ( wellPathGroup )
            {
                wellPathGroup->addChildWellPath( newModeledWellPath );
            }
            else
            {
                wellPathCollection->addWellPath( newModeledWellPath, false );
                wellPathCollection->groupWellPaths( { wellPath, newModeledWellPath } );
            }
            newModeledWellPath->firstAncestorOrThisOfTypeAsserted( wellPathGroup );
            wellPathGroup->updateAllRequiredEditors();
            project->scheduleCreateDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath->geometryDefinition() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathLateralAtDepthFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Path Lateral at this Depth" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellPathSelectionItem* RicNewWellPathLateralAtDepthFeature::wellPathSelectionItem()
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>( selItem );

    return wellPathItem;
}
