/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimAbstractCorrelationPlot.h"

//==================================================================================================
///
///
//==================================================================================================
class RimParameterResultCrossPlot : public RimAbstractCorrelationPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimParameterResultCrossPlot();
    ~RimParameterResultCrossPlot() override;
    void setEnsembleParameter( const QString& ensembleParameter );

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void onLoadDataAndUpdate() override;

    void updateAxes() override;

    // Private methods
    void updatePlotTitle() override;
    void createPoints();

private:
    caf::PdmField<QString> m_ensembleParameter;

    std::pair<double, double> m_xRange;
    std::pair<double, double> m_yRange;
};
