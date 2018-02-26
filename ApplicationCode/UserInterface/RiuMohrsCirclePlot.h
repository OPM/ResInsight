/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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
#include "qwt_plot_item.h"

#include <array>

class RiuSelectionItem;
class RimGeoMechView;
class QwtRoundScaleDraw;
class QwtPlotRescaler;
class QWidget;
class QwtPlotCurve;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuMohrsCirclePlot : public QwtPlot
{
    Q_OBJECT

public:
    RiuMohrsCirclePlot(QWidget* parent);
    ~RiuMohrsCirclePlot();

    void setPrincipals(double p1, double p2, double p3);
    void setPrincipalsAndRedrawPlot(double p1, double p2, double p3);

    void updateOnSelectionChanged(const RiuSelectionItem* selectionItem);
    void clearPlot();

protected:
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    void redrawCircles();
    void deleteCircles();

    void redrawEnvelope();
    void deleteEnvelope();

    void queryDataAndUpdatePlot(RimGeoMechView* geoMechView, size_t gridIndex, size_t cellIndex);
    
private:
    struct MohrCircle
    {
        MohrCircle(size_t component, double radius, double centerX)
            : component(component), radius(radius), centerX(centerX) {}
        MohrCircle() {};
        size_t component; //1, 2 or 3
        double radius;
        double centerX;
    };

private:
    void setDefaults();
    void createMohrCircles();

    void setFrictionAngle(double frictionAngle);
    void setCohesion(double cohesion);

    void updateTransparentCurveOnPrincipals();

    void replotAndScaleAxis();

private:
    double m_principal1;
    double m_principal2;
    double m_principal3;

    std::array<MohrCircle, 3> m_mohrCircles;

    std::vector<QwtPlotItem*> m_circlePlotItems;
    
    QwtPlotCurve* m_envolopePlotItem;
    QwtPlotCurve* m_transparentCurve;

    double m_frictionAngle;
    double m_cohesion;
    double m_factorOfSafety;

    QwtPlotRescaler* m_rescaler;
};
