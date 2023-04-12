

// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################

#pragma once

#include "cafPdmUiFieldEditorHandle.h"
#include "cafSignal.h"

namespace caf
{
class PdmUiTreeViewItemAttribute : public PdmUiEditorAttribute
{
public:
    struct Tag : public SignalEmitter
    {
        enum Position
        {
            IN_FRONT,
            AT_END
        };
        Tag()
            : text()
            , position( AT_END )
            , bgColor( Qt::red )
            , fgColor( Qt::white )
            , selectedOnly( false )
            , clicked( this )
        {
        }
        QString      text;
        IconProvider icon;
        Position     position;
        QColor       bgColor;
        QColor       fgColor;
        bool         selectedOnly;

        caf::Signal<size_t> clicked;

        static std::unique_ptr<Tag> create() { return std::unique_ptr<Tag>( new Tag ); }

    private:
        Tag& operator=( const Tag& rhs ) { return *this; }
    };

    std::vector<std::unique_ptr<Tag>> tags;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiTreeViewEditorAttribute()
        : currentObject( nullptr )
        , objectForUpdateOfUiTree( nullptr )
    {
    }

public:
    QStringList columnHeaders;

    /// This object is set as current item in the tree view in configureAndUpdateUi()
    caf::PdmObjectHandle* currentObject;

    caf::PdmObjectHandle* objectForUpdateOfUiTree;
};

} // namespace caf
