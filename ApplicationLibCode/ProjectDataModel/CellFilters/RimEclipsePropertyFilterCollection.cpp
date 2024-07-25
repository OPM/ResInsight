/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipsePropertyFilterCollection.h"

#include "RiaResultNames.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmUiEditorHandle.h"

CAF_PDM_SOURCE_INIT( RimEclipsePropertyFilterCollection, "CellPropertyFilters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection::RimEclipsePropertyFilterCollection()
{
    CAF_PDM_InitObject( "Property Filters", ":/CellFilter_Values.png" );

    CAF_PDM_InitFieldNoDefault( &m_propertyFilters, "PropertyFilters", "Property Filters" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipsePropertyFilterCollection::reservoirView()
{
    return firstAncestorOrThisOfType<RimEclipseView>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::setIsDuplicatedFromLinkedView()
{
    for ( RimEclipsePropertyFilter* propertyFilter : m_propertyFilters )
    {
        propertyFilter->setIsDuplicatedFromLinkedView( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipsePropertyFilter*> RimEclipsePropertyFilterCollection::propertyFilters() const
{
    return m_propertyFilters.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayField<RimEclipsePropertyFilter*>& RimEclipsePropertyFilterCollection::propertyFiltersField()
{
    return m_propertyFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for ( RimEclipsePropertyFilter* propertyFilter : m_propertyFilters )
    {
        propertyFilter->resultDefinition()->setEclipseCase( reservoirView()->eclipseCase() );
        propertyFilter->initAfterRead();
        if ( isActive() && propertyFilter->isActive() )
        {
            propertyFilter->resultDefinition()->loadResult();
            propertyFilter->computeResultValueRange();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::initAfterRead()
{
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::hasActiveFilters() const
{
    if ( !isActive ) return false;

    for ( RimEclipsePropertyFilter* propertyFilter : m_propertyFilters )
    {
        if ( propertyFilter->isActive() && propertyFilter->resultDefinition()->hasResult() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the active property filters are based on a dynamic result
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::hasActiveDynamicFilters() const
{
    if ( !isActive ) return false;

    for ( RimEclipsePropertyFilter* propertyFilter : m_propertyFilters )
    {
        if ( propertyFilter->isActive() && propertyFilter->resultDefinition()->hasDynamicResult() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::isUsingFormationNames() const
{
    if ( !isActive ) return false;

    for ( RimEclipsePropertyFilter* propertyFilter : m_propertyFilters )
    {
        if ( propertyFilter->isActive() && propertyFilter->resultDefinition()->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES &&
             propertyFilter->resultDefinition()->resultVariable() != RiaResultNames::undefinedResultName() )
            return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::updateIconState()
{
    bool activeIcon = true;

    auto view = firstAncestorOrThisOfType<RimEclipseView>();
    if ( view )
    {
        RimViewController* viewController = view->viewController();
        if ( viewController && ( viewController->isPropertyFilterOveridden() || viewController->isVisibleCellsOveridden() ) )
        {
            activeIcon = false;
        }
    }

    if ( !isActive )
    {
        activeIcon = false;
    }

    updateUiIconFromState( activeIcon );

    for ( RimEclipsePropertyFilter* cellFilter : m_propertyFilters )
    {
        cellFilter->updateActiveState();
        cellFilter->updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::updateFromCurrentTimeStep()
{
    for ( RimEclipsePropertyFilter* cellFilter : m_propertyFilters() )
    {
        cellFilter->updateFromCurrentTimeStep();
    }
}
