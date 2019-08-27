//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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

#include <QIcon>
#include <QPixmap>
#include <QString>

#include <memory>

namespace caf
{
//==================================================================================================
/// Utility class to provide QIcons when required. Qt crashes if a non-empty QIcon is created
/// ... without a GUI Application running. So create the icon on demand instead.
//==================================================================================================
class QIconProvider
{
public:
    QIconProvider();
    QIconProvider(const QString& iconResourceString);
    QIconProvider(const QPixmap& pixmap);
    QIconProvider(const QIconProvider& rhs);
    QIconProvider& operator=(const QIconProvider& rhs);

    QIcon        icon() const;
    virtual bool isNull() const;
    void         setActive(bool active);
    void         setIconResourceString(const QString& iconResourceString);
    void         setPixmap(const QPixmap& pixmap);

protected:
    bool          hasValidPixmap() const;
    virtual QIcon generateIcon() const;
    static bool   isGuiApplication();

protected:
    QString                  m_iconResourceString;
    std::unique_ptr<QPixmap> m_iconPixmap;
    mutable QIcon            m_icon;
    bool                     m_active;
};
}
