//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmObjectHandle.h"

#include "cafAssert.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldReorderCapability::PdmFieldReorderCapability( PdmPtrArrayFieldHandle* field, bool giveOwnership )
    : orderChanged( this )
    , m_field( field )

{
    field->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::canItemBeMovedUp( size_t index ) const
{
    return index != 0u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::canItemBeMovedDown( size_t index ) const
{
    return m_field->size() > 1u && index < m_field->size() - 1u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::moveItemUp( size_t index )
{
    if ( canItemBeMovedUp( index ) )
    {
        PdmObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            size_t newIndex = index - 1u;
            m_field->erase( index );
            m_field->insertAt( (int)newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::moveItemDown( size_t index )
{
    if ( canItemBeMovedDown( index ) )
    {
        PdmObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            size_t newIndex = index + 1u;
            m_field->erase( index );
            m_field->insertAt( (int)newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldReorderCapability* PdmFieldReorderCapability::addToField( PdmPtrArrayFieldHandle* field )
{
    if ( !fieldIsReorderable( field ) )
    {
        new PdmFieldReorderCapability( field, true );
    }
    return field->capability<PdmFieldReorderCapability>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::fieldIsReorderable( PdmPtrArrayFieldHandle* field )
{
    return field->capability<PdmFieldReorderCapability>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldReorderCapability::onMoveItemUp( const SignalEmitter* emitter, size_t index )
{
    moveItemUp( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldReorderCapability::onMoveItemDown( const SignalEmitter* emitter, size_t index )
{
    moveItemDown( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldReorderCapability::onMoveItemToTop( const SignalEmitter* emitter, size_t index )
{
    moveItemToTop( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldReorderCapability::moveItemToTop( size_t index )
{
    if ( canItemBeMovedUp( index ) )
    {
        PdmObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            int newIndex = 0;
            m_field->erase( index );
            m_field->insertAt( newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldReorderCapability* PdmFieldReorderCapability::reorderCapabilityOfParentContainer( const PdmObjectHandle* pdmObject )
{
    if ( pdmObject )
    {
        PdmPtrArrayFieldHandle* arrayField = dynamic_cast<PdmPtrArrayFieldHandle*>( pdmObject->parentField() );
        if ( arrayField )
        {
            PdmFieldReorderCapability* reorderability = arrayField->capability<PdmFieldReorderCapability>();
            return reorderability;
        }
    }

    return nullptr;
}
