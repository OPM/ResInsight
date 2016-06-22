/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafPdmPointer.h"

class QwtPlotCurve;
class QwtPlotGrid; 

class RimSummaryPlot;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public QwtPlot
{
public:
    RiuSummaryQwtPlot(RimSummaryPlot* plotDefinition, QWidget* parent = NULL);
    virtual ~RiuSummaryQwtPlot();

    RimSummaryPlot*                 ownerPlotDefinition();

protected:
    virtual bool                    eventFilter(QObject* watched, QEvent* event);

private:
    void                            setDefaults();
    void                            selectClosestCurve(const QPoint& pos);

private:
    QwtPlotGrid*                    m_grid;
    caf::PdmPointer<RimSummaryPlot> m_plotDefinition;
};



