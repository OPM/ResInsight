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

#include "RiuInterfaceToViewWindow.h"

#include "cafPdmPointer.h"

#include "qwt_plot.h"

#include <QFrame>
#include <QPointer>

class RimFlowCharacteristicsPlot;
class RiuNightchartsWidget;
class RiuResultQwtPlot;
class RiuLineSegmentQwtPlotCurve;

class QLabel;

namespace cvf {
    class Color3f;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuFlowCharacteristicsPlot : public QFrame, public RiuInterfaceToViewWindow
{
    Q_OBJECT;
public:
    RiuFlowCharacteristicsPlot(RimFlowCharacteristicsPlot* plotDefinition, QWidget* parent = nullptr);
    virtual ~RiuFlowCharacteristicsPlot();

    void setLorenzCurve(const QStringList& dateTimeStrings, const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues);
    void addFlowCapStorageCapCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals);
    void addSweepEfficiencyCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals);

    void removeAllCurves();
    void zoomAll();

    void showLegend(bool show);

    RimFlowCharacteristicsPlot*     ownerPlotDefinition();
    virtual RimViewWindow*          ownerViewWindow() const override;

    static void                        addWindowZoom(QwtPlot* plot);
    static RiuLineSegmentQwtPlotCurve* createEmptyCurve(QwtPlot* plot, const QString& curveName, const QColor& curveColor);

protected:
    virtual QSize                   sizeHint() const override;
    virtual QSize                   minimumSizeHint() const override;

private:
    void                            setDefaults();
    void                            initializeColors(const std::vector<QDateTime>& dateTimes);
    
    static void                     addCurveWithLargeSymbol(QwtPlot* plot, const QString& curveName, const QColor& color, const QDateTime& dateTime, double timeHistoryValue);

private:
    caf::PdmPointer<RimFlowCharacteristicsPlot> m_plotDefinition;
    QPointer<QwtPlot>                           m_lorenzPlot;
    QPointer<QwtPlot>                           m_flowCapVsStorageCapPlot;
    QPointer<QwtPlot>                           m_sweepEffPlot;

    std::map<QDateTime, QColor>                 m_dateToColorMap;
};

