/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimQwtPlotCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

#include <QPointF>
#include <QVector>

class RimCase;
class RimEclipseResultDefinition;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotCurve : public RimQwtPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCurve();
    ~RimGridCrossPlotCurve() override = default;
    void setGroupingInformation( int dataSetIndex, int groupIndex );
    void setSamples( const std::vector<double>& xValues, const std::vector<double>& yValues );

    void   setCurveAutoAppearance();
    int    groupIndex() const;
    size_t sampleCount() const;
    void   determineLegendIcon();
    void   setBlackAndWhiteLegendIcons( bool blackAndWhite );

protected:
    void    determineSymbol();
    void    updateZoomInParentPlot() override;
    void    updateLegendsInPlot() override;
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;
    void    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    int m_dataSetIndex;
    int m_groupIndex;
};
