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

#include "RicWellLogPlotCurveFeatureImpl.h"

#include <QColor>

static const int RI_LOGPLOT_CURVECOLORSCOUNT = 15;
static const int RI_LOGPLOT_CURVECOLORS[] =
{
    Qt::blue,
    Qt::red,
    Qt::green,
    Qt::yellow,
    Qt::magenta,
    Qt::cyan,
    Qt::gray,
    Qt::darkBlue,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkYellow,
    Qt::darkMagenta,
    Qt::darkCyan,
    Qt::darkGray,
    Qt::black
};

//--------------------------------------------------------------------------------------------------
/// Pick default curve color from an index based palette
//--------------------------------------------------------------------------------------------------
cvf::Color3f RicWellLogPlotCurveFeatureImpl::curveColorFromIndex(size_t curveIndex)
{
    QColor color = QColor(Qt::GlobalColor(RI_LOGPLOT_CURVECOLORS[curveIndex % RI_LOGPLOT_CURVECOLORSCOUNT]));

    cvf::Color3f cvfColor(color.redF(), color.greenF(), color.blueF());
    return cvfColor;
}
