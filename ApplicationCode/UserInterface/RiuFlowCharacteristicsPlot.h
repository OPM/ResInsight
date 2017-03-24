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

#pragma once

#include "qwt_plot.h"

#include "cafPdmPointer.h"

#include <QPointer>
#include <QFrame>

class RimFlowCharacteristicsPlot;
class RiuNightchartsWidget;
class RiuResultQwtPlot;

class QLabel;

namespace cvf {
    class Color3f;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuFlowCharacteristicsPlot : public QFrame
{
    Q_OBJECT;
public:
    RiuFlowCharacteristicsPlot(RimFlowCharacteristicsPlot* plotDefinition, QWidget* parent = NULL);
    virtual ~RiuFlowCharacteristicsPlot();

    void setLorenzCurve(const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues);
    void addFlowCapStorageCapCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals);
    void addSweepEfficiencyCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals);

    RimFlowCharacteristicsPlot*     ownerPlotDefinition();

protected:
    virtual QSize                   sizeHint() const override;
    virtual QSize                   minimumSizeHint() const override;

private:
    void                            setDefaults();

private:
    caf::PdmPointer<RimFlowCharacteristicsPlot> m_plotDefinition;
    QPointer<RiuResultQwtPlot> m_lorenzPlot;
    QPointer<RiuResultQwtPlot> m_flowCapVsStorageCapPlot;
    QPointer<RiuResultQwtPlot> m_sweepEffPlot;

};

