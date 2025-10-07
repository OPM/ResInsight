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

#include "cafPdmUiTreeViewEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldReorderCapability.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayFieldHandle.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDragDropInterface.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewItemDelegate.h"
#include "cafPdmUiTreeViewQModel.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QDebug>
#include <QDragMoveEvent>
#include <QEvent>
#include <QGridLayout>
#include <QIcon>
#include <QMenu>
#include <QModelIndexList>
#include <QPainter>
#include <QProxyStyle>
#include <QSortFilterProxyModel>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <QWidget>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewStyle::drawPrimitive( QStyle::PrimitiveElement element,
                                        const QStyleOption*      option,
                                        QPainter*                painter,
                                        const QWidget*           widget ) const
{
    if ( element == QStyle::PE_IndicatorItemViewItemDrop )
    {
        painter->setRenderHint( QPainter::Antialiasing, true );

        if ( option->rect.height() == 0 )
        {
            QPalette palette;
            QColor   c = QApplication::palette().color( QPalette::Highlight ).darker( 150 );
            QPen     pen( c );
            pen.setWidth( 2 );
            QBrush brush( c );

            painter->setPen( pen );
            painter->setBrush( brush );

            painter->drawEllipse( option->rect.topLeft(), 3, 3 );
            painter->drawLine( QPoint( option->rect.topLeft().x() + 3, option->rect.topLeft().y() ),
                               option->rect.topRight() );
        }
        else
        {
            QPalette palette;
            QColor   c = QApplication::palette().color( QPalette::Highlight ).darker( 150 );
            QPen     pen( c );
            pen.setWidth( 2 );

            painter->setPen( pen );

            painter->drawRoundedRect( option->rect, 4, 4 );
        }
    }
    else
    {
        QProxyStyle::drawPrimitive( element, option, painter, widget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewWidget::dragMoveEvent( QDragMoveEvent* event )
{
    caf::PdmUiTreeViewQModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewQModel*>( model() );
    if ( treeViewModel && treeViewModel->dragDropInterface() )
    {
        treeViewModel->dragDropInterface()->onProposedDropActionUpdated( event->proposedAction() );
    }

    QTreeView::dragMoveEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
    caf::PdmUiTreeViewQModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewQModel*>( model() );
    if ( treeViewModel && treeViewModel->dragDropInterface() )
    {
        treeViewModel->dragDropInterface()->onDragCanceled();
    }

    QTreeView::dragLeaveEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::PdmUiTreeViewEditor()
{
    m_useDefaultContextMenu       = false;
    m_updateSelectionManager      = false;
    m_appendClassNameToUiItemText = false;
    m_layout                      = nullptr;
    m_treeView                    = nullptr;
    m_treeViewModel               = nullptr;
    m_delegate                    = nullptr;
    m_filterModel                 = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::~PdmUiTreeViewEditor()
{
    m_treeView->removeEventFilter( this );
    m_treeViewModel->setPdmItemRoot( nullptr );

    delete m_mainWidget;
    m_mainWidget = nullptr;

    delete m_delegate;
    m_delegate = nullptr;

    delete m_treeViewModel;
    m_treeViewModel = nullptr;

    delete m_filterModel;
    m_filterModel = nullptr;

    delete m_treeView;
    m_treeView = nullptr;

    delete m_layout;
    m_layout = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeViewEditor::createWidget( QWidget* parent )
{
    m_mainWidget = new QWidget( parent );
    m_layout     = new QVBoxLayout();
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_mainWidget->setLayout( m_layout );

    m_treeViewModel = new caf::PdmUiTreeViewQModel( this );
    m_filterModel   = new QSortFilterProxyModel( this );
    m_filterModel->setFilterKeyColumn( 0 );
    m_filterModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
    m_filterModel->setRecursiveFilteringEnabled( true );

    m_filterModel->setSourceModel( m_treeViewModel );
    m_treeView = new PdmUiTreeViewWidget( m_mainWidget );
    m_treeView->setModel( m_filterModel );
    m_treeView->installEventFilter( this );

    m_delegate = new PdmUiTreeViewItemDelegate( this );

    m_treeView->setItemDelegate( m_delegate );

    connect( treeView()->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( slotOnSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    m_layout->addWidget( m_treeView );

    updateContextMenuSignals();

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    PdmUiTreeViewEditorAttribute editorAttributes;

    {
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( this->pdmItemRoot() );
        if ( uiObjectHandle )
        {
            uiObjectHandle->objectEditorAttribute( uiConfigName, &editorAttributes );
        }
    }

    m_treeViewModel->setColumnHeaders( editorAttributes.columnHeaders );
    m_treeViewModel->setUiConfigName( uiConfigName );
    m_treeViewModel->setPdmItemRoot( this->pdmItemRoot() );

    if ( editorAttributes.currentObject )
    {
        PdmUiObjectHandle* uiObjectHandle = editorAttributes.currentObject->uiCapability();
        if ( uiObjectHandle )
        {
            selectAsCurrentItem( uiObjectHandle );
        }
    }

    if ( m_delegate )
    {
        m_delegate->clearAllTags();
        updateItemDelegateForSubTree();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeViewEditor::treeView()
{
    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QTreeView* PdmUiTreeViewEditor::treeView() const
{
    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::isTreeItemEditWidgetActive() const
{
    return m_treeView->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectedUiItems( std::vector<PdmUiItem*>& objects )
{
    if ( !this->treeView() ) return;

    QModelIndexList idxList = this->treeView()->selectionModel()->selectedIndexes();
    QModelIndexList proxyList;

    for ( int i = 0; i < idxList.size(); i++ )
    {
        proxyList.append( m_filterModel->mapToSource( idxList[i] ) );
    }

    for ( int i = 0; i < proxyList.size(); i++ )
    {
        caf::PdmUiItem* item = this->m_treeViewModel->uiItemFromModelIndex( proxyList[i] );
        if ( item )
        {
            objects.push_back( item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateMySubTree( PdmUiItem* uiItem, bool notifyEditors )
{
    if ( m_treeViewModel )
    {
        // Clear filter, to avoid infinite recursion when updating the tree. This can happen if a search filter is
        // active and a calculation is parsed. This will cause a rebuild of the tree, and infinite recursion if a filter
        // is active.
        // https://github.com/OPM/ResInsight/issues/12983
        m_filterModel->setFilterWildcard( "" );

        PdmUiItem* itemToUpdate = uiItem;

        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( uiItem );
        if ( uiObjectHandle )
        {
            PdmUiTreeViewEditorAttribute editorAttributes;
            QString                      uiConfigName;
            uiObjectHandle->objectEditorAttribute( uiConfigName, &editorAttributes );
            if ( editorAttributes.objectForUpdateOfUiTree )
            {
                itemToUpdate = editorAttributes.objectForUpdateOfUiTree->uiCapability();
            }
        }

        m_treeViewModel->updateSubTree( itemToUpdate, notifyEditors );
        updateItemDelegateForSubTree();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableDefaultContextMenu( bool enable )
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableSelectionManagerUpdating( bool enable )
{
    m_updateSelectionManager = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateContextMenuSignals()
{
    if ( !m_treeView ) return;

    if ( m_useDefaultContextMenu )
    {
        m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( m_treeView, SIGNAL( customContextMenuRequested( QPoint ) ), SLOT( customMenuRequested( QPoint ) ) );
    }
    else
    {
        m_treeView->setContextMenuPolicy( Qt::DefaultContextMenu );
        disconnect( m_treeView, nullptr, this, nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::customMenuRequested( QPoint pos )
{
    // This seems a bit strange. Why ?
    SelectionManager::instance()->setActiveChildArrayFieldHandle( this->currentChildArrayFieldHandle() );

    QMenu menu;
    PdmUiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( m_mainWidget->parentWidget() );

    caf::PdmUiCommandSystemProxy::instance()->populateMenuWithDefaultCommands( "PdmUiTreeViewEditor", &menu );

    if ( !menu.actions().empty() )
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_treeView->viewport()->mapToGlobal( pos );

        menu.exec( globalPos );
    }

    PdmUiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmChildArrayFieldHandle* PdmUiTreeViewEditor::currentChildArrayFieldHandle()
{
    PdmUiItem* currentSelectedItem = SelectionManager::instance()->selectedItem( SelectionManager::FIRST_LEVEL );

    PdmUiFieldHandle* uiFieldHandle = dynamic_cast<PdmUiFieldHandle*>( currentSelectedItem );
    if ( uiFieldHandle )
    {
        PdmFieldHandle* fieldHandle = uiFieldHandle->fieldHandle();

        if ( dynamic_cast<PdmChildArrayFieldHandle*>( fieldHandle ) )
        {
            return dynamic_cast<PdmChildArrayFieldHandle*>( fieldHandle );
        }
    }

    PdmObjectHandle* pdmObject = dynamic_cast<caf::PdmObjectHandle*>( currentSelectedItem );
    if ( pdmObject )
    {
        PdmChildArrayFieldHandle* parentChildArray = dynamic_cast<PdmChildArrayFieldHandle*>( pdmObject->parentField() );

        if ( parentChildArray )
        {
            return parentChildArray;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectAsCurrentItem( const PdmUiItem* uiItem )
{
    QModelIndex index        = m_treeViewModel->findModelIndex( uiItem );
    QModelIndex indexForItem = m_filterModel->mapFromSource( index );

    auto currentSelected = treeView()->currentIndex();

    // Return if index is the same, as resetting the selection causes flickering
    if ( indexForItem == currentSelected ) return;

    m_treeView->clearSelection();

    m_treeView->setCurrentIndex( indexForItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectItems( std::vector<const PdmUiItem*> uiItems )
{
    m_treeView->clearSelection();

    if ( uiItems.empty() )
    {
        return;
    }

    QModelIndex index = findModelIndex( uiItems.back() );
    m_treeView->setCurrentIndex( index );

    for ( const PdmUiItem* uiItem : uiItems )
    {
        QModelIndex itemIndex = findModelIndex( uiItem );
        m_treeView->selectionModel()->select( itemIndex, QItemSelectionModel::Select );
    }
    m_treeView->setFocus( Qt::MouseFocusReason );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::slotOnSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
    this->updateSelectionManager();
    this->updateItemDelegateForSubTree();
    emit selectionChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setExpanded( const PdmUiItem* uiItem, bool doExpand ) const
{
    QModelIndex index       = m_treeViewModel->findModelIndex( uiItem );
    QModelIndex filterIndex = m_filterModel->mapFromSource( index );

    if ( filterIndex.isValid() )
    {
        m_treeView->setExpanded( filterIndex, doExpand );

        if ( doExpand )
        {
            m_treeView->scrollTo( filterIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewEditor::mapIndexIfNecessary( QModelIndex index ) const
{
    const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>( index.model() );

    QModelIndex returnIndex = index;
    if ( proxyModel )
    {
        returnIndex = proxyModel->mapToSource( index );
    }

    return returnIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeViewEditor::uiItemFromModelIndex( const QModelIndex& index ) const
{
    QModelIndex realIndex = mapIndexIfNecessary( index );
    return m_treeViewModel->uiItemFromModelIndex( realIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiTreeViewEditor::uiTreeOrderingFromModelIndex( const QModelIndex& index ) const
{
    QModelIndex realIndex = mapIndexIfNecessary( index );
    if ( realIndex.isValid() ) return static_cast<caf::PdmUiTreeOrdering*>( realIndex.internalPointer() );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewEditor::findModelIndex( const PdmUiItem* object ) const
{
    QModelIndex index = m_treeViewModel->findModelIndex( object );
    return m_filterModel->mapFromSource( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setDragDropInterface( PdmUiDragDropInterface* dragDropInterface )
{
    m_treeViewModel->setDragDropInterface( dragDropInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setFilterString( QString filterString )
{
    m_filterModel->setFilterWildcard( filterString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::FocusIn )
    {
        bool anyChanges = this->updateSelectionManager();
        if ( anyChanges )
        {
            emit selectionChanged();
        }
    }

    // standard event processing
    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::updateSelectionManager()
{
    if ( m_updateSelectionManager )
    {
        std::vector<PdmUiItem*> items;
        this->selectedUiItems( items );
        return SelectionManager::instance()->setSelectedItems( items );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateItemDelegateForSubTree( const QModelIndex& subRootIndex /* = QModelIndex() */ )
{
    auto allIndices = m_treeViewModel->allIndicesRecursive( subRootIndex );
    if ( allIndices.empty() ) return;

    std::vector<PdmUiItem*> selection;
    selectedUiItems( selection );

    for ( QModelIndex& index : allIndices )
    {
        QModelIndex filterIndex = m_filterModel->mapFromSource( index );
        if ( !filterIndex.isValid() ) continue;

        m_delegate->clearTags( filterIndex );

        PdmUiItem*         uiItem         = m_treeViewModel->uiItemFromModelIndex( index );
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( uiItem );

        if ( uiObjectHandle )
        {
            PdmObjectHandle* pdmObject = uiObjectHandle->objectHandle();
            if ( pdmObject )
            {
                PdmFieldReorderCapability* reorderability =
                    PdmFieldReorderCapability::reorderCapabilityOfParentContainer( pdmObject );

                if ( reorderability && filterIndex.row() >= 0 && selection.size() == 1u && selection.front() == uiItem )
                {
                    size_t indexInParentField = reorderability->indexOf( pdmObject );
                    {
                        auto tag          = PdmUiTreeViewItemAttribute::createTag();
                        tag->icon         = caf::IconProvider( ":/caf/Up16x16.png" );
                        tag->selectedOnly = true;
                        if ( reorderability->canItemBeMovedUp( indexInParentField ) )
                        {
                            tag->clicked.connect( reorderability, &PdmFieldReorderCapability::onMoveItemUp );
                        }
                        else
                        {
                            tag->icon.setActive( false );
                        }

                        m_delegate->addTag( filterIndex, std::move( tag ) );
                    }
                    {
                        auto tag          = PdmUiTreeViewItemAttribute::createTag();
                        tag->icon         = IconProvider( ":/caf/Down16x16.png" );
                        tag->selectedOnly = true;
                        if ( reorderability->canItemBeMovedDown( indexInParentField ) )
                        {
                            tag->clicked.connect( reorderability, &PdmFieldReorderCapability::onMoveItemDown );
                        }
                        else
                        {
                            tag->icon.setActive( false );
                        }
                        m_delegate->addTag( filterIndex, std::move( tag ) );
                    }
                }
            }

            PdmUiTreeViewItemAttribute attribute;
            uiObjectHandle->objectEditorAttribute( "", &attribute );
            for ( auto& tag : attribute.tags )
            {
                if ( !tag->text.isEmpty() || tag->icon.valid() )
                {
                    m_delegate->addTag( filterIndex, std::move( tag ) );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_appendClassNameToUiItemText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::isAppendOfClassNameToUiItemTextEnabled()
{
    return m_appendClassNameToUiItemText;
}

} // end namespace caf
