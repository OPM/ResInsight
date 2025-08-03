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
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"

#include "Riu3DMainWindowTools.h"

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
    duplicateWellPath( sourceWellPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate Well Path" );
    actionToSetup->setIcon( QIcon( ":/caf/duplicate.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicDuplicateWellPathFeature::duplicateWellPath( RimWellPath* wellPath )
{
    if ( wellPath == nullptr ) return nullptr;

    RimProject*            project            = RimProject::current();
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( project && wellPathCollection )
    {
        auto newModeledWellPath = wellPath->copyObject<RimModeledWellPath>();
        if ( newModeledWellPath == nullptr ) return nullptr;

        size_t modelledWellpathCount = wellPathCollection->modelledWellPathCount();
        newModeledWellPath->setName( "Well-" + QString::number( modelledWellpathCount + 1 ) );
        newModeledWellPath->setWellPathColor( RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( modelledWellpathCount ) );

        wellPathCollection->addWellPaths( { newModeledWellPath } );
        newModeledWellPath->createWellPathGeometry();

        project->scheduleCreateDisplayModelAndRedrawAllViews();

        Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath->geometryDefinition() );

        return newModeledWellPath;
    }

    return nullptr;
}