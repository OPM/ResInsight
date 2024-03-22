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

#include "RicPasteCellFiltersFeature.h"

#include "RicPasteFeatureImpl.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilter.h"
#include "RimCellFilterCollection.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmPointer.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicPasteCellFiltersFeature, "RicPasteCellFiltersFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteCellFiltersFeature::isCommandEnabled() const
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    std::vector<caf::PdmPointer<RimCellFilter>> typedObjects;
    objectGroup.objectsByType( &typedObjects );
    if ( typedObjects.empty() ) return false;

    return dynamic_cast<RimCellFilterCollection*>( caf::SelectionManager::instance()->selectedItem() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteCellFiltersFeature::onActionTriggered( bool isChecked )
{
    auto cellFilterCollection = dynamic_cast<RimCellFilterCollection*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !cellFilterCollection ) return;

    auto view = cellFilterCollection->firstAncestorOfType<Rim3dView>();
    if ( !view ) return;

    auto eclipseCase = view->ownerCase();
    if ( !eclipseCase ) return;

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs( &objectGroup );

    for ( auto obj : objectGroup.objects )
    {
        auto duplicatedObject =
            dynamic_cast<RimCellFilter*>( obj->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        if ( duplicatedObject )
        {
            cellFilterCollection->addFilterAndNotifyChanges( duplicatedObject, eclipseCase );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteCellFiltersFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Paste Filter" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteCellFiltersFeature::pasteGeometryCellFilters( RimCellFilterCollection* cellFilterCollection )
{
    if ( !cellFilterCollection ) return;
}
