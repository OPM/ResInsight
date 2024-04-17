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

#include "RicPasteEclipseViewsFeature.h"

#include "RicPasteFeatureImpl.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteEclipseViewsFeature, "RicPasteEclipseViewsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteEclipseViewsFeature::isCommandEnabled() const
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimEclipseView>> typedObjects;
    objectGroup.objectsByType( &typedObjects );

    if ( typedObjects.empty() )
    {
        return false;
    }

    auto* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimIdenticalGridCaseGroup* gridCaseGroup = RicPasteFeatureImpl::findGridCaseGroup( destinationObject );
    if ( gridCaseGroup ) return false;

    RimEclipseCase* eclipseCase = RicPasteFeatureImpl::findEclipseCase( destinationObject );
    return eclipseCase != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseViewsFeature::onActionTriggered( bool isChecked )
{
    auto* destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimEclipseCase* eclipseCase = RicPasteFeatureImpl::findEclipseCase( destinationObject );
    assert( eclipseCase );

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    if ( objectGroup.objects.empty() ) return;

    std::vector<caf::PdmPointer<RimEclipseView>> eclipseViews;
    objectGroup.objectsByType( &eclipseViews );

    RimEclipseView* lastViewCopy = nullptr;

    // Add cases to case group
    for ( const auto& eclipseView : eclipseViews )
    {
        auto* rimReservoirView =
            dynamic_cast<RimEclipseView*>( eclipseView->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
        CVF_ASSERT( rimReservoirView );

        QString nameOfCopy = QString( "Copy of " ) + rimReservoirView->name();
        rimReservoirView->setName( nameOfCopy );

        if ( dynamic_cast<RimEclipseContourMapView*>( eclipseView.p() ) )
        {
            auto contourMapView = dynamic_cast<RimEclipseContourMapView*>( rimReservoirView );
            CVF_ASSERT( contourMapView );

            eclipseCase->contourMapCollection()->addView( contourMapView );
        }
        else
        {
            eclipseCase->reservoirViews().push_back( rimReservoirView );
        }

        rimReservoirView->setEclipseCase( eclipseCase );

        // Resolve references after reservoir view has been inserted into Rim structures
        // Intersections referencing a well path/ simulation well requires this
        rimReservoirView->resolveReferencesRecursively();
        rimReservoirView->initAfterReadRecursively();

        eclipseCase->intersectionViewCollection()->syncFromExistingIntersections( false );
        rimReservoirView->loadDataAndUpdate();

        caf::PdmDocument::updateUiIconStateRecursively( rimReservoirView );

        eclipseCase->updateConnectedEditors();
        lastViewCopy = rimReservoirView;
    }

    if ( lastViewCopy ) Riu3DMainWindowTools::selectAsCurrentItem( lastViewCopy );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseViewsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste (Eclipse Views)" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}
