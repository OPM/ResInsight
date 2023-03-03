/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiuDockedQwtPlot.h"

#include <QString>
#include <map>
#include <vector>

class RimCase;
class QwtPlotCurve;
class QwtPlotGrid;

namespace cvf
{
class Color3f;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuDepthQwtPlot : public RiuDockedQwtPlot
{
    Q_OBJECT

public:
    explicit RiuDepthQwtPlot( QWidget* parent = nullptr );
    ~RiuDepthQwtPlot() override;

    void addCurve( const RimCase*             rimCase,
                   const QString&             curveName,
                   const cvf::Color3f&        curveColor,
                   const std::vector<int>&    kIndexes,
                   const std::vector<double>& depthValues,
                   const std::vector<double>& resultValues );

    void deleteAllCurves();

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void  contextMenuEvent( QContextMenuEvent* ) override;

private:
    void setDefaults();
    void resetRanges();
    void updateAxisScaling();

    QString asciiDataForUiSelectedCurves() const;

private slots:
    void slotCurrentPlotDataInTextDialog();

private:
    std::vector<QwtPlotCurve*> m_plotCurves;

    std::map<int, QString>                          m_caseNames;
    std::map<int, std::vector<int>>                 m_kSteps;
    std::map<int, std::vector<double>>              m_depthValues;
    std::map<int, std::vector<std::vector<double>>> m_curveData;
    std::map<int, std::vector<QString>>             m_curveNames;

    double m_maxY;
    double m_minY;
    double m_maxX;
    double m_minX;

    bool m_bShowDepth;
};
