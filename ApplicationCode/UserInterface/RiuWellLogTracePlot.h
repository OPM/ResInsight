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

#pragma once

#include "qwt_plot.h"

class RimWellLogPlotTrace;
class QwtPlotGrid;
class QwtLegend;

class QEvent;

//==================================================================================================
//
// RiuWellLogTracePlot
//
//==================================================================================================
class RiuWellLogTracePlot : public QwtPlot
{
    Q_OBJECT

public:
    RiuWellLogTracePlot(RimWellLogPlotTrace* plotTraceDefinition, QWidget* parent = NULL);
    virtual ~RiuWellLogTracePlot();

    void setDepthRange(double minDepth, double maxDepth);
    void setDepthTitle(const QString& title);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void focusInEvent(QFocusEvent* event);

private:
    void setDefaults();

private:
    RimWellLogPlotTrace*    m_plotTraceDefinition;
    QwtPlotGrid*            m_grid;
    QwtLegend*              m_legend;
};

