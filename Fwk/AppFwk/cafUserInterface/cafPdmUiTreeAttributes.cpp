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

#include "cafPdmUiTreeAttributes.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmUiTreeViewItemAttribute::Tag> caf::PdmUiTreeViewItemAttribute::createTag()
{
    return std::make_unique<Tag>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmUiTreeViewItemAttribute::Tag>
    caf::PdmUiTreeViewItemAttribute::createTag( const QColor& color, const QColor& backgroundColor, const QString& text )
{
    auto tag = createTag();

    auto   weight1      = 100;
    double transparency = 0.3;
    int    weight2      = std::max( 1, static_cast<int>( weight1 * 10 * transparency ) );

    int weightsum = weight1 + weight2;

    auto blendedColor = QColor( ( color.red() * weight1 + backgroundColor.red() * weight2 ) / weightsum,
                                ( color.green() * weight1 + backgroundColor.green() * weight2 ) / weightsum,
                                ( color.blue() * weight1 + backgroundColor.blue() * weight2 ) / weightsum );

    tag->bgColor = blendedColor;
    tag->fgColor = color;
    tag->text    = text;

    return tag;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeViewItemAttribute::createTagIfTreeViewItemAttribute( caf::PdmUiEditorAttribute* attribute,
                                                                        const QString&             iconResourceString )
{
    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        auto tag  = caf::PdmUiTreeViewItemAttribute::createTag();
        tag->icon = caf::IconProvider( iconResourceString );

        treeItemAttribute->tags.push_back( std::move( tag ) );
    }
}
