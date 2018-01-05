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

#include "cafPdmPointer.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QList>
#include <QPointer>
#include <QWidget>


#include "RiuInterfaceToViewWindow.h"

class RimTofAccumulatedPhaseFractionsPlot;

//==================================================================================================
//
// RiuTofAccumulatedPhaseFractionsPlot
//
//==================================================================================================
class RiuTofAccumulatedPhaseFractionsPlot : public QwtPlot, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuTofAccumulatedPhaseFractionsPlot(RimTofAccumulatedPhaseFractionsPlot* plotDefinition, QWidget* parent = NULL);
    virtual ~RiuTofAccumulatedPhaseFractionsPlot();

    RimTofAccumulatedPhaseFractionsPlot*                 ownerPlotDefinition();
    virtual RimViewWindow*          ownerViewWindow() const override;

    void                            setSamples(std::vector<double> xSamples,
                                               std::vector<double> watValues,
                                               std::vector<double> oilValues,
                                               std::vector<double> gasValues,
                                               int maxTofYears);

protected:
    virtual QSize                   sizeHint() const override;
    virtual int                     heightForWidth(int w) const override;

private:
    void         setDefaults();
    static void  setCommonPlotBehaviour(QwtPlot* plot);
    void         setCurveColor(QwtPlotCurve* curve, QColor color);

private:
    caf::PdmPointer<RimTofAccumulatedPhaseFractionsPlot> m_plotDefinition;

    QwtPlotCurve* m_watCurve;
    QwtPlotCurve* m_oilCurve;
    QwtPlotCurve* m_gasCurve;

    std::vector<double> m_watValues;
    std::vector<double> m_oilValues;
    std::vector<double> m_gasValues;
    std::vector<double> m_xValues;
};

