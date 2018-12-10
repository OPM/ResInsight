/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include <QWidget>

class QPaintEvent;
class QString;
class QStringList;


class RiuSimpleHistogramWidget : public QWidget
{
public:
    RiuSimpleHistogramWidget( QWidget * parent = nullptr, Qt::WindowFlags f = nullptr);

    void                setHistogramData(double min, double max, const std::vector<size_t>& histogram);
    void                setPercentiles(double pmin, double pmax);
    void                setMean(double mean) {m_mean = mean;}

protected:
     void       paintEvent(QPaintEvent* event) override;

private:
    void                draw(QPainter *painter,int x, int y, int width, int height );

    int                 xPosFromColIdx(size_t colIdx);
    int                 yPosFromCount(size_t colHeight);

    int                 xPosFromDomainValue(double value);

    std::vector<size_t> m_histogramData;
    double              m_max;
    double              m_min;
    double              m_minPercentile;
    double              m_maxPercentile;
    double              m_mean;
    size_t              m_maxHistogramCount;

    double              m_width;
    double              m_height;
    double              m_x;
    double              m_y;
};
