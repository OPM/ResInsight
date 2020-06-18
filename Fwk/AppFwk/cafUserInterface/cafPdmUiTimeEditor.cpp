//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafPdmUiTimeEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafQShortenedLabel.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiTimeEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTimeEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_timeEdit.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_timeEdit->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );

    caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &m_attributes );
    }

    if ( !m_attributes.timeFormat.isEmpty() )
    {
        m_timeEdit->setDisplayFormat( m_attributes.timeFormat );
    }

    m_timeEdit->setTime( uiField()->uiValue().toTime() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTimeEditor::createEditorWidget( QWidget* parent )
{
    m_timeEdit = new QTimeEdit( parent );
    connect( m_timeEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );
    connect( m_timeEdit, SIGNAL( timeChanged( QTime ) ), this, SLOT( slotTimeChanged( QTime ) ) );
    return m_timeEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTimeEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTimeEditor::slotEditingFinished()
{
    this->setValueToField( m_timeEdit->time() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTimeEditor::slotTimeChanged( const QTime& time )
{
    this->setValueToField( m_timeEdit->time() );
}

} // end namespace caf
