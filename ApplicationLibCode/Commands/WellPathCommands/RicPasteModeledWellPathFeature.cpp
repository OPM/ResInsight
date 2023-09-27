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
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathTieIn.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteModeledWellPathFeature::isCommandEnabled() const
{
    if ( !modeledWellPathsFromClipboard().empty() ) return true;
    {
        std::vector<RimWellPathCollection*> objects;
        caf::SelectionManager::instance()->objectsByType( &objects );

        if ( !objects.empty() )
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
    if ( modeledWellPathsFromClipboard().empty() ) return;

    RimProject* proj = RimProject::current();

    if ( proj && proj->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            RimModeledWellPath* wellPathToSelect = nullptr;
            for ( auto sourceWellPath : modeledWellPathsFromClipboard() )
            {
                auto destinationWellPath = duplicateAndInitializeWellPath( sourceWellPath );

                wellPathToSelect = destinationWellPath;
            }

            RimTools::wellPathCollection()->rebuildWellPathNodes();

            wellPathCollection->uiCapability()->updateConnectedEditors();

            proj->scheduleCreateDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem( wellPathToSelect );
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
std::vector<RimModeledWellPath*> RicPasteModeledWellPathFeature::modeledWellPathsFromClipboard()
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteModeledWellPathFeature::duplicateLaterals( const RimModeledWellPath* source, RimModeledWellPath* destination )
{
    auto wpc = RimTools::wellPathCollection();

    auto sourceLaterals = wpc->connectedWellPathLaterals( source );

    destination->createWellPathGeometry();
    for ( auto lateral : sourceLaterals )
    {
        auto sourceLateral = dynamic_cast<RimModeledWellPath*>( lateral );
        if ( !sourceLateral ) continue;

        auto destinationLateral = duplicateAndInitializeWellPath( sourceLateral );

        destinationLateral->connectWellPaths( destination, sourceLateral->wellPathTieIn()->tieInMeasuredDepth() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath* RicPasteModeledWellPathFeature::duplicateAndInitializeWellPath( const RimModeledWellPath* sourceWellPath )
{
    if ( !sourceWellPath ) return nullptr;

    auto wpc = RimTools::wellPathCollection();

    auto* destinationWellPath = dynamic_cast<RimModeledWellPath*>(
        sourceWellPath->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

    QString name = sourceWellPath->name() + "(copy)";
    destinationWellPath->setName( name );

    wpc->addWellPath( destinationWellPath );

    // Resolve references, will connect to the fracture template
    destinationWellPath->resolveReferencesRecursively();

    std::vector<RimWellPathFracture*> wellPathFractures = destinationWellPath->descendantsIncludingThisOfType<RimWellPathFracture>();
    destinationWellPath->createWellPathGeometry();

    for ( auto fracture : wellPathFractures )
    {
        fracture->loadDataAndUpdate();
    }

    duplicateLaterals( sourceWellPath, destinationWellPath );

    return destinationWellPath;
}
