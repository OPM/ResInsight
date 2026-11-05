/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaSnapshotTools.h"

#include "RiaRegressionTestRunner.h"

#include <QColor>
#include <QImage>
#include <QPixmap>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RiaSnapshotTools::renderSnapshotImage( int                                         width,
                                              int                                         height,
                                              const QColor&                               backgroundColor,
                                              const std::function<void( QPaintDevice* )>& renderFunction )
{
    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        // When painting into RGB32 or premultiplied ARGB32 images, Qt uses ClearType (subpixel) text antialiasing if
        // ClearType is enabled in Windows. Use non-premultiplied ARGB32 to get grayscale text antialiasing, making the
        // regression test images independent of the font smoothing settings on the computer.
        QImage image( width, height, QImage::Format_ARGB32 );
        image.fill( backgroundColor );
        renderFunction( &image );
        return image;
    }

    QPixmap pix( width, height );
    pix.fill( backgroundColor );
    renderFunction( &pix );
    return pix.toImage();
}
