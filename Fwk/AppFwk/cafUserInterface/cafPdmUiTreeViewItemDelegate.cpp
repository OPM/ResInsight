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

#include "cafPdmUiTreeViewItemDelegate.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldReorderCapability.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayFieldHandle.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDragDropInterface.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewEditor.h"
#include "cafPdmUiTreeViewQModel.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QModelIndexList>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTreeView>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewItemDelegate::PdmUiTreeViewItemDelegate( PdmUiTreeViewEditor* parent )
    : QStyledItemDelegate( parent->treeView() )
    , m_treeView( parent )
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

    if ( tagIndex < it->second.size() )
    {
        const PdmUiTreeViewItemAttribute::Tag* tag = it->second[tagIndex].get();
        return tag->rect;
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
            if ( tag->position == PdmUiTreeViewItemAttribute::Tag::Position::AT_END )
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
            tag->rect = iconRect;
        }
        else
        {
            const int insideTopBottomMargins  = 1;
            const int insideleftRightMargins  = 6;
            const int outsideLeftRightMargins = 4;

            QFont font = QApplication::font();
            font.setPointSize( std::max( 1, font.pointSize() - 1 ) );
            painter->setFont( font );

            QString text    = tag->text;
            QColor  bgColor = tag->bgColor;
            QColor  fgColor = tag->fgColor;

            QSize textSize( QFontMetrics( font ).size( Qt::TextSingleLine, text ) );
            int   textDiff = ( fullSize.height() - textSize.height() );

            QRect textRect;
            if ( tag->position == PdmUiTreeViewItemAttribute::Tag::Position::AT_END )
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
            tag->rect = tagRect;
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
        auto* mouseEvent = static_cast<QMouseEvent*>( event );
        if ( mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::NoModifier )
        {
            const PdmUiTreeViewItemAttribute::Tag* tag;
            if ( tagClicked( mouseEvent->pos(), option.rect, itemIndex, &tag ) )
            {
                auto uiItem       = m_treeView->uiItemFromModelIndex( itemIndex );
                auto parentIndex  = itemIndex.parent();
                auto parentUiItem = m_treeView->uiItemFromModelIndex( parentIndex );

                auto* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>( uiItem );
                if ( uiObjectHandle )
                {
                    PdmObjectHandle* pdmObject = uiObjectHandle->objectHandle();
                    if ( pdmObject )
                    {
                        size_t indexInParent = 0;

                        if ( PdmFieldReorderCapability* reorderability =
                                 PdmFieldReorderCapability::reorderCapabilityOfParentContainer( pdmObject ) )
                        {
                            indexInParent = reorderability->indexOf( pdmObject );
                        }
                        tag->clicked.send( indexInParent );
                    }
                }

                bool notifyEditors = true;
                m_treeView->updateSubTree( parentUiItem, notifyEditors );
                m_treeView->selectAsCurrentItem( uiItem );

                return true;
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

} // namespace caf
