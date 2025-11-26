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

#include "cafPdmUiRadioButtonEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWidget>

namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiRadioButtonEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiRadioButtonEditor::PdmUiRadioButtonEditor()
    : m_containerWidget( nullptr )
    , m_groupBox( nullptr )
    , m_verticalLayout( nullptr )
    , m_horizontalLayout( nullptr )
    , m_buttonGroup( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiRadioButtonEditor::~PdmUiRadioButtonEditor()
{
    clearRadioButtons();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiRadioButtonEditor::createEditorWidget( QWidget* parent )
{
    m_containerWidget = new QWidget( parent );
    m_buttonGroup     = new QButtonGroup( m_containerWidget );

    // Create main layout for the container
    QVBoxLayout* mainLayout = new QVBoxLayout( m_containerWidget );
    mainLayout->setContentsMargins( 0, 0, 0, 0 );

    // Create group box for radio buttons
    m_groupBox = new QGroupBox( m_containerWidget );
    mainLayout->addWidget( m_groupBox );

    return m_containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiRadioButtonEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_containerWidget.isNull() );

    if ( !m_label.isNull() )
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    // Handle attributes
    caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &m_attributes );
    }

    // Clear any existing layout and radio buttons
    if ( m_verticalLayout )
    {
        delete m_verticalLayout;
        m_verticalLayout = nullptr;
    }
    if ( m_horizontalLayout )
    {
        delete m_horizontalLayout;
        m_horizontalLayout = nullptr;
    }

    clearRadioButtons();

    // Create appropriate layout based on orientation
    if ( m_attributes.orientation == Qt::Vertical )
    {
        m_verticalLayout = new QVBoxLayout( m_groupBox );
        m_verticalLayout->setContentsMargins( 6, 6, 6, 6 );
        m_verticalLayout->setSpacing( 2 );
    }
    else
    {
        m_horizontalLayout = new QHBoxLayout( m_groupBox );
        m_horizontalLayout->setContentsMargins( 6, 6, 6, 6 );
        m_horizontalLayout->setSpacing( 6 );
    }

    // Update radio buttons based on current options
    updateRadioButtons();

    // Set enabled state and tooltip
    bool isEnabled = !uiField()->isUiReadOnly( uiConfigName );
    m_containerWidget->setEnabled( isEnabled );
    m_containerWidget->setToolTip( uiField()->uiToolTip( uiConfigName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiRadioButtonEditor::updateRadioButtons()
{
    QList<PdmOptionItemInfo> options = uiField()->valueOptions();

    for ( int i = 0; i < options.size(); ++i )
    {
        QRadioButton* radioButton = new QRadioButton( options[i].optionUiText() );

        // Store the option value as user data
        radioButton->setProperty( "optionValue", options[i].value() );

        // Add to button group for mutual exclusion
        m_buttonGroup->addButton( radioButton, i );

        // Add to layout
        if ( m_attributes.orientation == Qt::Vertical && m_verticalLayout )
        {
            m_verticalLayout->addWidget( radioButton );
        }
        else if ( m_attributes.orientation == Qt::Horizontal && m_horizontalLayout )
        {
            m_horizontalLayout->addWidget( radioButton );
        }

        // Connect signal
        connect( radioButton, SIGNAL( toggled( bool ) ), this, SLOT( slotRadioButtonToggled( bool ) ) );

        m_radioButtons.append( radioButton );
    }

    // Set current selection
    QVariant fieldValue = uiField()->uiValue();
    for ( QRadioButton* radioButton : m_radioButtons )
    {
        QVariant buttonValue = radioButton->property( "optionValue" );
        if ( buttonValue == fieldValue )
        {
            radioButton->setChecked( true );
            break;
        }
    }

    // Add stretch to vertical layout
    if ( m_attributes.orientation == Qt::Vertical && m_verticalLayout )
    {
        m_verticalLayout->addStretch();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiRadioButtonEditor::clearRadioButtons()
{
    for ( QRadioButton* radioButton : m_radioButtons )
    {
        if ( radioButton )
        {
            delete radioButton;
        }
    }
    m_radioButtons.clear();

    if ( m_buttonGroup )
    {
        // Clear the button group
        QList<QAbstractButton*> buttons = m_buttonGroup->buttons();
        for ( QAbstractButton* button : buttons )
        {
            m_buttonGroup->removeButton( button );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiRadioButtonEditor::slotRadioButtonToggled( bool checked )
{
    if ( !checked ) return; // Only respond to the button being checked, not unchecked

    QRadioButton* radioButton = dynamic_cast<QRadioButton*>( sender() );
    if ( !radioButton ) return;

    QVariant newValue = radioButton->property( "optionValue" );
    this->setValueToField( newValue );
}

} // end namespace caf