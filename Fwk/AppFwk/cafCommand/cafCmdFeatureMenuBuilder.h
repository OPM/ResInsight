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

#include <vector>

#include <QIcon>
#include <QVariant>

class QAction;
class QMenu;

namespace caf
{
class CmdFeature;

//==================================================================================================
///
//==================================================================================================
class CmdFeatureMenuBuilder
{
public:
    CmdFeatureMenuBuilder();
    virtual ~CmdFeatureMenuBuilder();

    CmdFeatureMenuBuilder& operator<<( const QString& commandIdOrSeparator );
    CmdFeatureMenuBuilder& addCmdFeature( const QString commandId, const QString& customUiText = "" );
    CmdFeatureMenuBuilder&
        addCmdFeatureWithUserData( const QString commandId, const QString& customUiText, const QVariant& userData );

    CmdFeatureMenuBuilder& addSeparator();

    CmdFeatureMenuBuilder& subMenuStart( const QString& menuName, const QIcon& menuIcon = QIcon() );
    CmdFeatureMenuBuilder& subMenuEnd();

    void appendToMenu( QMenu* menu );

    bool   isCmdFeatureAdded( const QString& commandId );
    size_t itemCount() const;

private:
    struct MenuItem
    {
    public:
        enum ItemType
        {
            COMMAND,
            SEPARATOR,
            SUBMENU_START,
            SUBMENU_END
        };

        ItemType itemType;
        QString  itemName;
        QString  uiText;
        QVariant userData;
        QIcon    icon;
    };

    std::vector<MenuItem> m_items;
};

} // end namespace caf
