//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUiCheckBoxDelegate.h"

#include <QApplication>
#include <QMouseEvent>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiCheckBoxDelegate::PdmUiCheckBoxDelegate( QObject* pParent )
    : QStyledItemDelegate( pParent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiCheckBoxDelegate::~PdmUiCheckBoxDelegate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect adjustedPaintRect( const QStyleOptionViewItem& option )
{
    const int margin = QApplication::style()->pixelMetric( QStyle::PM_FocusFrameHMargin ) + 1;

    QRect newRect =
        QStyle::alignedRect( option.direction,
                             Qt::AlignCenter,
                             QSize( option.decorationSize.width() + margin, option.rect.height() ),
                             QRect( option.rect.x(), option.rect.y(), option.rect.width(), option.rect.height() ) );

    return newRect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem viewItemOption( option );

    viewItemOption.rect = adjustedPaintRect( option );

    QStyledItemDelegate::paint( painter, viewItemOption, index );
}

//--------------------------------------------------------------------------------------------------
/// Returns true to avoid other factories to produce editors for a check box
//--------------------------------------------------------------------------------------------------
bool PdmUiCheckBoxDelegate::editorEvent( QEvent*                     event,
                                         QAbstractItemModel*         model,
                                         const QStyleOptionViewItem& option,
                                         const QModelIndex&          index )
{
    Q_ASSERT( event );
    Q_ASSERT( model );

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags( index );
    if ( !( flags & Qt::ItemIsUserCheckable ) || !( flags & Qt::ItemIsEnabled ) ) return false;

    // make sure that we have a check state
    QVariant value = index.data( Qt::CheckStateRole );
    if ( !value.isValid() ) return false;

    // make sure that we have the right event type
    if ( event->type() == QEvent::MouseButtonRelease )
    {
        QRect paintRect = adjustedPaintRect( option );
        QRect checkRect = QStyle::alignedRect( option.direction, Qt::AlignCenter, option.decorationSize, paintRect );

        if ( !checkRect.contains( static_cast<QMouseEvent*>( event )->pos() ) ) return true;
    }
    else if ( event->type() == QEvent::KeyPress )
    {
        if ( static_cast<QKeyEvent*>( event )->key() != Qt::Key_Space &&
             static_cast<QKeyEvent*>( event )->key() != Qt::Key_Select )
            return true;
    }
    else
    {
        return false;
    }

    Qt::CheckState state = ( static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    return model->setData( index, state, Qt::CheckStateRole );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize PdmUiCheckBoxDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    return QSize( option.decorationSize.width(), option.decorationSize.height() );
}

} // end namespace caf
