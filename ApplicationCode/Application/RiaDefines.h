/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include <QString>

namespace RiaDefines
{
    enum ResultCatType
    {
        DYNAMIC_NATIVE,
        STATIC_NATIVE,
        SOURSIMRL,
        GENERATED,
        INPUT_PROPERTY,
        FORMATION_NAMES,
        FLOW_DIAGNOSTICS,
        INJECTION_FLOODING,
        REMOVED
    };

    enum CompletionType {
        WELL_PATH,
        PERFORATION_INTERVAL,
        FISHBONES,
        FRACTURE,
    };

    bool isPerCellFaceResult(const QString& resultName);

    QString undefinedResultName();
    QString undefinedGridFaultName();
    QString undefinedGridFaultWithInactiveName();
    QString combinedTransmissibilityResultName();
    QString combinedWaterFluxResultName();
    QString combinedOilFluxResultName();
    QString combinedGasFluxResultName();

    QString ternarySaturationResultName();
    QString combinedMultResultName();

    QString riTranXResultName();
    QString riTranYResultName();
    QString riTranZResultName();
    QString combinedRiTranResultName();

    QString riMultXResultName();
    QString riMultYResultName();
    QString riMultZResultName();
    QString combinedRiMultResultName();

    QString riAreaNormTranXResultName();
    QString riAreaNormTranYResultName();
    QString riAreaNormTranZResultName();
    QString combinedRiAreaNormTranResultName();

    QString mobilePoreVolumeName();

    QString completionTypeResultName();

    // Mock model text identifiers
    QString mockModelBasic();
    QString mockModelBasicWithResults();
    QString mockModelLargeWithResults();
    QString mockModelCustomized();
    QString mockModelBasicInputCase();

    QString activeFormationNamesResultName();

    //Units and conversions
    enum DepthUnitType
    {
        UNIT_METER,
        UNIT_FEET,
        UNIT_NONE
    };



    // Defines relate to plotting
    enum PlotAxis
    {
        PLOT_AXIS_LEFT,
        PLOT_AXIS_RIGHT,
        PLOT_AXIS_BOTTOM
    };

    double minimumDefaultValuePlot();
    double maximumDefaultValuePlot();

    enum PhaseType {
        OIL_PHASE,
        GAS_PHASE,
        WATER_PHASE
    };
};

