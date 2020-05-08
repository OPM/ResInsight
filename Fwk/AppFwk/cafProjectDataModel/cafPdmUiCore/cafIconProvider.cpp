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
#include "cafIconProvider.h"

#include <QApplication>
#include <QPainter>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider()
    : m_active(true)
{
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider(const QString& iconResourceString)
    : m_active(true)
    , m_iconResourceString(iconResourceString)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider::IconProvider(const QPixmap& pixmap)
    : m_active(true)
    , m_pixmap(new QPixmap(pixmap))
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider(const IconProvider& rhs)
    : m_active(rhs.m_active)
    , m_iconResourceString(rhs.m_iconResourceString)
    , m_overlayResourceString(rhs.m_overlayResourceString)
    , m_backgroundColorString(rhs.m_backgroundColorString)
{
    copyPixmap(rhs);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider& IconProvider::operator=(const IconProvider& rhs)
{
    m_active                = rhs.m_active;
    m_iconResourceString    = rhs.m_iconResourceString;
    m_overlayResourceString = rhs.m_overlayResourceString;
    m_backgroundColorString = rhs.m_backgroundColorString;
    copyPixmap(rhs);

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setActive(bool active)
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> IconProvider::icon(const QSize& size /*= QSize(16, 16)*/) const
{
    if (!isGuiApplication())
    {
        return nullptr;
    }

    if (m_pixmap) return std::unique_ptr<QIcon>(new QIcon(*m_pixmap));

    QPixmap pixmap(size);

    if (!m_backgroundColorString.isEmpty() && QColor::isValidColor(m_backgroundColorString))
    {
        pixmap.fill(QColor(m_backgroundColorString));
    }
    else pixmap.fill(Qt::transparent);

    if (!m_iconResourceString.isEmpty())
    {
        QPixmap iconPixmap = QIcon(m_iconResourceString).pixmap(size, m_active ? QIcon::Normal : QIcon::Disabled);
        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, iconPixmap);
    }

    if (!m_overlayResourceString.isEmpty())
    {
        QPixmap overlayPixmap = QIcon(m_overlayResourceString).pixmap(size, m_active ? QIcon::Normal : QIcon::Disabled);
        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, overlayPixmap);
    }

    return std::unique_ptr<QIcon>(new QIcon(pixmap));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setIconResourceString(const QString& iconResourceString)
{
    m_iconResourceString = iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setOverlayResourceString(const QString& overlayResourceString)
{
    m_overlayResourceString = overlayResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setBackgroundColorString(const QString& colorName)
{
    m_backgroundColorString = colorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::IconProvider::valid() const
{
    if (isGuiApplication())
    {
        if (m_pixmap && !m_pixmap->isNull()) return true;

        if (!m_backgroundColorString.isEmpty() && QColor::isValidColor(m_backgroundColorString))
        {
            return true;
        }

        if (!m_iconResourceString.isEmpty())
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Use a pixmap instead of the resource strings.
//--------------------------------------------------------------------------------------------------
void caf::IconProvider::setPixmap(const QPixmap& pixmap)
{
    m_pixmap.reset(new QPixmap(pixmap));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::IconProvider::isGuiApplication()
{
    return dynamic_cast<QApplication*>(QCoreApplication::instance()) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::copyPixmap(const IconProvider& rhs)
{
    if (rhs.m_pixmap)
    {
        m_pixmap = std::unique_ptr<QPixmap>(new QPixmap(*rhs.m_pixmap));
    }
}

