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

#include "RiuQwtPlotItemGroup.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotItemGroup::RiuQwtPlotItemGroup()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotItemGroup::~RiuQwtPlotItemGroup()
{
    this->detach();
    for (QwtPlotItem* item : m_plotItems)
    {
        delete item;
    }
    for (QwtPlotItem* legendItem : m_legendItems)
    {
        delete legendItem;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotItemGroup::addPlotItem(QwtPlotItem* plotItem)
{
    m_plotItems.push_back(plotItem);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotItemGroup::addLegendItem(QwtPlotItem* legendItem)
{
    m_legendItems.push_back(legendItem);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuQwtPlotItemGroup::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    for (const QwtPlotItem* item : m_plotItems)
    {
        item->draw(painter, xMap, yMap, canvasRect);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRectF RiuQwtPlotItemGroup::boundingRect() const
{
    QRectF totalBoundingRect;
    for (const QwtPlotItem* item : m_plotItems)
    {
        totalBoundingRect = totalBoundingRect.united(item->boundingRect());
    }
    return totalBoundingRect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtGraphic RiuQwtPlotItemGroup::legendIcon(int index, const QSizeF& size) const
{
    QwtGraphic graphic;
    graphic.setDefaultSize(size);

    QPainter painter(&graphic);
    painter.setRenderHint(QPainter::Antialiasing,
        testRenderHint(QwtPlotItem::RenderAntialiased));

    for (QwtPlotItem* legendItem : m_legendItems)
    {
        QwtGraphic subGraphic = legendItem->legendIcon(index, legendItem->legendIconSize());
        QRectF boundingRect   = legendItem->boundingRect();
        QImage subImage       = subGraphic.toImage();
        QRectF subRect(0.0, 0.0, legendItem->legendIconSize().width(), legendItem->legendIconSize().height());  
        // Symbols may not have a bounding width/height. Force the width height to be the same as the icon
        boundingRect.setWidth(subRect.width());
        boundingRect.setHeight(subRect.height());
        // Paint onto the existing icon
        painter.drawImage(boundingRect, subImage, subRect);
    }
    return graphic;
}
