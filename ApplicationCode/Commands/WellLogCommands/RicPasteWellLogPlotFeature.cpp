/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RicPasteWellLogPlotFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteWellLogPlotFeature, "RicPasteWellLogPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteWellLogPlotFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return false;

    RimWellLogPlotCollection* wellLogPlotCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType( wellLogPlotCollection );
    if ( !wellLogPlotCollection )
    {
        return false;
    }

    return RicPasteWellLogPlotFeature::plots().size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogPlotFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !destinationObject ) return;

    RimWellLogPlotCollection* wellLogPlotCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType( wellLogPlotCollection );
    if ( !wellLogPlotCollection )
    {
        return;
    }

    std::vector<caf::PdmPointer<RimWellLogPlot>> sourceObjects = RicPasteWellLogPlotFeature::plots();

    for ( size_t i = 0; i < sourceObjects.size(); i++ )
    {
        RimWellLogPlot* fileCurve = sourceObjects[i];
        if ( fileCurve )
        {
            RimWellLogPlot* newObject = dynamic_cast<RimWellLogPlot*>(
                fileCurve->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
            CVF_ASSERT( newObject );

            wellLogPlotCollection->addWellLogPlot( newObject );

            // Resolve references after object has been inserted into the project data model
            newObject->resolveReferencesRecursively();
            newObject->initAfterReadRecursively();

            QString customName = "Copy of " + newObject->nameConfig()->customName();
            newObject->nameConfig()->setCustomName( customName );

            newObject->loadDataAndUpdate();

            wellLogPlotCollection->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Well Log Plot" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimWellLogPlot>> RicPasteWellLogPlotFeature::plots()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimWellLogPlot>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    return typedObjects;
}
