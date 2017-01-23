/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuNightchartsWidget.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuNightchartsWidget::RiuNightchartsWidget(QWidget* parent) :
    QWidget(parent)
{
    clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::setType(Nightcharts::type t)
{
    m_chart.setType(t);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::clear()
{
    m_chart = Nightcharts();
    m_chart.setType(Nightcharts::Pie);
    m_chart.setLegendType(Nightcharts::Vertical);

    m_marginLeft = 16;
    m_marginTop = 16;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if(!m_chart.pieceCount()) return ;

    QPainter painter;
    QFont font;
    painter.begin(this);
    int w = (this->width() - m_marginLeft - 150);
    int h = (this->height() - m_marginTop - 100);
    int size = (w<h)?w:h;
    m_chart.setCords(m_marginLeft, m_marginTop,size, size);

    m_chart.draw(&painter);
    m_chart.drawLegend(&painter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::addItem(QString name, QColor color, float value)
{
    m_chart.addPiece(name,color,value);
}
