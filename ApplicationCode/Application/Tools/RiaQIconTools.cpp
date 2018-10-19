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
    QPixmap scaledToFitIncoming;
    {
        QSize size = pixmap.size() / 2;

        scaledToFitIncoming = overlayPixmap.scaled(size);
    }

    QPixmap  pixmapWithSign(pixmap);
    QPainter painter(&pixmapWithSign);
    painter.drawPixmap(0, 0, scaledToFitIncoming);

    return pixmapWithSign;
}
