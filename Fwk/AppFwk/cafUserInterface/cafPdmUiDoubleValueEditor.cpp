//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of the GNU General Public License or
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

#include "cafPdmUiDoubleValueEditor.h"

#include "cafPdmUiNumberFormat.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmLogging.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiDoubleValueEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiDoubleValueEditor::PdmUiDoubleValueEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_lineEdit.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_lineEdit->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );

    // Validate: warn about unsupported attributes
    uiField()->validateAttributes( "PdmUiDoubleValueEditor", SUPPORTED_ATTRIBUTES, uiConfigName );

    m_lineEdit->setText( formattedValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDoubleValueEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    containerWidget->setLayout( layout );

    m_lineEdit = new QLineEdit( containerWidget );
    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    layout->addWidget( m_lineEdit );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::slotEditingFinished()
{
    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDoubleValueEditor::writeValueToField()
{
    QString  textValue = m_lineEdit->text();
    QVariant v         = textValue;

    this->setValueToField( v );

    // This is required if the user entered an invalid value, we want to reset the text to the current value in the field
    m_lineEdit->setText( formattedValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiDoubleValueEditor::formattedValue()
{
    if ( !uiField() )
    {
        return {};
    }
    auto uiFieldHandle = uiField();

    NumberFormatType numberFormat = NumberFormatType::AUTO;
    int              precision    = 6;

    if ( auto val = uiFieldHandle->attribute<int>( Keys::DECIMALS ) )
    {
        precision = val.value();
    }

    if ( auto val = uiFieldHandle->attribute<int>( Keys::NUMBER_FORMAT ) )
    {
        numberFormat = static_cast<NumberFormatType>( val.value() );
    }

    bool   valueOk = false;
    double value   = uiFieldHandle->uiValue().toDouble( &valueOk );
    if ( valueOk )
    {
        return PdmUiNumberFormat::valueToText( value, numberFormat, precision );
    }

    return uiFieldHandle->uiValue().toString();
}

} // end namespace caf
