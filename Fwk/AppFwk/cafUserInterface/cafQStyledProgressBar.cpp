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
#include "cafQStyledProgressBar.h"

#include "cafStyleSheetTools.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStyledProgressBar::QStyledProgressBar( QString objectName, QWidget* parent /*= nullptr*/ )
    : QProgressBar( parent )
{
    setObjectName( objectName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QStyledProgressBar::setTextBackgroundAndProgressColor( QColor textColor,
                                                            QColor backgroundColor,
                                                            QColor backgroundFrameColor,
                                                            QColor progressColor )
{
    QString styleSheetTemplate = "QProgressBar#%1::chunk"
                                 "{"
                                 "background-color: %2;"
                                 "}";
    QString progressColString = colorStringWithAlpha( progressColor );

    QString fullStyleSheet =
        StyleSheetTools::createFrameStyleSheet( "QProgressBar", objectName(), textColor, backgroundColor, backgroundFrameColor );
    fullStyleSheet += styleSheetTemplate.arg( objectName() ).arg( progressColString );

    setStyleSheet( fullStyleSheet );
    setAttribute( Qt::WA_TranslucentBackground );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QStyledProgressBar::colorStringWithAlpha( QColor color )
{
    return QString( "rgba(%1, %2, %3, %4)" ).arg( color.red() ).arg( color.green() ).arg( color.blue() ).arg( color.alpha() );
}
