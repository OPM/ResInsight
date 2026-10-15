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

#include "cafPdmUiCheckBoxEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafQShortenedLabel.h"

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiCheckBoxEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_checkBox.isNull() );
    CAF_ASSERT( !m_label.isNull() );

    uiField()->validateAttributes( "PdmUiCheckBoxEditor", SUPPORTED_ATTRIBUTES, uiConfigName );

    const bool useNative =
        uiField()->attribute<bool>( Keys::USE_NATIVE_CHECKBOX, uiConfigName ).value_or( useNativeCheckBoxByDefault() );

    if ( useNative )
    {
        m_checkBox->setText( uiField()->uiName( uiConfigName ) );

        m_label->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
        m_label->setToolTip( uiField()->uiToolTip( uiConfigName ) );
    }
    else
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    m_checkBox->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_checkBox->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    m_checkBox->setChecked( uiField()->uiValue().toBool() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxEditor::createEditorWidget( QWidget* parent )
{
    m_checkBox = new QCheckBox( parent );
    connect( m_checkBox, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxEditor::createLabelWidget( QWidget* parent )
{
    const QString uiConfigName;
    const bool    useWordWrap = uiField()->attribute<bool>( Keys::WORD_WRAP, uiConfigName ).value_or( false );

    if ( useWordWrap )
    {
        m_label = new QLabel( parent );
        m_label->setWordWrap( true );
    }
    else
    {
        m_label = new QShortenedLabel( parent );
    }

    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxEditor::slotClicked( bool checked )
{
    QVariant v;
    v = checked;
    this->setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiCheckBoxEditor::useNativeCheckBoxByDefault() const
{
    return false;
}

//==================================================================================================
//
//
// PdmUiNativeCheckBoxEditor
//
//
//==================================================================================================

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiNativeCheckBoxEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiNativeCheckBoxEditor::configureFieldForEditor( caf::PdmFieldHandle* fieldHandle )
{
    if ( !fieldHandle ) return;

    if ( auto uiCap = fieldHandle->uiCapability() )
    {
        uiCap->setUiEditorTypeName( caf::PdmUiNativeCheckBoxEditor::uiEditorTypeName() );

        // Hide the editor label, as the label is managed by the native checkbox
        uiCap->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiNativeCheckBoxEditor::useNativeCheckBoxByDefault() const
{
    return true;
}

} // end namespace caf
