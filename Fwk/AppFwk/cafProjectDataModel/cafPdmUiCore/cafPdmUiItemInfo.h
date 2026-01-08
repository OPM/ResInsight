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
#include <optional>

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
        , m_labelPosition( LabelPosition::LEFT )
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
    QString       m_uiName;
    IconProvider  m_iconProvider;
    QColor        m_contentTextColor;
    QString       m_toolTip;
    QString       m_whatsThis;
    QString       m_extraDebugText;
    QString       m_editorTypeName;
    QString       m_3dEditorTypeName;
    LabelPosition m_labelPosition;

    std::optional<bool> m_isHidden;
    std::optional<bool> m_isTreeHidden;
    std::optional<bool> m_isTreeChildrenHidden;
    std::optional<bool> m_isReadOnly;
    std::optional<bool> m_isCustomContextMenuEnabled;
    std::optional<bool> m_notifyAllFieldsInMultiFieldChangedEvents;
};

} // End of namespace caf
