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

#include "cafTreeNode.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( cafTreeNode, "cafTreeNode" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cafTreeNode::cafTreeNode()
{
    CAF_PDM_InitObject( "WellPath", ":/Folder.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_childNodes, "ChildNodes", "ChildNodes", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafTreeNode::addChild( cafTreeNode* treeNode )
{
    m_childNodes.push_back( treeNode );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cafTreeNode*> cafTreeNode::childNodes() const
{
    return m_childNodes.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* cafTreeNode::referencedObject() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObject*> cafTreeNode::allReferencedObjects() const
{
    std::vector<caf::PdmObject*> objects;

    allReferencedObjectsRecursively( this, objects );

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafTreeNode::allReferencedObjectsRecursively( const cafTreeNode* node, std::vector<caf::PdmObject*>& objects )
{
    if ( auto obj = node->referencedObject() )
    {
        objects.push_back( obj );

        return;
    }

    for ( auto c : node->childNodes() )
    {
        allReferencedObjectsRecursively( c, objects );
    }
}

//--------------------------------------------------------------------------------------------------
///
///
///
///
///
///
///
///
/// cafNamedTreeNode
//--------------------------------------------------------------------------------------------------

CAF_PDM_SOURCE_INIT( cafNamedTreeNode, "cafNamedTreeNode" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cafNamedTreeNode::cafNamedTreeNode()
    : m_showCheckedBox( false )
{
    CAF_PDM_InitObject( "Node", ":/Folder.svg", "", "" );

    CAF_PDM_InitField( &m_name, "Name", QString(), "Name", "", "", "" );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isChecked, "IsChecked", true, "Active", "", "", "" );
    m_isChecked.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafNamedTreeNode::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString cafNamedTreeNode::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafNamedTreeNode::setIcon( const QString& iconResourceName )
{
    this->setUiIconFromResourceString( iconResourceName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafNamedTreeNode::setCheckedState( bool enable )
{
    m_isChecked = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool cafNamedTreeNode::isChecked() const
{
    return m_isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* cafNamedTreeNode::objectToggleField()
{
    if ( m_showCheckedBox ) return &m_isChecked;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* cafNamedTreeNode::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
///
///
///
///
///
///
///
/// cafObjectReferenceTreeNode
//--------------------------------------------------------------------------------------------------

CAF_PDM_SOURCE_INIT( cafObjectReferenceTreeNode, "cafObjectReferenceTreeNode" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cafObjectReferenceTreeNode::cafObjectReferenceTreeNode()
{
    CAF_PDM_InitObject( "cafObjectReferenceTreeNode", ":/Folder.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_referencedObject, "ReferencedObject", "Referenced Object", "", "", "" );

    m_childNodes.uiCapability()->setUiTreeHidden( true );

    uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafObjectReferenceTreeNode::setReferencedObject( caf::PdmObject* object )
{
    m_referencedObject = object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* cafObjectReferenceTreeNode::referencedObject() const
{
    return m_referencedObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void cafObjectReferenceTreeNode::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                       QString                 uiConfigName /*= "" */ )
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
