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

#include "RicPasteIntersectionsFeature.h"

#include "RicPasteFeatureImpl.h"

#include "RimBoxIntersection.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteIntersectionsFeature, "RicPasteIntersectionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteIntersectionsFeature::isCommandEnabled()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimExtrudedCurveIntersection>> intersectionObjects;
    objectGroup.objectsByType( &intersectionObjects );

    std::vector<caf::PdmPointer<RimBoxIntersection>> intersectionBoxObjects;
    objectGroup.objectsByType( &intersectionBoxObjects );

    if ( intersectionObjects.empty() && intersectionBoxObjects.empty() )
    {
        return false;
    }

    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    if ( findIntersectionCollection( destinationObject ) )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteIntersectionsFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimIntersectionCollection* intersectionCollection =
        RicPasteIntersectionsFeature::findIntersectionCollection( destinationObject );

    CAF_ASSERT( intersectionCollection );

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    if ( objectGroup.objects.size() == 0 ) return;

    std::vector<caf::PdmPointer<RimExtrudedCurveIntersection>> intersectionObjects;
    objectGroup.objectsByType( &intersectionObjects );

    for ( size_t i = 0; i < intersectionObjects.size(); i++ )
    {
        RimExtrudedCurveIntersection* intersection = dynamic_cast<RimExtrudedCurveIntersection*>(
            intersectionObjects[i]->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        QString nameOfCopy = QString( "Copy of " ) + intersection->name();
        intersection->setName( nameOfCopy );

        if ( i == intersectionObjects.size() - 1 )
        {
            intersectionCollection->appendIntersectionAndUpdate( intersection, false );
        }
        else
        {
            intersectionCollection->appendIntersectionNoUpdate( intersection );
        }
    }

    std::vector<caf::PdmPointer<RimBoxIntersection>> intersectionBoxObjects;
    objectGroup.objectsByType( &intersectionBoxObjects );

    for ( size_t i = 0; i < intersectionBoxObjects.size(); i++ )
    {
        RimBoxIntersection* intersectionBox = dynamic_cast<RimBoxIntersection*>(
            intersectionBoxObjects[i]->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        QString nameOfCopy = QString( "Copy of " ) + intersectionBox->name();
        intersectionBox->setName( nameOfCopy );

        if ( i == intersectionBoxObjects.size() - 1 )
        {
            intersectionCollection->appendIntersectionBoxAndUpdate( intersectionBox );
        }
        else
        {
            intersectionCollection->appendIntersectionBoxNoUpdate( intersectionBox );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteIntersectionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste (Intersections)" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RicPasteIntersectionsFeature::findIntersectionCollection( caf::PdmObjectHandle* objectHandle )
{
    RimIntersectionCollection* intersectionCollection = dynamic_cast<RimIntersectionCollection*>( objectHandle );
    if ( intersectionCollection )
    {
        return intersectionCollection;
    }

    RimExtrudedCurveIntersection* intersection = dynamic_cast<RimExtrudedCurveIntersection*>( objectHandle );
    if ( intersection )
    {
        intersection->firstAncestorOrThisOfType( intersectionCollection );
        return intersectionCollection;
    }

    RimBoxIntersection* intersectionBox = dynamic_cast<RimBoxIntersection*>( objectHandle );
    if ( intersectionBox )
    {
        intersectionBox->firstAncestorOrThisOfType( intersectionCollection );
        return intersectionCollection;
    }

    return nullptr;
}
