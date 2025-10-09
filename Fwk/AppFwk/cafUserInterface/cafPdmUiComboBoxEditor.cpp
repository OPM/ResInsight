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

#include "cafPdmUiComboBoxEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafUiAppearanceSettings.h"
#include "cafUiIconFactory.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QWheelEvent>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiComboBoxEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    if ( !uiField() ) return;

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

    if ( !m_comboBox.isNull() )
    {
        m_comboBox->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
        m_comboBox->setToolTip( uiField()->uiToolTip( uiConfigName ) );

        QList<PdmOptionItemInfo> options = uiField()->valueOptions();

        m_comboBox->blockSignals( true );
        m_comboBox->clear();
        QListView* listView = dynamic_cast<QListView*>( m_comboBox->view() );
        if ( listView )
        {
            listView->setSpacing( 2 );
        }

        if ( !options.isEmpty() )
        {
            for ( const auto& option : options )
            {
                auto icon = option.icon();
                if ( icon )
                    m_comboBox->addItem( *icon, option.optionUiText() );
                else
                    m_comboBox->addItem( option.optionUiText() );
                m_comboBox->setIconSize( m_attributes.iconSize );
            }
            m_comboBox->setCurrentIndex( uiField()->uiValue().toInt() );
        }
        else
        {
            m_comboBox->addItem( uiField()->uiValue().toString() );
            m_comboBox->setCurrentIndex( 0 );
        }

        if ( m_attributes.adjustWidthToContents )
        {
            m_comboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
        }
        else if ( m_attributes.minimumContentsLength > 0 )
        {
            m_comboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
            m_comboBox->setMinimumContentsLength( m_attributes.minimumContentsLength );
            // Make sure the popup adjusts to the content even if the widget itself doesn't
            QFont font = m_comboBox->view()->font();

            int  maxTextWidth = 0;
            bool labelsElided = false;
            for ( const PdmOptionItemInfo& option : options )
            {
                QString label = option.optionUiText();
                if ( label.size() > m_attributes.maximumMenuContentsLength )
                {
                    label.resize( m_attributes.maximumMenuContentsLength );
                    labelsElided = true;
                }
                maxTextWidth = std::max( maxTextWidth, QFontMetrics( font ).boundingRect( label ).width() );
            }

            int marginWidth = m_comboBox->view()->contentsMargins().left() + m_comboBox->view()->contentsMargins().right();
            m_comboBox->view()->setMinimumWidth( maxTextWidth + marginWidth );
            m_comboBox->view()->setTextElideMode( labelsElided ? Qt::ElideMiddle : Qt::ElideNone );
        }

        if ( m_attributes.enableEditableContent )
        {
            m_comboBox->setEditable( true );

            if ( !m_attributes.enableAutoComplete )
            {
                m_comboBox->setCompleter( nullptr );
            }

            m_comboBox->lineEdit()->setPlaceholderText( m_attributes.placeholderText );
        }

        if ( m_attributes.notifyWhenTextIsEdited )
        {
            connect( m_comboBox,
                     SIGNAL( editTextChanged( const QString& ) ),
                     this,
                     SLOT( slotEditTextChanged( const QString& ) ) );

            if ( m_interactiveEditText == m_comboBox->lineEdit()->text() && m_interactiveEditCursorPosition > -1 )
            {
                m_comboBox->lineEdit()->setCursorPosition( m_interactiveEditCursorPosition );
                m_comboBox->lineEdit()->deselect();
            }
        }

        if ( m_attributes.minimumWidth != -1 )
        {
            m_comboBox->setMinimumWidth( m_attributes.minimumWidth );
        }

        m_comboBox->blockSignals( false );
    }

    if ( !m_layout.isNull() )
    {
        if ( m_attributes.showPreviousAndNextButtons )
        {
            if ( m_previousItemButton.isNull() )
            {
                m_previousItemButton = new QToolButton( m_placeholder );
                connect( m_previousItemButton, SIGNAL( clicked() ), this, SLOT( slotPreviousButtonPressed() ) );

                m_previousItemButton->setToolTip( "Previous" );
            }

            if ( m_nextItemButton.isNull() )
            {
                m_nextItemButton = new QToolButton( m_placeholder );
                connect( m_nextItemButton, SIGNAL( clicked() ), this, SLOT( slotNextButtonPressed() ) );

                m_nextItemButton->setToolTip( "Next" );
            }

            m_layout->addWidget( m_previousItemButton );
            m_layout->addWidget( m_nextItemButton );

            {
                QIcon toolButtonIcon;
                if ( !m_attributes.previousIcon.isNull() )
                {
                    toolButtonIcon = m_attributes.previousIcon;
                }
                else
                {
                    toolButtonIcon = UiIconFactory::stepUpIcon();
                }

                if ( m_comboBox->count() == 0 || m_comboBox->currentIndex() <= 0 )
                {
                    QIcon disabledIcon( toolButtonIcon.pixmap( 16, 16, QIcon::Disabled ) );
                    m_previousItemButton->setIcon( disabledIcon );
                }
                else
                {
                    m_previousItemButton->setIcon( toolButtonIcon );
                }
            }

            {
                QIcon toolButtonIcon;
                if ( !m_attributes.nextIcon.isNull() )
                {
                    toolButtonIcon = m_attributes.nextIcon;
                }
                else
                {
                    toolButtonIcon = UiIconFactory::stepDownIcon();
                }
                if ( m_comboBox->count() == 0 || m_comboBox->currentIndex() >= m_comboBox->count() - 1 )
                {
                    QIcon disabledIcon( toolButtonIcon.pixmap( 16, 16, QIcon::Disabled ) );
                    m_nextItemButton->setIcon( disabledIcon );
                }
                else
                {
                    m_nextItemButton->setIcon( toolButtonIcon );
                }
            }

            // Update button texts
            if ( !m_attributes.nextButtonText.isEmpty() )
            {
                m_nextItemButton->setToolTip( m_attributes.nextButtonText );
            }

            if ( !m_attributes.prevButtonText.isEmpty() )
            {
                m_previousItemButton->setToolTip( m_attributes.prevButtonText );
            }
        }
        else
        {
            if ( m_previousItemButton )
            {
                m_layout->removeWidget( m_previousItemButton );
                m_previousItemButton->deleteLater();
            }

            if ( m_nextItemButton )
            {
                m_layout->removeWidget( m_nextItemButton );
                m_nextItemButton->deleteLater();
            }
        }
    }

    if ( uiField()->isAutoValueEnabled() )
    {
        QString highlightColor = UiAppearanceSettings::instance()->autoValueEditorColor();
        m_comboBox->setStyleSheet( QString( "QComboBox {background-color: %1;}" ).arg( highlightColor ) );
    }
    else
    {
        m_comboBox->setStyleSheet( "" );
    }

    if ( uiField()->isAutoValueSupported() )
    {
        auto icon = UiIconFactory::createTwoStateChainIcon();
        m_autoValueToolButton->setIcon( icon );

        m_autoValueToolButton->setChecked( uiField()->isAutoValueEnabled() );
        QString tooltipText = uiField()->isAutoValueEnabled() ? UiAppearanceSettings::globaleValueButtonText()
                                                              : UiAppearanceSettings::localValueButtonText();
        m_autoValueToolButton->setToolTip( tooltipText );
        m_layout->addWidget( m_autoValueToolButton );
        m_autoValueToolButton->show();
    }
    else
    {
        m_layout->removeWidget( m_autoValueToolButton );
        m_autoValueToolButton->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins PdmUiComboBoxEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_comboBox->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int   heightDiff = editorSize.height() - labelSize.height();

    QMargins contentMargins = m_label->contentsMargins();
    if ( heightDiff > 0 )
    {
        contentMargins.setTop( contentMargins.top() + heightDiff / 2 );
        contentMargins.setBottom( contentMargins.bottom() + heightDiff / 2 );
    }
    return contentMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiComboBoxEditor::PdmUiComboBoxEditor()
    : m_interactiveEditCursorPosition( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiComboBoxEditor::createEditorWidget( QWidget* parent )
{
    m_comboBox = new CustomQComboBox( parent );
    m_comboBox->setFocusPolicy( Qt::StrongFocus );

    m_placeholder = new QWidget( parent );

    m_layout = new QHBoxLayout( m_placeholder );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_layout->setSpacing( 0 );
    m_layout->addWidget( m_comboBox );

    connect( m_comboBox, SIGNAL( activated( int ) ), this, SLOT( slotIndexActivated( int ) ) );
    connect( m_comboBox, SIGNAL( signalFocusOutEvent() ), this, SLOT( slotSetCurrentTextToField() ) );

    m_autoValueToolButton = new QToolButton( m_placeholder );
    m_autoValueToolButton->setCheckable( true );
    m_autoValueToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );

    connect( m_autoValueToolButton, SIGNAL( clicked() ), this, SLOT( slotApplyAutoValue() ) );

    // Forward focus event to combo box editor
    m_placeholder->setFocusProxy( m_comboBox );

    return m_placeholder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotIndexActivated( int index )
{
    if ( !uiField() ) return;

    uiField()->enableAutoValue( false );

    if ( m_attributes.enableEditableContent )
    {
        // Use the text directly, as the item text could be entered directly by the user

        auto text = m_comboBox->itemText( index );
        this->setValueToField( text );
    }
    else
    {
        // Use index as data carrier to PDM field
        // The index will be used as a lookup in a list of option items

        QVariant v;
        v = index;

        QVariant uintValue( v.toUInt() );
        this->setValueToField( uintValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotEditTextChanged( const QString& text )
{
    if ( !uiField() ) return;

    if ( text == m_interactiveEditText ) return;

    uiField()->enableAutoValue( false );

    m_interactiveEditText           = text;
    m_interactiveEditCursorPosition = m_comboBox->lineEdit()->cursorPosition();

    this->setValueToField( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotSetCurrentTextToField()
{
    if ( m_attributes.enableEditableContent )
    {
        // Use the text directly, as the item text could be entered directly by the user

        auto text = m_comboBox->currentText();
        this->setValueToField( text );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotNextButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() + 1;

    if ( indexCandidate < m_comboBox->count() )
    {
        slotIndexActivated( indexCandidate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotPreviousButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() - 1;

    if ( indexCandidate > -1 && indexCandidate < m_comboBox->count() )
    {
        slotIndexActivated( indexCandidate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotApplyAutoValue()
{
    if ( !uiField() ) return;

    bool enable = m_autoValueToolButton->isChecked();
    uiField()->enableAutoValue( enable );
    configureAndUpdateUi( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CustomQComboBox::CustomQComboBox( QWidget* parent /*= nullptr */ )
    : QComboBox( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CustomQComboBox::wheelEvent( QWheelEvent* e )
{
    if ( hasFocus() )
    {
        QComboBox::wheelEvent( e );
    }
    else
    {
        // Ignore the event to make sure event is handled by another widget
        e->ignore();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CustomQComboBox::focusInEvent( QFocusEvent* e )
{
    setFocusPolicy( Qt::WheelFocus );
    QComboBox::focusInEvent( e );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CustomQComboBox::focusOutEvent( QFocusEvent* e )
{
    setFocusPolicy( Qt::StrongFocus );
    QComboBox::focusOutEvent( e );

    emit signalFocusOutEvent();
}

} // end namespace caf
