/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicDuplicateWellPathFeature.h"

#include "RiaColorTables.h"

#include "Well/RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDuplicateWellPathFeature, "RicDuplicateWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDuplicateWellPathFeature::isCommandEnabled() const
{
    auto wellPath = caf::firstAncestorOfTypeFromSelectedObject<RimModeledWellPath>();
    return wellPath != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateWellPathFeature::onActionTriggered( bool isChecked )
{
    auto sourceWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimModeledWellPath>();
    if ( !sourceWellPath ) return;

    RimProject* project = RimProject::current();
    if ( project && RimProject::current()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            auto newModeledWellPath = new RimModeledWellPath();

            newModeledWellPath->setUnitSystem( project->commonUnitSystemForAllCases() );

            size_t modelledWellpathCount = wellPathCollection->modelledWellPathCount();

            newModeledWellPath->setName( "Well-" + QString::number( modelledWellpathCount + 1 ) );
            newModeledWellPath->setWellPathColor( RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( modelledWellpathCount ) );

            wellPathCollection->addWellPaths( { newModeledWellPath } );
            wellPathCollection->uiCapability()->updateConnectedEditors();

            if ( sourceWellPath->wellPathGeometry() && sourceWellPath->wellPathGeometry()->measuredDepths().size() > 2 )
            {
                auto destinationGeometryDef = newModeledWellPath->geometryDefinition();

                std::vector<cvf::Vec3d> targetCoordinates;

                for ( auto target : sourceWellPath->geometryDefinition()->activeWellTargets( false ) )
                {
                    targetCoordinates.push_back( target->targetPointXYZ() );
                }
                destinationGeometryDef->createAndInsertTargets( targetCoordinates );
                newModeledWellPath->createWellPathGeometry();
            }

            project->scheduleCreateDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath->geometryDefinition() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate Well Path" );
    actionToSetup->setIcon( QIcon( ":/caf/duplicate.svg" ) );
}
