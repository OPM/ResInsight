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

#pragma once

#include <QCursor>



//==================================================================================================
//
// Singleton for getting the cursors
//
//==================================================================================================
class RiuCursors
{
public:
    enum CursorIndex
    {
        FILTER_BOX,
        NORMAL,
        PAN,
        WALK,
        ZOOM,
        ROTATE,
        PICK,
        PICK_GOTO,
        PICK_ROTPOINT,
        NUM_CURSORS        
    };

public:
    static QCursor get(CursorIndex cursorIdx);

private:
    RiuCursors();
    static QCursor cursorFromFile(const QString& fileName, int hotspotX = -1, int hotspotY = -1);

private:
    QCursor    m_cursors[NUM_CURSORS];
};
