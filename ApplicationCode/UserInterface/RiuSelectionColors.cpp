/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuSelectionColors.h"

#include <QColor>

static const int RI_SELECTION_COLOR_COUNT = 7;
static const int RI_SELECTION_COLOR[] =
{
    Qt::magenta,
    Qt::cyan,
    Qt::blue,
    Qt::red,
    Qt::green,
    Qt::yellow,
    Qt::gray
};

static int riuSelectionColorIndex = 0;

//--------------------------------------------------------------------------------------------------
/// Pick default curve color from an index based palette
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiuSelectionColors::curveColorFromTable()
{
    QColor color = QColor(Qt::GlobalColor(RI_SELECTION_COLOR[riuSelectionColorIndex % RI_SELECTION_COLOR_COUNT]));
    ++riuSelectionColorIndex;
    cvf::Color3f cvfColor(color.redF(), color.greenF(), color.blueF());
    return cvfColor;
}

//--------------------------------------------------------------------------------------------------
/// Color rarely present in result value colors
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiuSelectionColors::singleCurveColor()
{
    riuSelectionColorIndex = 0;

    return curveColorFromTable();
}
