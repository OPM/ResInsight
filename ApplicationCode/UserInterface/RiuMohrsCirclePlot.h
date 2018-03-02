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
#include "qwt_plot_curve.h"

#include "cafTensor3.h"

#include "cvfColor3.h"

#include <array>

class QWidget;
class QwtPlotRescaler;
class QwtPlotTextLabel;
class QwtRoundScaleDraw;
class RimGeoMechView;
class RiuSelectionItem;
class Rim3dView;
class RimGeoMechView;

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

    void updateOnSelectionChanged(const RiuSelectionItem* selectionItem);
    void clearPlot();

    void updateOnTimeStepChanged(Rim3dView* changedView);

public:
    struct MohrsCirclesInfo
    {
        cvf::Vec3f principals;
        size_t elmIndex;
        size_t i, j, k;
        double factorOfSafety;
        cvf::Color3ub color;
    };

private:
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    void addMohrCircles(const MohrsCirclesInfo& mohrsCirclesInfo);
    void deleteCircles();

    void addEnvelope(const cvf::Vec3f& principals, RimGeoMechView* view);
    void deleteEnvelopes();

    void queryDataAndUpdatePlot(RimGeoMechView* geoMechView, size_t gridIndex, size_t elmIndex, const cvf::Color3ub& color);
    
    void addMohrsCirclesInfo(const MohrsCirclesInfo& mohrsCircleInfo, RimGeoMechView* view);

    void updateTransparentCurvesOnPrincipals();
    double largestCircleRadiusInPlot() const;
    double smallestPrincipal() const;

    void replotAndScaleAxis();

    static bool isValidPrincipals(const cvf::Vec3f& principals);
    
    static float calculateFOS(const cvf::Vec3f& principals, double frictionAngle, double cohesion);

    QColor envelopeColor(RimGeoMechView* view);

    void deletePlotItems();
    
private:
    std::vector<QwtPlotItem*>  m_circlePlotItems;
    std::vector<QwtPlotCurve*> m_transparentCurves;
    
    std::map<RimGeoMechView*, QwtPlotCurve*> m_envolopePlotItems;
    std::map<RimGeoMechView*, QColor>        m_envolopeColors;

    std::vector<MohrsCirclesInfo> m_mohrsCiclesInfos;

    RimGeoMechView* m_sourceGeoMechViewOfLastPlot;

    QwtPlotRescaler* m_rescaler;
};
