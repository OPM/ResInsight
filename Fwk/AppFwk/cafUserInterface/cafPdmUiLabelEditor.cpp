//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmUiLabelEditor.h"

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafQShortenedLabel.h"

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiLabelEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiLabelEditor::PdmUiLabelEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiLabelEditor::~PdmUiLabelEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLabelEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_label.isNull() );

    PdmUiLabelEditorAttribute attributes;
    if ( auto uiObject = uiObj( uiField()->fieldHandle()->ownerObject() ) )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );
    }

    if ( !attributes.m_linkText.isEmpty() )
    {
        // Configure rich text and hyper link support
        m_label->setTextFormat( Qt::RichText );
        m_label->setTextInteractionFlags( Qt::TextBrowserInteraction );

        caf::setLabelText( m_label, attributes.m_linkText );
    }
    else
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    if ( attributes.m_linkActivatedCallback )
    {
        connect( m_label,
                 &QLabel::linkActivated,
                 [attributes]( const QString& link ) { attributes.m_linkActivatedCallback( link ); } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLabelEditor::createCombinedWidget( QWidget* parent )
{
    if ( auto uiObject = uiObj( uiField()->fieldHandle()->ownerObject() ) )
    {
        const QString             uiConfigName;
        PdmUiLabelEditorAttribute attributes;
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );

        if ( attributes.m_useSingleWidgetInsteadOfLabelAndEditorWidget )
        {
            return createLabelWidget( parent );
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLabelEditor::createEditorWidget( QWidget* parent )
{
    return createLabelWidget( parent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLabelEditor::createLabelWidget( QWidget* parent )
{
    if ( m_label.isNull() )
    {
        PdmUiLabelEditorAttribute attributes;
        if ( auto uiObject = uiObj( uiField()->fieldHandle()->ownerObject() ) )
        {
            const QString uiConfigName;
            uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );
        }

        if ( attributes.m_useWordWrap )
        {
            m_label = new QLabel( parent );
            m_label->setWordWrap( true );
        }
        else
        {
            m_label = new QShortenedLabel( parent );
        }
    }

    return m_label;
}

} // end namespace caf
