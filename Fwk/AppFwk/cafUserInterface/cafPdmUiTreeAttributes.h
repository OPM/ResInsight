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

#include "cafIconProvider.h"

#include <QColor>
#include <QRect>
#include <QString>

#include <functional>

namespace caf
{
// A tag shown next to an item in the tree view. Provide tags by setting a
// std::function<std::vector<PdmUiTreeViewItemTag>()> on an object using PdmUiTreeViewEditor::Keys::TREE_ITEM_TAGS
struct PdmUiTreeViewItemTag
{
    enum class Position
    {
        IN_FRONT,
        AT_END
    };

    QString      text;
    IconProvider icon;
    Position     position     = Position::AT_END;
    QColor       bgColor      = Qt::red;
    QColor       fgColor      = Qt::white;
    bool         selectedOnly = false;
    QRect        rect;
    QString      toolTip;

    // Called with the index of the clicked object in its parent container
    std::function<void( size_t )> clicked;

    static PdmUiTreeViewItemTag create();
    static PdmUiTreeViewItemTag create( const QColor& color, const QColor& backgroundColor, const QString& text );
    static PdmUiTreeViewItemTag createWithIcon( const QString& iconResourceString );
};

} // namespace caf