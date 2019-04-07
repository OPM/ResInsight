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
#include "cafStyleSheetTools.h"

#include <QColor>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString StyleSheetTools::createFrameStyleSheet(const QString& classType,
                                                    const QString& objectName,
                                                    const QColor&  textColor,
                                                    const QColor&  backgroundColor,
                                                    const QColor&  backgroundFrameColor)
{
    QString styleSheetTemplate = "%1#%2"
                                 "{"
                                 "color: %3;"
                                 "background-color: %4;"
                                 "border: 1px solid %5;"
                                 "}";
    QString textColString     = colorStringWithAlpha(textColor);
    QString bgColString       = colorStringWithAlpha(backgroundColor);
    QString bgFrameColString  = colorStringWithAlpha(backgroundFrameColor);

    QString fullStyleSheet =
        styleSheetTemplate.arg(classType).arg(objectName).arg(textColString).arg(bgColString).arg(bgFrameColString);
    return fullStyleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString StyleSheetTools::colorStringWithAlpha(const QColor& color) 
{
    return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
}
