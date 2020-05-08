//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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
#include <QSize>
#include <QString>

#include <memory>

class QIcon;
class QPixmap;

namespace caf
{
//==================================================================================================
/// Utility class to provide Icons when required. Qt crashes if a non-empty QIcon is created
/// ... without a GUI Application running. So create the icon on demand instead.
//==================================================================================================
class IconProvider
{
public:
    IconProvider();
    IconProvider(const QString& iconResourceString);
    IconProvider(const QPixmap& pixmap);
    IconProvider(const IconProvider& rhs);
    IconProvider& operator=(const IconProvider& rhs);
    
    void setActive(bool active);
    bool valid() const;

    std::unique_ptr<QIcon> icon(const QSize& size = QSize(16, 16)) const;

    void setIconResourceString(const QString& iconResourceString);
    void setOverlayResourceString(const QString& overlayResourceString);
    void setBackgroundColorString(const QString& colorName);

    void setPixmap(const QPixmap& pixmap);

private:
    static bool isGuiApplication();
    void copyPixmap(const IconProvider& rhs);
private:
    bool m_active;

    QString                  m_iconResourceString;
    QString                  m_overlayResourceString;
    QString                  m_backgroundColorString;

    std::unique_ptr<QPixmap> m_pixmap;
};
}
