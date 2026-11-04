/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimGroupNetworkNode.h"

#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT( RimGroupNetworkNode, "RimGroupNetworkNode" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkNode::RimGroupNetworkNode()
{
    CAF_PDM_InitObject( "Network Node", ":/summary/components/images/production-network.svg" );

    CAF_PDM_InitFieldNoDefault( &m_treeText, "TreeText", "Selected Node" );
    m_treeText.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitFieldNoDefault( &m_nodeType, "NodeType", "Type" );
    CAF_PDM_InitFieldNoDefault( &m_parent, "Parent", "Parent" );
    CAF_PDM_InitFieldNoDefault( &m_keyword, "Keyword", "Defined by" );
    CAF_PDM_InitFieldNoDefault( &m_properties, "Properties", "Properties" );
    CAF_PDM_InitFieldNoDefault( &m_values, "Values", "Values" );
    m_values.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_values.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );

    for ( caf::PdmFieldHandle* field : fields() )
    {
        field->uiCapability()->setUiReadOnly( true );
        field->xmlCapability()->disableIO();
    }

    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkNode::setNodeName( const QString& name )
{
    m_name     = name;
    m_treeText = "Selected Node: " + ( name.isEmpty() ? "None" : name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkNode::setDescription( const QString& nodeType, const QString& parent, const QString& keyword )
{
    m_nodeType = nodeType;
    m_parent   = parent;
    m_keyword  = keyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkNode::setValues( const QString& properties, const QString& values )
{
    m_properties = properties;
    m_values     = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkNode::clear()
{
    setNodeName( QString() );
    setDescription( QString(), QString(), QString() );
    setValues( QString(), QString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkNode::nodeName() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGroupNetworkNode::userDescriptionField()
{
    return &m_treeText;
}
