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
#include <QLinearGradient>
#include <QPainter>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const QSize& preferredSize )
    : m_active( true )
    , m_preferredSize( preferredSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const QString& iconResourceString, const QSize& preferredSize )
    : m_active( true )
    , m_iconResourceString( iconResourceString )
    , m_preferredSize( preferredSize )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider::IconProvider( const QPixmap& pixmap )
    : m_active( true )
    , m_pixmap( new QPixmap( pixmap ) )
    , m_preferredSize( pixmap.size() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider::IconProvider( const IconProvider& rhs )
    : m_active( rhs.m_active )
    , m_iconResourceString( rhs.m_iconResourceString )
    , m_overlayResourceString( rhs.m_overlayResourceString )
    , m_backgroundColorStrings( rhs.m_backgroundColorStrings )
    , m_preferredSize( rhs.m_preferredSize )
{
    copyPixmap( rhs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
IconProvider& IconProvider::operator=( const IconProvider& rhs )
{
    m_active                 = rhs.m_active;
    m_iconResourceString     = rhs.m_iconResourceString;
    m_overlayResourceString  = rhs.m_overlayResourceString;
    m_backgroundColorStrings = rhs.m_backgroundColorStrings;
    m_preferredSize          = rhs.m_preferredSize;
    copyPixmap( rhs );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setActive( bool active )
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::IconProvider::valid() const
{
    if ( isGuiApplication() )
    {
        if ( m_pixmap && !m_pixmap->isNull() ) return true;

        if ( backgroundColorsAreValid() )
        {
            return true;
        }

        if ( !m_iconResourceString.isEmpty() )
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
    m_preferredSize = size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> IconProvider::icon() const
{
    return this->icon( m_preferredSize );
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

    if ( m_pixmap ) return std::unique_ptr<QIcon>( new QIcon( *m_pixmap ) );

    QPixmap pixmap( size );

    bool validIcon = false;
    if ( !m_backgroundColorStrings.empty() )
    {
        if ( m_backgroundColorStrings.size() == 1u && QColor::isValidColor( m_backgroundColorStrings.front() ) )
        {
            pixmap.fill( QColor( m_backgroundColorStrings.front() ) );
            validIcon = true;
        }
        else
        {
            validIcon = true;

            // Draw color gradient based on background colors

            QLinearGradient gradient( QPointF( 0.0f, 0.0f ), QPoint( size.width(), 0.0f ) );
            for ( size_t i = 0; i < m_backgroundColorStrings.size(); ++i )
            {
                if ( !QColor::isValidColor( m_backgroundColorStrings[i] ) )
                {
                    validIcon = false;
                    break;
                }
                QColor color( m_backgroundColorStrings[i] );
                float  frac = i / ( (float)m_backgroundColorStrings.size() - 1.0 );
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

    if ( !m_iconResourceString.isEmpty() )
    {
        QIcon resourceStringIcon( m_iconResourceString );
        if ( !resourceStringIcon.isNull() )
        {
            QPixmap  iconPixmap = resourceStringIcon.pixmap( size, m_active ? QIcon::Normal : QIcon::Disabled );
            QPainter painter( &pixmap );
            painter.drawPixmap( 0, 0, iconPixmap );
            validIcon = true;
        }
    }

    if ( !m_overlayResourceString.isEmpty() )
    {
        QIcon overlayIcon( m_overlayResourceString );
        if ( !overlayIcon.isNull() )
        {
            QPixmap  overlayPixmap = overlayIcon.pixmap( size, m_active ? QIcon::Normal : QIcon::Disabled );
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
    m_iconResourceString = iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setOverlayResourceString( const QString& overlayResourceString )
{
    m_overlayResourceString = overlayResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setBackgroundColorString( const QString& colorName )
{
    m_backgroundColorStrings = { colorName };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void IconProvider::setBackgroundColorGradient( const std::vector<QString>& colorNames )
{
    m_backgroundColorStrings = colorNames;
}

//--------------------------------------------------------------------------------------------------
/// Use a pixmap instead of the resource strings.
//--------------------------------------------------------------------------------------------------
void caf::IconProvider::setPixmap( const QPixmap& pixmap )
{
    m_pixmap.reset( new QPixmap( pixmap ) );
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
void IconProvider::copyPixmap( const IconProvider& rhs )
{
    if ( rhs.m_pixmap )
    {
        m_pixmap = std::unique_ptr<QPixmap>( new QPixmap( *rhs.m_pixmap ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool IconProvider::backgroundColorsAreValid() const
{
    if ( !m_backgroundColorStrings.empty() )
    {
        bool validBackgroundColors = true;
        for ( QString colorName : m_backgroundColorStrings )
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
