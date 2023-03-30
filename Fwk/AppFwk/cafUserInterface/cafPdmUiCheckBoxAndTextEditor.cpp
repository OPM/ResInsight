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

#include "cafPdmUiCheckBoxAndTextEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafQShortenedLabel.h"

#include <QIntValidator>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiCheckBoxAndTextEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxAndTextEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_textEdit.isNull() );
    CAF_ASSERT( !m_label.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_textEdit->setReadOnly( uiField()->isUiReadOnly( uiConfigName ) );
    // m_textEdit->setEnabled(!field()->isUiReadOnly(uiConfigName)); // Neccesary ?
    m_textEdit->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    connect( m_textEdit, SIGNAL( editingFinished() ), this, SLOT( slotSetValueToField() ) );

    std::pair<bool, double> pairValue;
    pairValue = uiField()->uiValue().value<std::pair<bool, double>>();

    m_textEdit->blockSignals( true );
    m_textEdit->setText( QString( "%1" ).arg( pairValue.second ) );
    m_textEdit->blockSignals( false );

    m_checkBox->blockSignals( true );
    m_checkBox->setChecked( pairValue.first );
    m_checkBox->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxAndTextEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    m_textEdit = new QLineEdit( containerWidget );
    connect( m_textEdit, SIGNAL( editingFinished() ), this, SLOT( slotSetValueToField() ) );

    m_checkBox = new QCheckBox( "", containerWidget );
    connect( m_checkBox, SIGNAL( clicked() ), this, SLOT( slotSetValueToField() ) );

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget( m_checkBox );
    layout->addWidget( m_textEdit );
    layout->setMargin( 0 );

    containerWidget->setLayout( layout );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxAndTextEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxAndTextEditor::slotSetValueToField()
{
    bool isChecked = m_checkBox->checkState() == Qt::CheckState::Checked;

    double   value     = m_textEdit->text().toDouble();
    auto     pairValue = std::make_pair( isChecked, value );
    QVariant v         = QVariant::fromValue( pairValue );

    this->setValueToField( v );
}

} // end namespace caf
