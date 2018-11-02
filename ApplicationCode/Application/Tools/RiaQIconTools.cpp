/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaQIconTools.h"

#include <QPainter>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPixmap RiaQIconTools::appendPixmapUpperLeft(const QPixmap& pixmap, const QPixmap& overlayPixmap)
{
    QPixmap scaledPixmap;
    {
        QSize size = pixmap.size() - pixmap.size() / 4;

        scaledPixmap = overlayPixmap.scaled(size);
    }

    QPixmap  combinedPixmap(pixmap);
    QPainter painter(&combinedPixmap);

    int x = 0;
    int y = -4;

    painter.drawPixmap(x, y, scaledPixmap);

    return combinedPixmap;
}
