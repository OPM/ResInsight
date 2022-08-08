/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Equinor ASA
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

#pragma once

#include "RiuPlotCurveSymbol.h"

#include <QImage>

class QColor;
class QPen;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuPlotCurveSymbolImageCreator
{
public:
    static QImage createSymbolImage( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle,
                                     const QSize&                        size,
                                     const QPen&                         pen,
                                     const QColor&                       color );

    static QImage
        createSymbolImage( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle, const QSize& size, const QColor& color );

protected:
    static QImage createTriangleImage( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle,
                                       const QSize&                        size,
                                       const QPen&                         pen,
                                       const QColor&                       color );
    static QImage createRectImage( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createStar1Image( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createStar2Image( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createHexagonImage( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createEllipseImage( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createCrossImage( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createXCrossImage( const QSize& size, const QPen& pen, const QColor& color );
    static QImage createDiamondImage( const QSize& size, const QPen& pen, const QColor& color );
};
