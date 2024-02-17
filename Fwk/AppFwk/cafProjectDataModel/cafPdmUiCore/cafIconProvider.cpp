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
#include <QIcon>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmapCache>

using namespace caf;

class caf::IconProvider::Impl
{
public:
    bool                 m_active;
    QString              m_iconResourceString;
    QString              m_overlayResourceString;
    std::vector<QString> m_backgroundColorStrings;
    QSize                m_preferredSize;
    QPixmap              m_pixmap;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const QSize& preferredSize )
    : m_impl( new Impl )
{
    m_impl->m_active        = true;
    m_impl->m_preferredSize = preferredSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const QString& iconResourceString, const QSize& preferredSize )
    : m_impl( new Impl )
{
    m_impl->m_active             = true;
    m_impl->m_iconResourceString = iconResourceString;
    m_impl->m_preferredSize      = preferredSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider::IconProvider( const QPixmap& pixmap )
    : m_impl( new Impl )
{
    m_impl->m_active        = true;
    m_impl->m_pixmap        = pixmap;
    m_impl->m_preferredSize = pixmap.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::~IconProvider()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const IconProvider& rhs )
{
    *m_impl = *rhs.m_impl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider& IconProvider::operator=( const IconProvider& rhs )
{
    *m_impl = *rhs.m_impl;

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setActive( bool active )
{
    m_impl->m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::IconProvider::valid() const
{
    if ( isGuiApplication() )
    {
        if ( !m_impl->m_pixmap.isNull() ) return true;

        if ( backgroundColorsAreValid() )
        {
            return true;
        }

        if ( !m_impl->m_iconResourceString.isEmpty() )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setPreferredSize( const QSize& size )
{
    m_impl->m_preferredSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> IconProvider::icon() const
{
    return this->icon( m_impl->m_preferredSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> IconProvider::icon( const QSize& size ) const
{
    if ( !isGuiApplication() )
    {
        return nullptr;
    }

    if ( !m_impl->m_pixmap.isNull() ) return std::unique_ptr<QIcon>( new QIcon( m_impl->m_pixmap ) );

    QPixmap pixmap( size );

    bool validIcon = false;
    if ( !m_impl->m_backgroundColorStrings.empty() )
    {
        if ( m_impl->m_backgroundColorStrings.size() == 1u &&
             QColor::isValidColor( m_impl->m_backgroundColorStrings.front() ) )
        {
            pixmap.fill( QColor( m_impl->m_backgroundColorStrings.front() ) );
            validIcon = true;
        }
        else
        {
            validIcon = true;

            // Draw color gradient based on background colors

            QLinearGradient gradient( QPointF( 0.0f, 0.0f ), QPoint( size.width(), 0.0f ) );
            for ( size_t i = 0; i < m_impl->m_backgroundColorStrings.size(); ++i )
            {
                if ( !QColor::isValidColor( m_impl->m_backgroundColorStrings[i] ) )
                {
                    validIcon = false;
                    break;
                }
                QColor color( m_impl->m_backgroundColorStrings[i] );
                float  frac = i / ( (float)m_impl->m_backgroundColorStrings.size() - 1.0 );
                gradient.setColorAt( frac, color );
            }
            QBrush   gradientBrush( gradient );
            QPainter painter( &pixmap );
            painter.fillRect( 0, 0, size.width(), size.height(), gradientBrush );
        }

        // Draw border

        QPainter painter2( &pixmap );
        painter2.setRenderHint( QPainter::Antialiasing );
        painter2.setPen( QPen( Qt::black, 1 ) );
        painter2.drawRect( QRectF( 0, 0, size.width(), size.height() ) );
    }
    else
        pixmap.fill( Qt::transparent );

    if ( !m_impl->m_iconResourceString.isEmpty() )
    {
        QPixmap pm;
        if ( !QPixmapCache::find( m_impl->m_iconResourceString, &pm ) )
        {
            pm.load( m_impl->m_iconResourceString );
            QPixmapCache::insert( m_impl->m_iconResourceString, pm );
        }

        if ( !pm.isNull() )
        {
            QIcon    resourceStringIcon( pm );
            QPixmap  iconPixmap = resourceStringIcon.pixmap( size, m_impl->m_active ? QIcon::Normal : QIcon::Disabled );
            QPainter painter( &pixmap );
            painter.drawPixmap( 0, 0, iconPixmap );
            validIcon = true;
        }
    }

    if ( !m_impl->m_overlayResourceString.isEmpty() )
    {
        QIcon overlayIcon( m_impl->m_overlayResourceString );
        if ( !overlayIcon.isNull() )
        {
            QPixmap  overlayPixmap = overlayIcon.pixmap( size, m_impl->m_active ? QIcon::Normal : QIcon::Disabled );
            QPainter painter( &pixmap );
            painter.drawPixmap( 0, 0, overlayPixmap );
            validIcon = true;
        }
    }

    return validIcon ? std::unique_ptr<QIcon>( new QIcon( pixmap ) ) : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setIconResourceString( const QString& iconResourceString )
{
    m_impl->m_iconResourceString = iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setOverlayResourceString( const QString& overlayResourceString )
{
    m_impl->m_overlayResourceString = overlayResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setBackgroundColorString( const QString& colorName )
{
    m_impl->m_backgroundColorStrings = { colorName };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setBackgroundColorGradient( const std::vector<QString>& colorNames )
{
    m_impl->m_backgroundColorStrings = colorNames;
}

//--------------------------------------------------------------------------------------------------
/// Use a pixmap instead of the resource strings.
//--------------------------------------------------------------------------------------------------
void caf::IconProvider::setPixmap( const QPixmap& pixmap )
{
    m_impl->m_pixmap = pixmap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::IconProvider::isGuiApplication()
{
    return dynamic_cast<QApplication*>( QCoreApplication::instance() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool IconProvider::backgroundColorsAreValid() const
{
    if ( !m_impl->m_backgroundColorStrings.empty() )
    {
        bool validBackgroundColors = true;
        for ( QString colorName : m_impl->m_backgroundColorStrings )
        {
            if ( !QColor::isValidColor( colorName ) )
            {
                validBackgroundColors = false;
                break;
            }
        }
        return validBackgroundColors;
    }
    return false;
}
