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

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseCellColors.h"
#include "RimViewController.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"
#include "RiuMainWindow.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilter*> RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilters()
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters;
    caf::SelectionManager::instance()->objectsByType(&propertyFilters);

    return propertyFilters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilterCollection*> RicEclipsePropertyFilterFeatureImpl::selectedPropertyFilterCollections()
{
    std::vector<RimEclipsePropertyFilterCollection*> propertyFilterCollections;
    caf::SelectionManager::instance()->objectsByType(&propertyFilterCollections);

    return propertyFilterCollections;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::addPropertyFilter(RimEclipsePropertyFilterCollection* propertyFilterCollection)
{
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFilters.push_back(propertyFilter);
    setDefaults(propertyFilter);

    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(propertyFilter);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::insertPropertyFilter(RimEclipsePropertyFilterCollection* propertyFilterCollection, size_t index)
{
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilterCollection->propertyFilters.insertAt(static_cast<int>(index), propertyFilter);
    setDefaults(propertyFilter);

    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(propertyFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable(caf::PdmObjectHandle* object)
{
    RimView* rimView = NULL;
    object->firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        RimViewController* vc = rimView->viewController();
        if (vc && vc->isPropertyFilterOveridden())
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterFeatureImpl::setDefaults(RimEclipsePropertyFilter* propertyFilter)
{
    CVF_ASSERT(propertyFilter);

    RimEclipsePropertyFilterCollection* propertyFilterCollection = nullptr;
    propertyFilter->firstAncestorOrThisOfTypeAsserted(propertyFilterCollection);

    RimEclipseView* reservoirView = nullptr;
    propertyFilter->firstAncestorOrThisOfTypeAsserted(reservoirView);

    propertyFilter->resultDefinition->setEclipseCase(reservoirView->eclipseCase());
    propertyFilter->resultDefinition->simpleCopy(reservoirView->cellResult);

    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();
    propertyFilter->m_useCategorySelection = true;
}
