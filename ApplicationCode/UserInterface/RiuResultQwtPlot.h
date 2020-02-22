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

#include "RiuDockedQwtPlot.h"

#include "qwt_plot.h"

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
class RiuResultQwtPlot : public RiuDockedQwtPlot
{
    Q_OBJECT

public:
    explicit RiuResultQwtPlot( QWidget* parent = nullptr );
    ~RiuResultQwtPlot() override;

    void addCurve( const RimCase*                rimCase,
                   const QString&                curveName,
                   const cvf::Color3f&           curveColor,
                   const std::vector<QDateTime>& dateTimes,
                   const std::vector<double>&    timeHistoryValues );
    void addCurve( const RimCase*             rimCase,
                   const QString&             curveName,
                   const cvf::Color3f&        curveColor,
                   const std::vector<double>& frameTimes,
                   const std::vector<double>& timeHistoryValues );

    void deleteAllCurves();

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void  contextMenuEvent( QContextMenuEvent* ) override;

private:
    void setDefaults();

    QString asciiDataForUiSelectedCurves() const;

private slots:
    void slotCurrentPlotDataInTextDialog();

private:
    std::vector<QwtPlotCurve*> m_plotCurves;

    std::map<int, QString>                          m_caseNames;
    std::map<int, std::vector<QDateTime>>           m_timeSteps;
    std::map<int, std::vector<std::vector<double>>> m_curveData;
    std::map<int, std::vector<QString>>             m_curveNames;
};
