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

#include "cafPdmUiDateEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmLogging.h"
#include "cafPdmObject.h"
#include "cafPdmUiAttributeTools.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QDate>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiDateEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDateEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_dateEdit.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_dateEdit->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );

    uiField()->validateAttributes( "PdmUiDateEditor", SUPPORTED_ATTRIBUTES, uiConfigName );

    QString dateFormat =
        PdmUiAttributeTools::stringAttribute( uiField(), Keys::DATE_FORMAT, uiConfigName ).value_or( QString() );
    if ( !dateFormat.isEmpty() )
    {
        m_dateEdit->setDisplayFormat( dateFormat );
    }

    m_dateEdit->setDate( uiField()->uiValue().toDate() );
    m_dateEdit->setTime( uiField()->uiValue().toTime() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDateEditor::createEditorWidget( QWidget* parent )
{
    m_dateEdit = new QDateTimeEdit( parent );
    m_dateEdit->setCalendarPopup( true );
    connect( m_dateEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );
    return m_dateEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDateEditor::slotEditingFinished()
{
    this->setValueToField( m_dateEdit->dateTime() );
}

} // end namespace caf
