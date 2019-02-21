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

#include "RimPlotCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

#include <QPointF>
#include <QVector>

class RimCase;
class RimEclipseResultDefinition;
class QwtPlotCurve;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCurve();
    ~RimGridCrossPlotCurve() override = default;
    void determineColorAndSymbol(int curveSetIndex, int categoryIndex, bool contrastColors = false);
    void setSamples(const QVector<QPointF>& samples);

protected:
    void updateZoomInParentPlot() override;
    void updateLegendsInPlot() override;
    QString createCurveAutoName() override;
    void onLoadDataAndUpdate(bool updateParentPlot) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
};

