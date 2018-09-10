/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


#include "RiuCursors.h"

#include <QtCore/QString>
#include <QtGui/QBitmap>



//==================================================================================================
///
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuCursors::RiuCursors()
{
    m_cursors[FILTER_BOX]   = cursorFromFile(":/Cursors/curFilterBox.bmp",      10, 10);
    m_cursors[NORMAL]        = cursorFromFile(":/Cursors/curNormal.bmp",            10, 10);
    m_cursors[PAN]            = cursorFromFile(":/Cursors/curPan.bmp");
    m_cursors[WALK]            = cursorFromFile(":/Cursors/curWalk.bmp");
    m_cursors[ZOOM]            = cursorFromFile(":/Cursors/curZoom.bmp");
    m_cursors[ROTATE]        = cursorFromFile(":/Cursors/curRotate.bmp");

    m_cursors[PICK]            = cursorFromFile(":/Cursors/curPick.bmp",            10, 10);
    m_cursors[PICK_ROTPOINT]= cursorFromFile(":/Cursors/curPickRotPoint.bmp",   10, 10);
    m_cursors[PICK_GOTO]    = cursorFromFile(":/Cursors/curPickGoto.bmp",        10, 10);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QCursor RiuCursors::get(CursorIndex cursorIdx)
{
    // Create our single instance in a local static variable
    static RiuCursors myStaticInstance;
    
    return myStaticInstance.m_cursors[cursorIdx];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QCursor    RiuCursors::cursorFromFile(const QString& fileName, int hotspotX, int hotspotY)
{
    QImage image(fileName);
    if (image.width() == 0 || image.height() == 0)
    {
        return QCursor();
    }

    //QRgb maskClr = image.pixel(0, 0);
    //QImage imgMask = image.createMaskFromColor(maskClr, Qt::MaskInColor);
    QImage imgMask = image.createHeuristicMask(true);

    QBitmap bmMask = QBitmap::fromImage(imgMask, Qt::ThresholdDither | Qt::AvoidDither);

    QBitmap bitmap = QBitmap::fromImage(image, Qt::ThresholdDither | Qt::AvoidDither);

    return QCursor(bitmap, bmMask, hotspotX, hotspotY);
}




