/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimFieldSelection.h"

#include "RimFieldQuickAccessInterface.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimFieldSelection, "RimFieldSelection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFieldSelection::RimFieldSelection()
{
    CAF_PDM_InitFieldNoDefault( &m_objectName, "ObjectName", "Object" );
    m_objectName.registerGetMethod( this, &RimFieldSelection::objectName );
    m_objectName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_keywords, "Keywords", "Field Keywords" );
    m_keywords.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_keywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldSelection::setObject( caf::PdmObject* object )
{
    m_object = object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldSelection::selectAllFields()
{
    if ( auto quickInterface = dynamic_cast<RimFieldQuickAccessInterface*>( m_object.p() ) )
    {
        std::vector<QString> keywords;
        for ( const auto& [groupName, fields] : quickInterface->quickAccessFields() )
        {
            for ( auto field : fields )
            {
                keywords.push_back( field->keyword() );
            }
        }

        m_keywords = keywords;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimFieldSelection::fields() const
{
    std::vector<caf::PdmFieldHandle*> fieldSelection;
    if ( auto quickInterface = dynamic_cast<RimFieldQuickAccessInterface*>( m_object.p() ) )
    {
        for ( const auto& [groupName, fields] : quickInterface->quickAccessFields() )
        {
            for ( auto field : fields )
            {
                const auto& keyword = field->keyword();
                if ( m_keywords().empty() || std::find( m_keywords().begin(), m_keywords().end(), keyword ) != m_keywords().end() )
                {
                    fieldSelection.push_back( field );
                }
            }
        }
    }

    return fieldSelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFieldSelection::objectName() const
{
    if ( m_object )
    {
        return m_object->uiName();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFieldSelection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_keywords )
    {
        if ( auto quickInterface = dynamic_cast<RimFieldQuickAccessInterface*>( m_object.p() ) )
        {
            for ( const auto& [groupName, fields] : quickInterface->quickAccessFields() )
            {
                for ( auto field : fields )
                {
                    QString displayText = field->keyword();
                    if ( field->uiCapability() )
                    {
                        displayText = field->uiCapability()->uiName();
                    }
                    options.push_back( { displayText, field->keyword() } );
                }
            }
        }
    }

    return options;
}
