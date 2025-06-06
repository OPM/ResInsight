
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

#pragma once

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeEditorHandle.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QModelIndex>
#include <QPoint>
#include <QRect>
#include <QStyledItemDelegate>

#include <memory>

namespace caf
{
class PdmUiTreeViewEditor;

class PdmUiTreeViewItemDelegate : public QStyledItemDelegate
{
public:
    PdmUiTreeViewItemDelegate( PdmUiTreeViewEditor* parent );
    void clearTags( QModelIndex index );
    void clearAllTags();
    void addTag( QModelIndex index, std::unique_ptr<PdmUiTreeViewItemAttribute::Tag> tag );
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;
    std::vector<const PdmUiTreeViewItemAttribute::Tag*> tags( QModelIndex index ) const;

protected:
    bool  editorEvent( QEvent*                     event,
                       QAbstractItemModel*         model,
                       const QStyleOptionViewItem& option,
                       const QModelIndex&          itemIndex ) override;
    bool  tagClicked( const QPoint&                           clickPos,
                      const QRect&                            itemRect,
                      const QModelIndex&                      itemIndex,
                      const PdmUiTreeViewItemAttribute::Tag** tag ) const;
    QRect tagRect( const QRect& itemRect, QModelIndex itemIndex, size_t tagIndex ) const;

private:
    PdmUiTreeViewEditor* m_treeView;

    std::map<QModelIndex, std::vector<std::unique_ptr<PdmUiTreeViewItemAttribute::Tag>>> m_tags;
};
} // namespace caf
