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

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RiaResultNames.h"

#include "QuickAccess/RimQuickAccessCollection.h"
#include "RimCombinedFilter.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFilterInViewCollection.h"
#include "RimViewController.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilter*> RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilters()
{
    return caf::SelectionManager::instance()->objectsByType<RimEclipsePropertyFilter>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilterCollection*> RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections()
{
    return caf::SelectionManager::instance()->objectsByType<RimEclipsePropertyFilterCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RicEclipsePropertyFilterFeatureImpl::resolveTargetPropertyFilterCollection()
{
    auto direct = caf::SelectionManager::instance()->objectsByType<RimEclipsePropertyFilterCollection>();
    if ( !direct.empty() ) return direct.front();

    auto facades = caf::SelectionManager::instance()->objectsByType<RimFilterInViewCollection>();
    if ( !facades.empty() && facades.front()->propertyFilters() ) return facades.front()->propertyFilters();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::addPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection )
{
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFiltersField().push_back( propertyFilter );
    setDefaults( propertyFilter );

    RimQuickAccessCollection::instance()->addQuickAccessFields( propertyFilter );

    propertyFilterCollection->reservoirView()->scheduleGeometryRegen( PROPERTY_FILTERED );
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    propertyFilterCollection->notifyFiltersChanged();

    // The filter is displayed in the tree under the RimFilterInViewCollection facade, which only
    // rebuilds its tree ordering when notifyFiltersChanged() fires. Select and expand after that, so
    // the new filter node exists in the tree.
    Riu3DMainWindowTools::selectAsCurrentItem( propertyFilter, false );
    Riu3DMainWindowTools::setExpanded( propertyFilter, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter* RicEclipsePropertyFilterFeatureImpl::addPropertyFilterToCombinedFilter( RimCombinedFilter* combined )
{
    if ( !combined ) return nullptr;

    auto* propertyFilter = new RimEclipsePropertyFilter();
    combined->addFilter( propertyFilter );

    setDefaults( propertyFilter );

    RimQuickAccessCollection::instance()->addQuickAccessFields( propertyFilter );

    if ( RimEclipseView* view = combined->firstAncestorOrThisOfType<RimEclipseView>() )
    {
        view->scheduleGeometryRegen( PROPERTY_FILTERED );
        view->scheduleCreateDisplayModelAndRedraw();
    }

    combined->updateConnectedEditors();

    // setDefaults updates m_name via updateFilterName but doesn't fire a filterChanged signal.
    // Notify here so the combined parent can refresh its auto-derived display name.
    propertyFilter->triggerFilterChanged();

    // Select and expand after the tree has been refreshed, so the new filter node exists in the tree.
    Riu3DMainWindowTools::selectAsCurrentItem( propertyFilter, false );
    Riu3DMainWindowTools::setExpanded( propertyFilter, true );

    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::insertPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection, size_t index )
{
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFiltersField().insertAt( static_cast<int>( index ), propertyFilter );
    setDefaults( propertyFilter );

    RimQuickAccessCollection::instance()->addQuickAccessFields( propertyFilter );

    propertyFilterCollection->reservoirView()->scheduleGeometryRegen( PROPERTY_FILTERED );
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    propertyFilterCollection->notifyFiltersChanged();

    // The filter is displayed in the tree under the RimFilterInViewCollection facade, which only
    // rebuilds its tree ordering when notifyFiltersChanged() fires. Select and expand after that, so
    // the new filter node exists in the tree.
    Riu3DMainWindowTools::selectAsCurrentItem( propertyFilter, false );
    Riu3DMainWindowTools::setExpanded( propertyFilter, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable( caf::PdmObjectHandle* object )
{
    auto rimView = object->firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        RimViewController* vc = rimView->viewController();
        if ( vc && vc->isPropertyFilterOveridden() )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::setDefaults( RimEclipsePropertyFilter* propertyFilter )
{
    CAF_ASSERT( propertyFilter );

    // View-tolerant: case-level data filters have no RimEclipseView ancestor; the result
    // definition's eclipse case is already wired by RimEclipsePropertyFilter::setCase via the
    // owning collection's onItemsChanged hook.
    if ( auto* reservoirView = propertyFilter->firstAncestorOrThisOfType<RimEclipseView>() )
    {
        propertyFilter->resultDefinition()->setEclipseCase( reservoirView->eclipseCase() );

        if ( !RiaResultNames::isPerCellFaceResult( reservoirView->cellResult()->resultVariable() ) )
        {
            propertyFilter->resultDefinition()->simpleCopy( reservoirView->cellResult() );
        }
    }

    propertyFilter->resultDefinition()->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();
    propertyFilter->m_useCategorySelection = true;
}
