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

#include "RicGeoMechPropertyFilterFeatureImpl.h"
#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimGeoMechResultDefinition.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechPropertyFilter*> RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilters()
{
    std::vector<RimGeoMechPropertyFilter*> propertyFilters;
    caf::SelectionManager::instance()->objectsByType(&propertyFilters);

    return propertyFilters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechPropertyFilterCollection*> RicGeoMechPropertyFilterFeatureImpl::selectedPropertyFilterCollections()
{
    std::vector<RimGeoMechPropertyFilterCollection*> propertyFilterCollections;
    caf::SelectionManager::instance()->objectsByType(&propertyFilterCollections);

    return propertyFilterCollections;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterFeatureImpl::addPropertyFilter(RimGeoMechPropertyFilterCollection* propertyFilterCollection)
{
    RimGeoMechPropertyFilter* propertyFilter = createPropertyFilter(propertyFilterCollection);
    CVF_ASSERT(propertyFilter);

    propertyFilterCollection->propertyFilters.push_back(propertyFilter);
    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(propertyFilter);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterFeatureImpl::insertPropertyFilter(RimGeoMechPropertyFilterCollection* propertyFilterCollection, size_t index)
{
    RimGeoMechPropertyFilter* propertyFilter = createPropertyFilter(propertyFilterCollection);
    CVF_ASSERT(propertyFilter);

    propertyFilterCollection->propertyFilters.insertAt(static_cast<int>(index), propertyFilter);
    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    propertyFilterCollection->reservoirView()->scheduleCreateDisplayModelAndRedraw();

    propertyFilterCollection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(propertyFilter);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterFeatureImpl::isPropertyFilterCommandAvailable(caf::PdmObjectHandle* object)
{
    // Reuse code from EclipseProperty filter, as the function is only dependent on RimView
    return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter* RicGeoMechPropertyFilterFeatureImpl::createPropertyFilter(RimGeoMechPropertyFilterCollection* propertyFilterCollection)
{
    CVF_ASSERT(propertyFilterCollection);

    RimGeoMechPropertyFilter* propertyFilter = new RimGeoMechPropertyFilter();
    propertyFilter->setParentContainer(propertyFilterCollection);

    setDefaults(propertyFilter);

    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterFeatureImpl::setDefaults(RimGeoMechPropertyFilter* propertyFilter)
{
    CVF_ASSERT(propertyFilter);

    RimGeoMechPropertyFilterCollection* propertyFilterCollection = propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    RimGeoMechView* reservoirView = propertyFilterCollection->reservoirView();
    CVF_ASSERT(reservoirView);

    propertyFilter->resultDefinition->setGeoMechCase(reservoirView->geoMechCase());
    propertyFilter->resultDefinition->setResultAddress(reservoirView->cellResultResultDefinition()->resultAddress());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();
}
