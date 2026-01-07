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

#include <memory>

class QIcon;

namespace caf
{
//==================================================================================================
/// Class to keep (principally static) gui presentation information
/// of a data structure item (field or object) used by PdmUiItem
//==================================================================================================

class PdmUiItemInfo
{
public:
    enum class LabelPosition
    {
        LEFT,
        TOP,
        HIDDEN
    };

    PdmUiItemInfo()
        : m_editorTypeName( "" )
        , m_isHidden( -1 )
        , m_isTreeHidden( -1 )
        , m_isTreeChildrenHidden( -1 )
        , m_isReadOnly( -1 )
        , m_labelPosition( LabelPosition::LEFT )
        , m_isCustomContextMenuEnabled( -1 )
        , m_notifyAllFieldsInMultiFieldChangedEvents( -1 )
    {
    }

    PdmUiItemInfo( const QString& uiName,
                   QString        iconResourceLocation = "",
                   QString        toolTip              = "",
                   QString        whatsThis            = "",
                   QString        extraDebugText       = "" );

    PdmUiItemInfo( const QString& uiName,
                   IconProvider   iconProvider   = IconProvider(),
                   QString        toolTip        = "",
                   QString        whatsThis      = "",
                   QString        extraDebugText = "" );

    std::unique_ptr<QIcon> icon() const;
    const IconProvider&    iconProvider() const;

private:
    friend class PdmUiItem;
    QString      m_uiName;
    IconProvider m_iconProvider;
    QColor  m_contentTextColor; ///< Color of a fields value text. Invalid by default. An Invalid color is not used.
    QString m_toolTip;
    QString m_whatsThis;
    QString m_extraDebugText;
    QString m_editorTypeName; ///< Use this exact type of editor to edit this UiItem
    QString m_3dEditorTypeName; ///< If set, use this editor type to edit this UiItem in 3D
    int     m_isHidden; ///< UiItem should be hidden. -1 means not set
    int     m_isTreeHidden; ///< UiItem should be hidden in tree. -1 means not set
    int     m_isTreeChildrenHidden; ///< Children of UiItem should be hidden. -1 means not set
    int     m_isReadOnly; ///< UiItem should be insensitive, or read only. -1 means not set.
    LabelPosition m_labelPosition;
    int           m_isCustomContextMenuEnabled;
    int           m_notifyAllFieldsInMultiFieldChangedEvents;
};

} // End of namespace caf
