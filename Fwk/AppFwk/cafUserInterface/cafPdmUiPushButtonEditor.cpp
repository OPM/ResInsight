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

#include "cafPdmUiPushButtonEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafQShortenedLabel.h"

#include <QBoxLayout>

#include <cmath>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiPushButtonEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    if ( !m_pushButton ) return;

    if ( m_label )
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    m_pushButton->setCheckable( true );
    m_pushButton->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_pushButton->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    PdmUiPushButtonEditorAttribute attributes;
    caf::PdmUiObjectHandle*        uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );
    }

    QVariant variantFieldValue = uiField()->uiValue();

    if ( !attributes.m_buttonIcon.isNull() )
    {
        m_pushButton->setIcon( attributes.m_buttonIcon );
    }

    if ( !attributes.m_buttonText.isEmpty() )
    {
        m_pushButton->setText( attributes.m_buttonText );
    }
    else
    {
        if ( variantFieldValue.metaType().id() == QMetaType::Bool )
        {
            m_pushButton->setText( variantFieldValue.toBool() ? "On" : "Off" );
        }
        else
        {
            m_pushButton->setText( variantFieldValue.toString() );
        }
    }

    if ( uiField()->uiLabelPosition( uiConfigName ) != PdmUiItemInfo::HIDDEN )
    {
        QSize defaultSize = m_pushButton->sizeHint();
        m_pushButton->setMinimumWidth( 10 * std::round( 0.1 * ( defaultSize.width() + 10 ) ) );
        m_buttonLayout->setAlignment( m_pushButton, Qt::AlignRight );
    }

    if ( variantFieldValue.metaType().id() == QMetaType::Bool )
    {
        m_pushButton->setChecked( uiField()->uiValue().toBool() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureEditorLabelLeft( PdmFieldHandle* fieldHandle )
{
    if ( fieldHandle )
    {
        if ( auto xmlCap = fieldHandle->xmlCapability() )
        {
            xmlCap->disableIO();
        }

        if ( auto uiCap = fieldHandle->uiCapability() )
        {
            uiCap->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
            uiCap->setUiLabelPosition( caf::PdmUiItemInfo::LEFT );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::configureEditorLabelHidden( PdmFieldHandle* fieldHandle )
{
    if ( fieldHandle )
    {
        if ( auto xmlCap = fieldHandle->xmlCapability() )
        {
            xmlCap->disableIO();
        }

        if ( auto uiCap = fieldHandle->uiCapability() )
        {
            uiCap->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
            uiCap->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    m_pushButton = new QPushButton( "", containerWidget );
    connect( m_pushButton, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );

    m_buttonLayout = new QHBoxLayout( containerWidget );
    m_buttonLayout->addWidget( m_pushButton );
    m_buttonLayout->setContentsMargins( 0, 0, 0, 0 );
    m_buttonLayout->setSpacing( 0 );

    containerWidget->setLayout( m_buttonLayout );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiPushButtonEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiPushButtonEditor::slotClicked( bool checked )
{
    if ( uiField() && dynamic_cast<PdmField<bool>*>( uiField()->fieldHandle() ) )
    {
        QVariant v;
        v = checked;
        this->setValueToField( v );
    }
}

} // end namespace caf
