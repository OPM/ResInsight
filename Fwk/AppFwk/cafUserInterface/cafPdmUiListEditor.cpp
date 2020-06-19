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

#include "cafPdmUiListEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"

#include "cafFactory.h"
#include "cafQShortenedLabel.h"

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QStringListModel>
#include <QTimer>

//==================================================================================================
/// Helper class used to override flags to disable editable items
//==================================================================================================
class MyStringListModel : public QStringListModel
{
public:
    explicit MyStringListModel( QObject* parent = nullptr )
        : QStringListModel( parent )
        , m_isItemsEditable( false )
    {
    }

    Qt::ItemFlags flags( const QModelIndex& index ) const override
    {
        if ( m_isItemsEditable )
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        else
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    void setItemsEditable( bool isEditable ) { m_isItemsEditable = isEditable; }

private:
    bool m_isItemsEditable;
};

//==================================================================================================
/// Helper class used to control height of size hint
//==================================================================================================
class QListViewHeightHint : public QListView
{
public:
    explicit QListViewHeightHint( QWidget* parent = nullptr )
        : m_heightHint( -1 )
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QSize sizeHint() const override
    {
        QSize mySize = QListView::sizeHint();

        if ( m_heightHint > 0 )
        {
            mySize.setHeight( m_heightHint );
        }

        return mySize;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void setHeightHint( int heightHint ) { m_heightHint = heightHint; }

private:
    int m_heightHint;
};

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiListEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiListEditor::PdmUiListEditor()
    : m_isEditOperationsAvailable( true )
    , m_optionItemCount( 0 )
    , m_isScrollToItemAllowed( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiListEditor::~PdmUiListEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    // TODO: Fix CAF_ASSERT( here when undoing in testapp
    // See PdmUiComboBoxEditor for pattern
    // This might also apply to other editors

    CAF_ASSERT( !m_listView.isNull() );
    CAF_ASSERT( !m_label.isNull() );
    CAF_ASSERT( m_listView->selectionModel() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_listView->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_listView->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    bool                     optionsOnly = true;
    QList<PdmOptionItemInfo> options     = uiField()->valueOptions( &optionsOnly );
    m_optionItemCount                    = options.size();
    if ( options.size() > 0 || uiField()->isUiReadOnly( uiConfigName ) )
    {
        m_isEditOperationsAvailable = false;
    }
    else
    {
        m_isEditOperationsAvailable = true;
    }

    PdmUiListEditorAttribute attributes;
    caf::PdmUiObjectHandle*  uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), uiConfigName, &attributes );

        QPalette myPalette;

        if ( attributes.m_baseColor == myPalette.color( QPalette::Active, QPalette::Base ) )
        {
            m_listView->setStyleSheet( "" );
        }
        else
        {
            m_listView->setStyleSheet( "background-color: " + attributes.m_baseColor.name() + ";" );
        }

        m_listView->setHeightHint( attributes.m_heightHint );
        if ( !attributes.m_allowHorizontalScrollBar )
        {
            m_listView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        }
    }

    MyStringListModel* strListModel = dynamic_cast<MyStringListModel*>( m_model.data() );

    CAF_ASSERT( strListModel );

    if ( !options.isEmpty() )
    {
        CAF_ASSERT( optionsOnly ); // Handling Additions on the fly not implemented

        strListModel->setItemsEditable( false );
        QModelIndex currentItem = m_listView->selectionModel()->currentIndex();
        QStringList texts       = PdmOptionItemInfo::extractUiTexts( options );
        strListModel->setStringList( texts );

        QVariant fieldValue = uiField()->uiValue();
        if ( fieldValue.type() == QVariant::Int || fieldValue.type() == QVariant::UInt )
        {
            int col = 0;
            int row = uiField()->uiValue().toInt();

            QModelIndex mi = strListModel->index( row, col );

            m_listView->selectionModel()->blockSignals( true );
            m_listView->setSelectionMode( QAbstractItemView::SingleSelection );
            if ( row >= 0 )
            {
                m_listView->selectionModel()->select( mi, QItemSelectionModel::SelectCurrent );
                m_listView->selectionModel()->setCurrentIndex( mi, QItemSelectionModel::SelectCurrent );
            }
            else // A negative value (Undefined UInt ) is interpreted as no selection
            {
                m_listView->selectionModel()->clearSelection();
            }

            m_listView->selectionModel()->blockSignals( false );
        }
        else if ( fieldValue.type() == QVariant::List )
        {
            QList<QVariant> valuesSelectedInField = fieldValue.toList();
            QItemSelection  selection;

            for ( int i = 0; i < valuesSelectedInField.size(); ++i )
            {
                QModelIndex mi = strListModel->index( valuesSelectedInField[i].toInt(), 0 );
                selection.append( QItemSelectionRange( mi ) );
            }

            m_listView->selectionModel()->blockSignals( true );

            m_listView->setSelectionMode( QAbstractItemView::ExtendedSelection );
            m_listView->selectionModel()->select( selection, QItemSelectionModel::Select );
            m_listView->selectionModel()->setCurrentIndex( currentItem, QItemSelectionModel::Current );

            m_listView->selectionModel()->blockSignals( false );
        }
    }
    else
    {
        m_listView->selectionModel()->blockSignals( true );

        QItemSelection selection   = m_listView->selectionModel()->selection();
        QModelIndex    currentItem = m_listView->selectionModel()->currentIndex();
        QVariant       fieldValue  = uiField()->uiValue();
        QStringList    texts       = fieldValue.toStringList();
        texts.push_back( "" );
        strListModel->setStringList( texts );

        strListModel->setItemsEditable( true );

        m_listView->setSelectionMode( QAbstractItemView::ExtendedSelection );
        m_listView->selectionModel()->select( selection, QItemSelectionModel::Select );
        m_listView->selectionModel()->setCurrentIndex( currentItem, QItemSelectionModel::Current );

        m_listView->selectionModel()->blockSignals( false );
    }

    QTimer::singleShot( 150, this, SLOT( slotScrollToSelectedItem() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListEditor::createEditorWidget( QWidget* parent )
{
    m_listView = new QListViewHeightHint( parent );

    m_model = new MyStringListModel( m_listView );
    m_listView->setModel( m_model );

    connect( m_listView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this,
             SLOT( slotSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    connect( m_model,
             SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
             this,
             SLOT( slotListItemEdited( const QModelIndex&, const QModelIndex& ) ) );

    // Used to track key press
    m_listView->installEventFilter( this );

    // Used to track mouse events
    m_listView->viewport()->installEventFilter( this );

    return m_listView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QShortenedLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
    if ( m_optionItemCount == 0 ) return;

    m_isScrollToItemAllowed = false;

    QVariant fieldValue = uiField()->uiValue();
    if ( fieldValue.type() == QVariant::Int || fieldValue.type() == QVariant::UInt )
    {
        // NOTE : Workaround for update issue seen on RHEL6 with Qt 4.6.2
        // An invalid call to setSelection() from QAbstractItemView::keyPressEvent() causes the stepping using arrow
        // keys in a single selection list to not work as expected.
        //
        // See also https://github.com/OPM/ResInsight/issues/879
        //
        // WORKAROUND : The list view is in single selection mode, and the selection is set based on current index
        m_listView->selectionModel()->select( m_listView->currentIndex(), QItemSelectionModel::SelectCurrent );

        QModelIndexList idxList = m_listView->selectionModel()->selectedIndexes();
        if ( idxList.size() >= 1 )
        {
            if ( idxList[0].row() < m_optionItemCount )
            {
                this->setValueToField( QVariant( static_cast<unsigned int>( idxList[0].row() ) ) );
            }
        }
    }
    else if ( fieldValue.type() == QVariant::List )
    {
        QModelIndexList idxList = m_listView->selectionModel()->selectedIndexes();

        QList<QVariant> valuesSelectedInField = fieldValue.toList();

        if ( idxList.size() == 1 && valuesSelectedInField.size() == 1 )
        {
            // NOTE : Workaround for update issue seen on RHEL6 with Qt 4.6.2
            // An invalid call to setSelection() from QAbstractItemView::keyPressEvent() causes the stepping using arrow
            // keys in a multi selection list to not work as expected.
            //
            // See also https://github.com/OPM/ResInsight/issues/879
            //
            // WORKAROUND : The list view has one or none items selected, manually set the selection from current index
            m_listView->selectionModel()->select( m_listView->currentIndex(), QItemSelectionModel::SelectCurrent );

            // Update the list of indexes after selection has been modified
            idxList = m_listView->selectionModel()->selectedIndexes();
        }

        QList<QVariant> valuesToSetInField;
        for ( int i = 0; i < idxList.size(); ++i )
        {
            if ( idxList[i].row() < m_optionItemCount )
            {
                valuesToSetInField.push_back( QVariant( static_cast<unsigned int>( idxList[i].row() ) ) );
            }
        }

        this->setValueToField( valuesToSetInField );
    }

    m_isScrollToItemAllowed = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::slotListItemEdited( const QModelIndex&, const QModelIndex& )
{
    CAF_ASSERT( m_isEditOperationsAvailable );

    QStringList uiList = m_model->stringList();

    trimAndSetValuesToField( uiList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::slotScrollToSelectedItem() const
{
    if ( m_isScrollToItemAllowed )
    {
        QModelIndex mi = m_listView->currentIndex();
        if ( mi.isValid() )
        {
            m_listView->scrollTo( mi );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiListEditor::contentAsString() const
{
    QString str;

    if ( m_model )
    {
        QStringList uiList = m_model->stringList();

        str = uiList.join( "\n" );
    }

    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::pasteFromString( const QString& content )
{
    QStringList strList = content.split( "\n" );

    trimAndSetValuesToField( strList );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::trimAndSetValuesToField( const QStringList& stringList )
{
    QStringList result;
    for ( const auto& str : stringList )
    {
        if ( str != "" && str != " " ) result += str;
    }

    this->setValueToField( result );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiListEditor::eventFilter( QObject* object, QEvent* event )
{
    if ( !m_listView )
    {
        return false;
    }
    if ( object == m_listView->viewport() && event->type() == QEvent::MouseMove )
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>( event );
        if ( mouseEvent )
        {
            if ( mouseEvent->buttons() & Qt::LeftButton && mouseEvent->modifiers() & Qt::ControlModifier )
            {
                // When Ctrl button is pressed, left mouse button is pressed, and the mouse is moving,
                // a possible bug in Qt is observed causing the selection to end up with single item selection
                // When returning here without doing anything, system behaves as expected

                // NOTE: The mouse event is handled by the viewport() of the list view, not the list view itself

                return true;
            }
        }
    }

    if ( object == m_listView && event->type() == QEvent::KeyPress )
    {
        QKeyEvent* keyEv = static_cast<QKeyEvent*>( event );

        if ( m_isEditOperationsAvailable )
        {
            if ( keyEv->key() == Qt::Key_Delete || keyEv->key() == Qt::Key_Backspace )
            {
                QModelIndexList idxList      = m_listView->selectionModel()->selectedIndexes();
                bool            isAnyDeleted = false;
                while ( idxList.size() )
                {
                    m_model->removeRow( idxList[0].row() );
                    idxList      = m_listView->selectionModel()->selectedIndexes();
                    isAnyDeleted = true;
                }

                if ( isAnyDeleted )
                {
                    QStringList uiList = m_model->stringList();

                    trimAndSetValuesToField( uiList );
                }
                return true;
            }
        }

        if ( keyEv->modifiers() & Qt::ControlModifier )
        {
            if ( keyEv->key() == Qt::Key_C )
            {
                QClipboard* clipboard = QApplication::clipboard();
                if ( clipboard )
                {
                    QString content = contentAsString();
                    clipboard->setText( content );

                    return true;
                }
            }
            else if ( m_isEditOperationsAvailable && keyEv->key() == Qt::Key_V )
            {
                QClipboard* clipboard = QApplication::clipboard();
                if ( clipboard )
                {
                    QString content = clipboard->text();
                    pasteFromString( content );

                    return true;
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiListEditor::isMultiRowEditor() const
{
    return true;
}

} // end namespace caf
