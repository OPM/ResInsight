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
#include "cafPdmUiOrdering.h"
#include "cafPdmUniqueIdValidator.h"
#include "cafQShortenedLabel.h"
#include "cafSelectionManager.h"

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QApplication>
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
QWidget* PdmUiLineEditor::createEditorWidget( QWidget* parent )
{
    m_lineEdit = new PdmUiLineEdit( parent );

    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    return m_lineEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiLineEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
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
        bool isReadOnly = uiField()->isUiReadOnly( uiConfigName );
        if ( isReadOnly )
        {
            m_lineEdit->setReadOnly( true );

            m_lineEdit->setStyleSheet( "QLineEdit {"
                                       "color: #808080;"
                                       "background-color: #F0F0F0;}" );
        }
        else
        {
            m_lineEdit->setReadOnly( false );
            m_lineEdit->setStyleSheet( "" );
        }

        m_lineEdit->setToolTip( uiField()->uiToolTip( uiConfigName ) );

        PdmUiLineEditorAttribute leab;
        {
            caf::PdmUiObjectHandle* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
            if ( uiObject )
            {
                uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &leab );
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
        }

        bool fromMenuOnly = true;
        m_optionCache     = uiField()->valueOptions( &fromMenuOnly );
        CAF_ASSERT( fromMenuOnly ); // Not supported

        if ( !m_optionCache.isEmpty() && fromMenuOnly == true )
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
#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) && QT_VERSION < QT_VERSION_CHECK( 5, 9, 0 ) )
                bool   valueOk = false;
                double value   = uiField()->uiValue().toDouble( &valueOk );
                if ( valueOk )
                {
                    // Workaround for issue seen on Qt 5.6.1 on Linux
                    int precision = 8;
                    displayString = QString::number( value, 'g', precision );
                }
                else
                {
                    displayString = uiField()->uiValue().toString();
                }
#else
                displayString = uiField()->uiValue().toString();
#endif
            }
            else
            {
                displayString = displayStringAttrib.m_displayString;
            }

            m_lineEdit->setText( displayString );
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

    if ( m_optionCache.size() )
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
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems( items, SelectionManager::FIRST_LEVEL );

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
PdmUiLineEdit::PdmUiLineEdit( QWidget* parent )
    : QLineEdit( parent )
    , m_avoidSendingEnterEvent( false )
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

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build.
// The variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same
// line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiLineEditor );

} // end namespace caf
