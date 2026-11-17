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

#include "RicEclipsePropertyFilterNewExec.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimGridView.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::RicEclipsePropertyFilterNewExec( RimEclipsePropertyFilterCollection* propertyFilterCollection,
                                                                  bool                                formationNames )
    : CmdExecuteCommand( nullptr )
    , m_formationNames( formationNames )
{
    assert( propertyFilterCollection );
    m_propertyFilterCollection = propertyFilterCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::~RicEclipsePropertyFilterNewExec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicEclipsePropertyFilterNewExec::name()
{
    return m_formationNames ? "New Formation Filter" : "New Property Filter";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::redo()
{
    RicEclipsePropertyFilterFeatureImpl::addPropertyFilter( m_propertyFilterCollection, m_formationNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::undo()
{
    m_propertyFilterCollection->propertyFiltersField().erase( m_propertyFilterCollection->propertyFiltersField().size() - 1 );

    m_propertyFilterCollection->updateConnectedEditors();
    m_propertyFilterCollection->filtersChanged.send();
}
