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

#include "RicEclipsePropertyFilter.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseCellColors.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilter*> RicEclipsePropertyFilter::selectedPropertyFilters()
{
    std::vector<RimEclipsePropertyFilter*> propertyFilters;
    caf::SelectionManager::instance()->objectsByType(&propertyFilters);

    return propertyFilters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilterCollection*> RicEclipsePropertyFilter::selectedPropertyFilterCollections()
{
    std::vector<RimEclipsePropertyFilterCollection*> propertyFilterCollections;
    caf::SelectionManager::instance()->objectsByType(&propertyFilterCollections);

    return propertyFilterCollections;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilter::addPropertyFilter(RimEclipsePropertyFilterCollection* propertyFilterCollection)
{
    RimEclipsePropertyFilter* propertyFilter = createPropertyFilter(propertyFilterCollection);
    CVF_ASSERT(propertyFilter);

    propertyFilterCollection->propertyFilters.push_back(propertyFilter);
    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);

    propertyFilterCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilter::insertPropertyFilter(RimEclipsePropertyFilterCollection* propertyFilterCollection, size_t index)
{
    RimEclipsePropertyFilter* propertyFilter = createPropertyFilter(propertyFilterCollection);
    CVF_ASSERT(propertyFilter);

    propertyFilterCollection->propertyFilters.insertAt(index, propertyFilter);
    propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);

    propertyFilterCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter* RicEclipsePropertyFilter::createPropertyFilter( RimEclipsePropertyFilterCollection* propertyFilterCollection )
{
    CVF_ASSERT(propertyFilterCollection);

    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilter->setParentContainer(propertyFilterCollection);

    setDefaults(propertyFilter);

    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilter::setDefaults(RimEclipsePropertyFilter* propertyFilter)
{
    CVF_ASSERT(propertyFilter);

    RimEclipsePropertyFilterCollection* propertyFilterCollection = propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    RimEclipseView* reservoirView = propertyFilterCollection->reservoirView();
    CVF_ASSERT(reservoirView);

    propertyFilter->resultDefinition->setReservoirView(reservoirView);
    propertyFilter->resultDefinition->setResultVariable(reservoirView->cellResult->resultVariable());
    propertyFilter->resultDefinition->setPorosityModel(reservoirView->cellResult->porosityModel());
    propertyFilter->resultDefinition->setResultType(reservoirView->cellResult->resultType());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();
}
