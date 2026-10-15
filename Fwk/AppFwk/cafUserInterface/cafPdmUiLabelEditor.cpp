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

#include "cafPdmLogging.h"
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

    const bool    useWordWrap = uiField()->attribute<bool>( Keys::USE_WORD_WRAP, uiConfigName ).value_or( false );
    const QString linkText    = uiField()->attribute<QString>( Keys::LINK_TEXT, uiConfigName ).value_or( QString() );
    const auto    linkActivatedCallback =
        uiField()->attribute<std::function<void( const QString& )>>( Keys::LINK_ACTIVATED_CALLBACK, uiConfigName );

    uiField()->validateAttributes( "PdmUiLabelEditor", SUPPORTED_ATTRIBUTES, uiConfigName );

    if ( !linkText.isEmpty() )
    {
        // Configure rich text and hyper link support
        m_label->setTextFormat( Qt::RichText );
        m_label->setTextInteractionFlags( Qt::TextBrowserInteraction );

        caf::setLabelText( m_label, linkText );
    }
    else
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    // Called on every UI update, so drop the previous connection to avoid invoking the callback multiple times
    disconnect( m_label, &QLabel::linkActivated, this, nullptr );

    if ( linkActivatedCallback && linkActivatedCallback.value() )
    {
        connect( m_label,
                 &QLabel::linkActivated,
                 this,
                 [callback = linkActivatedCallback.value()]( const QString& link ) { callback( link ); } );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLabelEditor::createCombinedWidget( QWidget* parent )
{
    const QString uiConfigName;
    if ( uiField()->attribute<bool>( Keys::USE_SINGLE_WIDGET_INSTEAD_OF_LABEL_AND_EDITOR_WIDGET, uiConfigName ).value_or( false ) )
    {
        return createLabelWidget( parent );
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
        const QString uiConfigName;
        const bool    useWordWrap = uiField()->attribute<bool>( Keys::USE_WORD_WRAP, uiConfigName ).value_or( false );

        if ( useWordWrap )
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
