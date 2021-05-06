/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimWellPathNode.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellPathNode, "WellPathNode" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathNode::RimWellPathNode()
{
    CAF_PDM_InitObject( "WellPath", ":/Folder.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_referencedObject, "ReferencedObject", "Referenced Object", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_childNodes, "ChildNodes", "ChildNodes", "", "", "" );
    m_childNodes.uiCapability()->setUiTreeHidden( true );

    uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathNode::addChild( RimWellPathNode* object )
{
    m_childNodes.push_back( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathNode::setReferencedObject( caf::PdmObject* object )
{
    m_referencedObject = object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimWellPathNode::referencedObject()
{
    return m_referencedObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathNode*> RimWellPathNode::childNodes() const
{
    return m_childNodes.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathNode::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathNode::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( m_referencedObject )
    {
        uiTreeOrdering.add( m_referencedObject() );
    }

    for ( auto c : m_childNodes.childObjects() )
    {
        if ( auto obj = c->referencedObject() )
        {
            uiTreeOrdering.add( obj );
        }
        else
        {
            uiTreeOrdering.add( c );
        }
    }

    uiTreeOrdering.skipRemainingChildren();
}
