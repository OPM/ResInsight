//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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

#include "cafPdmUiActionPushButtonEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"

#include "cafFactory.h"

#include <QBoxLayout>

#include <cmath>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiActionPushButtonEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiActionPushButtonEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_pushButton.isNull() );

    m_pushButton->setCheckable( true );
    m_pushButton->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_pushButton->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    m_buttonLayout->setAlignment( Qt::AlignRight );

    auto icon = uiField()->uiIcon( uiConfigName );
    if ( icon )
    {
        m_pushButton->setIcon( *icon );
        m_pushButton->setMaximumWidth( m_pushButton->sizeHint().width() );
    }
    else
    {
        m_pushButton->setText( uiField()->uiName( uiConfigName ) );
        QFontMetrics fontMetr = m_pushButton->fontMetrics();
        m_pushButton->setMaximumWidth( fontMetr.width( m_pushButton->text() ) + 2 * fontMetr.width( "M" ) );
    }

    QVariant variantFieldValue = uiField()->uiValue();

    if ( variantFieldValue.type() == QVariant::Bool )
    {
        m_pushButton->setChecked( uiField()->uiValue().toBool() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiActionPushButtonEditor::configureEditorForField( PdmFieldHandle* fieldHandle )
{
    CAF_ASSERT( fieldHandle );
    CAF_ASSERT( fieldHandle->uiCapability() );

    if ( fieldHandle->xmlCapability() )
    {
        fieldHandle->xmlCapability()->disableIO();
    }

    fieldHandle->uiCapability()->setUiEditorTypeName( caf::PdmUiActionPushButtonEditor::uiEditorTypeName() );
    fieldHandle->uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiActionPushButtonEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    m_pushButton = new QPushButton( containerWidget );
    connect( m_pushButton, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );

    m_buttonLayout = new QHBoxLayout( containerWidget );
    m_buttonLayout->addWidget( m_pushButton );
    m_buttonLayout->setMargin( 0 );
    m_buttonLayout->setSpacing( 0 );
    containerWidget->setLayout( m_buttonLayout );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiActionPushButtonEditor::slotClicked( bool checked )
{
    if ( uiField() && dynamic_cast<PdmField<bool>*>( uiField()->fieldHandle() ) )
    {
        QVariant v;
        v = checked;
        this->setValueToField( v );
    }
}

} // end namespace caf
