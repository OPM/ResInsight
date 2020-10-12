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
#include "cafPdmUiTreeViewQModel.h"
#include "cafSelectionManager.h"

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::~PdmUiTreeViewEditor()
{
    m_treeViewModel->setPdmItemRoot( nullptr );
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
    m_treeView      = new PdmUiTreeViewWidget( m_mainWidget );
    m_treeView->setModel( m_treeViewModel );
    m_treeView->installEventFilter( this );

    m_delegate = new PdmUiTreeViewItemDelegate( this, m_treeViewModel );

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

    for ( int i = 0; i < idxList.size(); i++ )
    {
        caf::PdmUiItem* item = this->m_treeViewModel->uiItemFromModelIndex( idxList[i] );
        if ( item )
        {
            objects.push_back( item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateMySubTree( PdmUiItem* uiItem )
{
    if ( m_treeViewModel )
    {
        m_treeViewModel->updateSubTree( uiItem );
        QModelIndex index = m_treeViewModel->findModelIndex( uiItem );
        updateItemDelegateForSubTree( index );
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

    if ( menu.actions().size() > 0 )
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
    QModelIndex currentIndex = m_treeView->currentIndex();

    m_treeView->clearSelection();

    m_treeView->setCurrentIndex( index );
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
    QModelIndex index = m_treeViewModel->findModelIndex( uiItem );
    m_treeView->setExpanded( index, doExpand );

    if ( doExpand )
    {
        m_treeView->scrollTo( index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeViewEditor::uiItemFromModelIndex( const QModelIndex& index ) const
{
    return m_treeViewModel->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewEditor::findModelIndex( const PdmUiItem* object ) const
{
    return m_treeViewModel->findModelIndex( object );
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
bool PdmUiTreeViewEditor::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::FocusIn )
    {
        this->updateSelectionManager();
    }

    // standard event processing
    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateSelectionManager()
{
    if ( m_updateSelectionManager )
    {
        std::vector<PdmUiItem*> items;
        this->selectedUiItems( items );
        SelectionManager::instance()->setSelectedItems( items );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateItemDelegateForSubTree( const QModelIndex& modelIndex /*= QModelIndex()*/ )
{
    auto allIndices = m_treeViewModel->allIndicesRecursive();
    for ( QModelIndex index : allIndices )
    {
        m_delegate->clearTags( index );

        PdmUiItem*         uiItem         = m_treeViewModel->uiItemFromModelIndex( index );
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( uiItem );

        if ( uiObjectHandle )
        {
            PdmObjectHandle* pdmObject = uiObjectHandle->objectHandle();
            if ( pdmObject )
            {
                PdmFieldReorderCapability* reorderability =
                    PdmFieldReorderCapability::reorderCapabilityOfParentContainer( pdmObject );

                std::vector<PdmUiItem*> selection;
                selectedUiItems( selection );

                if ( reorderability && index.row() >= 0 && selection.size() == 1u && selection.front() == uiItem )
                {
                    size_t indexInParent = static_cast<size_t>( index.row() );
                    {
                        auto tag          = PdmUiTreeViewItemAttribute::Tag::create();
                        tag->icon         = caf::IconProvider( ":/caf/Up16x16.png" );
                        tag->selectedOnly = true;
                        if ( reorderability->canItemBeMovedUp( indexInParent ) )
                        {
                            tag->clicked.connect( reorderability, &PdmFieldReorderCapability::onMoveItemUp );
                        }
                        else
                        {
                            tag->icon.setActive( false );
                        }

                        m_delegate->addTag( index, std::move( tag ) );
                    }
                    {
                        auto tag          = PdmUiTreeViewItemAttribute::Tag::create();
                        tag->icon         = IconProvider( ":/caf/Down16x16.png" );
                        tag->selectedOnly = true;
                        if ( reorderability->canItemBeMovedDown( indexInParent ) )
                        {
                            tag->clicked.connect( reorderability, &PdmFieldReorderCapability::onMoveItemDown );
                        }
                        else
                        {
                            tag->icon.setActive( false );
                        }
                        m_delegate->addTag( index, std::move( tag ) );
                    }
                }
            }

            PdmUiTreeViewItemAttribute attribute;
            uiObjectHandle->objectEditorAttribute( "", &attribute );
            for ( auto& tag : attribute.tags )
            {
                if ( !tag->text.isEmpty() || tag->icon.valid() )
                {
                    m_delegate->addTag( index, std::move( tag ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewItemDelegate::PdmUiTreeViewItemDelegate( PdmUiTreeViewEditor* parent, PdmUiTreeViewQModel* model )
    : QStyledItemDelegate( parent->treeView() )
    , m_treeView( parent )
    , m_model( model )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::clearTags( QModelIndex index )
{
    m_tags.erase( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::clearAllTags()
{
    m_tags.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::addTag( QModelIndex index, std::unique_ptr<PdmUiTreeViewItemAttribute::Tag> tag )
{
    std::vector<std::unique_ptr<PdmUiTreeViewItemAttribute::Tag>>& tagList = m_tags[index];
    tagList.push_back( std::move( tag ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect PdmUiTreeViewItemDelegate::tagRect( const QRect& itemRect, QModelIndex index, size_t tagIndex ) const
{
    auto it = m_tags.find( index );
    if ( it == m_tags.end() ) return QRect();

    QSize fullSize = itemRect.size();

    QPoint offset( 0, 0 );

    for ( size_t i = 0; i < it->second.size(); ++i )
    {
        const PdmUiTreeViewItemAttribute::Tag* tag = it->second[i].get();
        if ( tag->icon.valid() )
        {
            auto  icon     = tag->icon.icon();
            QSize iconSize = icon->actualSize( fullSize );
            QRect iconRect;
            if ( tag->position == PdmUiTreeViewItemAttribute::Tag::AT_END )
            {
                QPoint bottomRight = itemRect.bottomRight() - offset;
                QPoint topLeft     = bottomRight - QPoint( iconSize.width(), iconSize.height() );
                iconRect           = QRect( topLeft, bottomRight );
            }
            else
            {
                QPoint topLeft     = itemRect.topLeft() + offset;
                QPoint bottomRight = topLeft + QPoint( iconSize.width(), iconSize.height() );
                iconRect           = QRect( topLeft, bottomRight );
            }
            offset += QPoint( iconSize.width() + 2, 0 );

            if ( i == tagIndex ) return iconRect;
        }
    }
    return QRect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyledItemDelegate::paint( painter, option, index );

    auto it = m_tags.find( index );
    if ( it == m_tags.end() ) return;

    // Save painter so we can restore it
    painter->save();

    QRect  rect   = option.rect;
    QPoint center = rect.center();

    QSize fullSize = rect.size();

    QPoint offset( 0, 0 );

    for ( const std::unique_ptr<PdmUiTreeViewItemAttribute::Tag>& tag : it->second )
    {
        if ( tag->selectedOnly && !( option.state & QStyle::State_Selected ) ) continue;

        if ( tag->icon.valid() )
        {
            auto  icon     = tag->icon.icon();
            QSize iconSize = icon->actualSize( fullSize );
            QRect iconRect;
            if ( tag->position == PdmUiTreeViewItemAttribute::Tag::AT_END )
            {
                QPoint bottomRight( rect.bottomRight().x() - offset.x(), center.y() + iconSize.height() / 2 );
                QPoint topLeft( bottomRight.x() - iconSize.width(), bottomRight.y() - iconSize.height() );
                iconRect = QRect( topLeft, bottomRight );
            }
            else
            {
                QPoint topLeft( rect.topLeft().x() + offset.x(), center.y() - iconSize.height() / 2 );
                QPoint bottomRight( topLeft.x() + iconSize.width(), topLeft.y() + iconSize.height() );
                iconRect = QRect( topLeft, bottomRight );
            }
            offset += QPoint( iconSize.width() + 2, 0 );
            icon->paint( painter, iconRect );
        }
        else
        {
            const int insideTopBottomMargins  = 1;
            const int insideleftRightMargins  = 6;
            const int outsideLeftRightMargins = 4;

            QFont font = QApplication::font();
            if ( font.pixelSize() > 0 )
            {
                font.setPixelSize( std::max( 1, font.pixelSize() - 1 ) );
            }
            else
            {
                font.setPointSize( std::max( 1, font.pointSize() - 1 ) );
            }
            painter->setFont( font );

            QString text    = tag->text;
            QColor  bgColor = tag->bgColor;
            QColor  fgColor = tag->fgColor;

            QSize textSize( QFontMetrics( font ).size( Qt::TextSingleLine, text ) );
            int   textDiff = ( fullSize.height() - textSize.height() );

            QRect textRect;
            if ( tag->position == PdmUiTreeViewItemAttribute::Tag::AT_END )
            {
                QPoint bottomRight     = rect.bottomRight() - QPoint( outsideLeftRightMargins, 0 ) - offset;
                QPoint textBottomRight = bottomRight - QPoint( insideleftRightMargins, textDiff / 2 );
                QPoint textTopLeft     = textBottomRight - QPoint( textSize.width(), textSize.height() );
                textRect               = QRect( textTopLeft, textBottomRight );
            }
            else
            {
                QPoint textTopLeft = QPoint( 0, rect.topLeft().y() ) + offset +
                                     QPoint( outsideLeftRightMargins + insideleftRightMargins, +textDiff / 2 );
                QPoint textBottomRight = textTopLeft + QPoint( textSize.width(), textSize.height() );
                textRect               = QRect( textTopLeft, textBottomRight );
            }

            QRect tagRect = textRect.marginsAdded(
                QMargins( insideleftRightMargins, insideTopBottomMargins, insideleftRightMargins, insideTopBottomMargins ) );

            offset += QPoint( tagRect.width() + 2, 0 );

            QBrush brush( bgColor );

            painter->setBrush( brush );
            painter->setPen( bgColor );
            painter->setRenderHint( QPainter::Antialiasing );
            const double xRoundingRadiusPercent = 50.0;
            const double yRoundingRadiusPercent = 25.0;
            painter->drawRoundedRect( tagRect, xRoundingRadiusPercent, yRoundingRadiusPercent, Qt::RelativeSize );

            painter->setPen( fgColor );
            painter->drawText( textRect, Qt::AlignCenter, text );
        }
    }
    // Restore painter
    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const caf::PdmUiTreeViewItemAttribute::Tag*> PdmUiTreeViewItemDelegate::tags( QModelIndex index ) const
{
    std::vector<const caf::PdmUiTreeViewItemAttribute::Tag*> tagPtrVector;

    auto it = m_tags.find( index );
    if ( it != m_tags.end() )
    {
        for ( const auto& tag : it->second )
        {
            tagPtrVector.push_back( tag.get() );
        }
    }
    return tagPtrVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewItemDelegate::editorEvent( QEvent*                     event,
                                             QAbstractItemModel*         model,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex&          itemIndex )
{
    if ( event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );

        if ( mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::NoModifier )
        {
            const PdmUiTreeViewItemAttribute::Tag* tag;
            if ( tagClicked( mouseEvent->pos(), option.rect, itemIndex, &tag ) )
            {
                QModelIndex parentIndex = itemIndex.parent();

                auto uiItem       = m_treeView->uiItemFromModelIndex( itemIndex );
                auto parentUiItem = m_treeView->uiItemFromModelIndex( parentIndex );

                tag->clicked.send( (size_t)itemIndex.row() );

                m_treeView->updateSubTree( parentUiItem );
                m_treeView->selectAsCurrentItem( uiItem );
            }
        }
    }

    return QStyledItemDelegate::editorEvent( event, model, option, itemIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewItemDelegate::tagClicked( const QPoint&                           pos,
                                            const QRect&                            itemRect,
                                            const QModelIndex&                      itemIndex,
                                            const PdmUiTreeViewItemAttribute::Tag** tag ) const
{
    if ( itemIndex.isValid() )
    {
        auto itemTags = tags( itemIndex );

        for ( size_t i = 0; i < itemTags.size(); ++i )
        {
            QRect rect = tagRect( itemRect, itemIndex, i );
            if ( rect.contains( pos ) )
            {
                *tag = itemTags[i];
                return true;
            }
        }
    }
    return false;
}
} // end namespace caf
