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
#include "cafFontTools.h"

#include "cafAppEnum.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiItem.h"

#include <QApplication>
#include <QDesktopWidget>

#include <cmath>

namespace caf
{
const int FontTools::MIN_FONT_SIZE = 6;

template <>
void FontTools::FontSizeEnum::setUp()
{
    addItem( FontTools::FontSize::FONT_SIZE_8, "8", "8" );
    addItem( FontTools::FontSize::FONT_SIZE_10, "10", "10" );
    addItem( FontTools::FontSize::FONT_SIZE_12, "12", "12" );
    addItem( FontTools::FontSize::FONT_SIZE_14, "14", "14" );
    addItem( FontTools::FontSize::FONT_SIZE_16, "16", "16" );
    addItem( FontTools::FontSize::FONT_SIZE_24, "24", "24" );
    addItem( FontTools::FontSize::FONT_SIZE_32, "32", "32" );

    setDefault( FontTools::FontSize::FONT_SIZE_8 );
}

template <>
void FontTools::RelativeSizeEnum::setUp()
{
    addItem( FontTools::RelativeSize::XXSmall, "XX_Small", "XX Small" );
    addItem( FontTools::RelativeSize::XSmall, "X_Small", "X Small" );
    addItem( FontTools::RelativeSize::Small, "Small", "Small" );
    addItem( FontTools::RelativeSize::Medium, "Medium", "Medium" );
    addItem( FontTools::RelativeSize::Large, "Large", "Large" );
    addItem( FontTools::RelativeSize::XLarge, "X_Large", "X Large" );
    addItem( FontTools::RelativeSize::XXLarge, "XX_Large", "XX Large" );

    setDefault( FontTools::RelativeSize::Medium );
}
} // namespace caf

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::absolutePointSize( FontSize normalPointSize, RelativeSize relativeSize )
{
    return static_cast<int>( normalPointSize ) + static_cast<int>( relativeSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pointSizeToPixelSize( int pointSize )
{
    auto app = dynamic_cast<const QApplication*>( QCoreApplication::instance() );
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pointSize / 72.0;
        return static_cast<int>( std::ceil( inches * dpi ) );
    }
    return pointSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pointSizeToPixelSize( FontSize pointSize )
{
    return pointSizeToPixelSize( (int)pointSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pixelSizeToPointSize( int pixelSize )
{
    auto app = dynamic_cast<const QApplication*>( QCoreApplication::instance() );
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pixelSize / dpi;
        return static_cast<int>( std::ceil( inches * 72.0 ) );
    }
    return pixelSize;
}

QList<PdmOptionItemInfo> FontTools::relativeSizeValueOptions( FontSize normalPointSize )
{
    QList<caf::PdmOptionItemInfo> options;
    for ( size_t i = 0; i < RelativeSizeEnum::size(); ++i )
    {
        QString      uiText            = RelativeSizeEnum::uiTextFromIndex( i );
        RelativeSize relSize           = RelativeSizeEnum::fromIndex( i );
        int          absolutePointSize = FontTools::absolutePointSize( normalPointSize, relSize );
        if ( absolutePointSize >= MIN_FONT_SIZE )
        {
            uiText += QString( " (%1 pt)" ).arg( absolutePointSize );
            options.push_back( PdmOptionItemInfo( uiText, relSize ) );
        }
    }
    return options;
}
