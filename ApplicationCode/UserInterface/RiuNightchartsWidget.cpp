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
    m_showLegend = true;
    m_showPie = true;
    updateSizePolicy();
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
void RiuNightchartsWidget::showLegend(bool doShow)
{
    m_showLegend = doShow;
    updateSizePolicy();

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::showPie(bool doShow)
{
    m_showPie = doShow;
    updateSizePolicy();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::updateSizePolicy()
{
    if (m_showPie && m_showLegend)
    {
        this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
    else if (m_showPie && !m_showLegend )
    {
        this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    }
    else if (!m_showPie && m_showLegend )
    {
        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
    else
    {
        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::clear()
{
    m_chart = Nightcharts();
    m_chart.setType(Nightcharts::Pie);
    m_chart.setLegendType(Nightcharts::Vertical);
    m_chart.setShadows(false);

    m_marginLeft = 10;
    m_marginTop = 10;
    m_maxNameWidth = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuNightchartsWidget::sizeHint() const
{
    int widthHint = 0;
    int heightHint = 0;

    if ( m_showLegend )
    {
        int lineHeight = this->fontMetrics().height();
        int lineCount = m_chart.pieceCount();

        int exactLegendHeight = (lineCount + lineCount-1) * lineHeight;

        widthHint  =  m_maxNameWidth + 5 + lineHeight;
        heightHint =  exactLegendHeight;
    }

    if (m_showPie)
    {
        int maxPieSize = 180;

        widthHint = widthHint + maxPieSize;
        heightHint = heightHint > maxPieSize ?  heightHint : maxPieSize;
    }

    if ( m_showPie || m_showLegend )
    {
        widthHint  += 2*m_marginLeft;
        heightHint += 2*m_marginTop;
        return QSize(widthHint, heightHint);
    }
    else
    {
        return QSize();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if(!m_chart.pieceCount()) return ;

    QPainter painter;
    painter.begin(this);
    
    int legendWidth = 170; 
    int legendMargin = 20;

    if (!m_showLegend) 
    {
        legendWidth = 0;
        legendMargin = 0;
    }

    int w = (this->width() - 2* m_marginLeft - legendWidth - legendMargin);
    int h = (this->height() - 2* m_marginTop );

    int size = ( w < h ) ? w : h;

    if ( m_showPie )
    {
        m_chart.setCords(m_marginLeft, m_marginTop, size, size);
        m_chart.setLegendCords(m_marginLeft + size + legendMargin, m_marginTop);
    }
    else
    {
        m_chart.setLegendCords(m_marginLeft, m_marginTop);
    }

    if (m_showPie) m_chart.draw(&painter);

    if ( m_showLegend) m_chart.drawLegend(&painter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuNightchartsWidget::addItem(const QString& name, const QColor& color, float value)
{
    m_chart.addPiece(name, color, value);
    int textWidth = this->fontMetrics().width(name + " (00 %)");

    m_maxNameWidth = textWidth > m_maxNameWidth ? textWidth: m_maxNameWidth;
}
