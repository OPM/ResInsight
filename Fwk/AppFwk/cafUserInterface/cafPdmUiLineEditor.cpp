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

#include "cafPdmUiLineEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUniqueIdValidator.h"
#include "cafSelectionManager.h"
#include "cafUiAppearanceSettings.h"
#include "cafUiIconFactory.h"

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QApplication>
#include <QBitmap>
#include <QCompleter>
#include <QDebug>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>
#include <QStringListModel>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::updateLineEditFromReadOnlyState( QLineEdit* lineEdit, bool isReadOnly )
{
    if ( !lineEdit ) return;

    if ( isReadOnly )
    {
        lineEdit->setReadOnly( true );
    }
    else
    {
        lineEdit->setReadOnly( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createEditorWidget( QWidget* parent )
{
    m_lineEdit = new PdmUiLineEdit( parent, false );

    auto isOptionalField = []( caf::PdmUiFieldHandle* uiFieldHandle )
    {
        if ( !uiFieldHandle ) return false;

        if ( auto fieldHandle = uiFieldHandle->fieldHandle() )
        {
            const std::type_info& typeInfo = typeid( *fieldHandle );
            QString               typeName = QString( typeInfo.name() );

            return typeName.contains( "optional", Qt::CaseInsensitive );
        }
        return false;
    };

    if ( isOptionalField( uiField() ) )
    {
        auto icon   = UiIconFactory::informationIcon();
        auto action = m_lineEdit->addAction( icon, QLineEdit::TrailingPosition );
        action->setToolTip( "This field is optional" );
    }

    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    m_placeholder = new QWidget( parent );
    m_layout      = new QHBoxLayout( m_placeholder );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_layout->setSpacing( 0 );
    m_layout->addWidget( m_lineEdit );

    m_autoValueToolButton = new QToolButton( m_placeholder );
    m_autoValueToolButton->setCheckable( true );
    m_autoValueToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );

    connect( m_autoValueToolButton, SIGNAL( clicked() ), this, SLOT( slotApplyAutoValue() ) );

    if ( uiField()->isAutoValueSupported() )
    {
        // If we return the placeholder widget to be used in a table editor, Qt will crash when the editor is closed
        m_placeholder->show();
        return m_placeholder;
    }

    m_placeholder->hide();

    // Return line edit if no auto value is supported
    return m_lineEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    if ( !m_label.isNull() )
    {
        PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );
    }

    if ( !m_lineEdit.isNull() )
    {
        updateLineEditFromReadOnlyState( m_lineEdit, uiField()->isUiReadOnly( uiConfigName ) );

        m_lineEdit->setToolTip( uiField()->uiToolTip( uiConfigName ) );

        PdmUiLineEditorAttribute leab;
        {
            caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
            if ( uiObject )
            {
                uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &leab );
            }

            if ( uiField()->isAutoValueEnabled() )
            {
                QString highlightColor = UiAppearanceSettings::instance()->autoValueEditorColor();
                m_lineEdit->setStyleSheet( QString( "QLineEdit {background-color: %1;}" ).arg( highlightColor ) );
            }
            else if ( uiField()->isUiReadOnly() )
            {
                m_lineEdit->setStyleSheet( "QLineEdit:read-only{background: palette(window);}" );
            }
            else
            {
                m_lineEdit->setStyleSheet( "" );
            }

            if ( uiField()->isAutoValueSupported() )
            {
                auto icon = UiIconFactory::twoStateChainIcon();
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

            if ( leab.validator )
            {
                m_lineEdit->setValidator( leab.validator );
            }

            m_lineEdit->setAvoidSendingEnterEventToParentWidget( leab.avoidSendingEnterEventToParentWidget );

            if ( leab.maximumWidth != -1 )
            {
                m_lineEdit->setMaximumWidth( leab.maximumWidth );
            }

            if ( !leab.placeholderText.isEmpty() )
            {
                m_lineEdit->setPlaceholderText( leab.placeholderText );
            }

            if ( leab.notifyWhenTextIsEdited )
            {
                connect( m_lineEdit, SIGNAL( textEdited( const QString& ) ), this, SLOT( slotEditingFinished() ) );
            }
        }

        m_optionCache = uiField()->valueOptions();
        if ( !m_optionCache.isEmpty() )
        {
            if ( !m_completer )
            {
                m_completer         = new QCompleter( this );
                m_completerTextList = new QStringListModel( this );

                m_completer->setModel( m_completerTextList );
                m_completer->setFilterMode( leab.completerFilterMode );
                m_completer->setCaseSensitivity( leab.completerCaseSensitivity );

                m_lineEdit->setCompleter( m_completer );
                connect( m_completer,
                         SIGNAL( activated( const QModelIndex& ) ),
                         this,
                         SLOT( slotCompleterActivated( const QModelIndex& ) ) );
                m_completer->popup()->installEventFilter( this );
            }

            QStringList optionNames;
            for ( const PdmOptionItemInfo& item : m_optionCache )
            {
                optionNames.push_back( item.optionUiText() );
            }

            m_completerTextList->setStringList( optionNames );

            int enumValue = uiField()->uiValue().toInt();

            if ( enumValue < m_optionCache.size() && enumValue > -1 )
            {
                m_lineEdit->setText( m_optionCache[enumValue].optionUiText() );
            }
        }
        else
        {
            m_lineEdit->setCompleter( nullptr );
            delete m_completerTextList;
            delete m_completer;
            m_optionCache.clear();

            PdmUiLineEditorAttributeUiDisplayString displayStringAttrib;
            caf::PdmUiObjectHandle*                 uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
            if ( uiObject )
            {
                uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &displayStringAttrib );
            }

            QString displayString;
            if ( displayStringAttrib.m_displayString.isEmpty() )
            {
                displayString = uiField()->uiValue().toString();
            }
            else
            {
                displayString = displayStringAttrib.m_displayString;
            }

            if ( displayString != m_lineEdit->text() )
            {
                m_lineEdit->setText( displayString );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins PdmUiLineEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_lineEdit->sizeHint();
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
void PdmUiLineEditor::slotEditingFinished()
{
    QVariant v;

    uiField()->enableAutoValue( false );

    if ( !m_optionCache.empty() )
    {
        int index = findIndexToOption( m_lineEdit->text() );
        if ( index > -1 )
        {
            v = QVariant( static_cast<unsigned int>( index ) );
            this->setValueToField( v );
        }
        else
        {
            // Try to complete the text in the widget

            QModelIndex sourceindex =
                static_cast<QAbstractProxyModel*>( m_completer->completionModel() )->mapToSource( m_completer->currentIndex() );

            if ( sourceindex.isValid() )
            {
                int currentRow = sourceindex.row();
                {
                    // If the existing value in the field is the same as the completer will hit, we need to echo the
                    // choice into the text field because the field values are equal, so the normal echoing is
                    // considered unneccessary by the caf system.
                    int currentFieldIndexValue = uiField()->uiValue().toInt();
                    if ( currentRow == currentFieldIndexValue )
                    {
                        m_lineEdit->setText(
                            m_completer->completionModel()->data( m_completer->currentIndex() ).toString() );
                    }
                }

                v = QVariant( static_cast<unsigned int>( sourceindex.row() ) );
                this->setValueToField( v );
            }
            else
            {
                // Revert to value stored in the PdmField, because we didn't find any matches
                this->updateUi();
            }
        }
    }
    else
    {
        QString textValue = m_lineEdit->text();
        v                 = textValue;
        this->setValueToField( v );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiLineEditor::isMultipleFieldsWithSameKeywordSelected( PdmFieldHandle* editorField ) const
{
    std::vector<PdmFieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back( editorField );

    // For current selection, find all fields with same keyword
    const auto items = SelectionManager::instance()->selectedItems( SelectionManager::FIRST_LEVEL );

    for ( size_t i = 0; i < items.size(); i++ )
    {
        PdmUiFieldHandle* uiField = dynamic_cast<PdmUiFieldHandle*>( items[i] );
        if ( !uiField ) continue;

        PdmFieldHandle* field = uiField->fieldHandle();
        if ( field && field != editorField && field->keyword() == editorField->keyword() )
        {
            fieldsToUpdate.push_back( field );
        }
    }

    if ( fieldsToUpdate.size() > 1 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiLineEdit::PdmUiLineEdit( QWidget* parent, bool avoidSendingEnterEvent )
    : QLineEdit( parent )
    , m_avoidSendingEnterEvent( avoidSendingEnterEvent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEdit::setAvoidSendingEnterEventToParentWidget( bool avoidSendingEnterEvent )
{
    m_avoidSendingEnterEvent = avoidSendingEnterEvent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEdit::keyPressEvent( QKeyEvent* event )
{
    QLineEdit::keyPressEvent( event );
    if ( m_avoidSendingEnterEvent )
    {
        if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        {
            // accept enter/return events so they won't
            // be ever propagated to the parent dialog..
            event->accept();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Event filter filtering events to the QCompleter
//--------------------------------------------------------------------------------------------------
bool PdmUiLineEditor::eventFilter( QObject* watched, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>( event );

        if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter )
        {
            m_ignoreCompleterActivated = true;
            this->m_completer->popup()->close();
            this->slotEditingFinished();
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::slotCompleterActivated( const QModelIndex& index )
{
    if ( m_completer && !m_ignoreCompleterActivated )
    {
        slotEditingFinished();
    }

    m_ignoreCompleterActivated = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiLineEditor::slotApplyAutoValue()
{
    bool enable = m_autoValueToolButton->isChecked();
    uiField()->enableAutoValue( enable );
    configureAndUpdateUi( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiLineEditor::findIndexToOption( const QString& uiText )
{
    QString uiTextTrimmed = uiText.trimmed();
    for ( int idx = 0; idx < m_optionCache.size(); ++idx )
    {
        if ( uiTextTrimmed == m_optionCache[idx].optionUiText() )
        {
            return idx;
        }
    }

    return -1;
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire
// build. The variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at
// the same line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiLineEditor );

} // end namespace caf
