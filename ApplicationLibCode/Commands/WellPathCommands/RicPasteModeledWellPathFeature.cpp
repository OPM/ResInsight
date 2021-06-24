/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RicPasteModeledWellPathFeature.h"

CAF_CMD_SOURCE_INIT( RicPasteModeledWellPathFeature, "RicPasteModeledWellPathFeature" );

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteModeledWellPathFeature::isCommandEnabled()
{
    if ( !modeledWellPaths().empty() ) return true;
    {
        std::vector<RimWellPathCollection*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        if ( objects.size() > 0 )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteModeledWellPathFeature::onActionTriggered( bool isChecked )
{
    if ( modeledWellPaths().empty() ) return;

    RimProject* proj = RimProject::current();

    if ( proj && proj->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = proj->activeOilField()->wellPathCollection();

        if ( wellPathCollection )
        {
            for ( auto souceWellPath : modeledWellPaths() )
            {
                RimModeledWellPath* newModeledWellPath = dynamic_cast<RimModeledWellPath*>(
                    souceWellPath->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

                QString name = souceWellPath->name() + " (copy)";
                newModeledWellPath->setName( name );

                wellPathCollection->addWellPath( newModeledWellPath, false );
                wellPathCollection->uiCapability()->updateConnectedEditors();

                proj->scheduleCreateDisplayModelAndRedrawAllViews();

                Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteModeledWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Well Path" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimModeledWellPath*> RicPasteModeledWellPathFeature::modeledWellPaths()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimModeledWellPath>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    std::vector<RimModeledWellPath*> wellPaths;
    for ( auto obj : typedObjects )
    {
        wellPaths.push_back( obj );
    }

    return wellPaths;
}
