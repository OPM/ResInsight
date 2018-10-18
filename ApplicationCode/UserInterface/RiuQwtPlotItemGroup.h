/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "qwt_plot_item.h"

class QwtGraphic;
class QRectF;
class QPainter;

//==================================================================================================
/// Class that can combine multiple Qwt plot items into one Qwt graphic with a combined legend.
/// 
//==================================================================================================
class RiuQwtPlotItemGroup : public QwtPlotItem
{
public:
    RiuQwtPlotItemGroup();
    ~RiuQwtPlotItemGroup() override;

    void addPlotItem(QwtPlotItem* plotItem);
    void addLegendItem(QwtPlotItem* legendItem);

    int rtti() const override { return 5000; }
    void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const override;
    QRectF boundingRect() const override;
    QwtGraphic legendIcon(int index, const QSizeF &size) const override;

private:
    std::vector<QwtPlotItem*> m_plotItems;
    std::vector<QwtPlotItem*> m_legendItems;
};

