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

class RiaDefines
{

public:
    enum ResultCatType
    {
        DYNAMIC_NATIVE,
        STATIC_NATIVE,
        GENERATED,
        INPUT_PROPERTY,
        FORMATION_NAMES,
        FLOW_DIAGNOSTICS,
        REMOVED
    };

    enum CompletionType {
        WELL_PATH,
        PERFORATION_INTERVAL,
        FISHBONES
    };

    static bool isPerCellFaceResult(const QString& resultName);

    static QString undefinedResultName()                { return "None"; }
    static QString undefinedGridFaultName()             { return "Undefined Grid Faults"; }
    static QString undefinedGridFaultWithInactiveName() { return "Undefined Grid Faults With Inactive"; }
    static QString combinedTransmissibilityResultName() { return "TRANXYZ"; }
    static QString ternarySaturationResultName()        { return "TERNARY"; }
    static QString combinedMultResultName()             { return "MULTXYZ"; }

    static QString riTranXResultName()                  { return "riTRANX"; }
    static QString riTranYResultName()                  { return "riTRANY"; }
    static QString riTranZResultName()                  { return "riTRANZ"; }
    static QString combinedRiTranResultName()           { return "riTRANXYZ"; }

    static QString riMultXResultName()                  { return "riMULTX"; }
    static QString riMultYResultName()                  { return "riMULTY"; }
    static QString riMultZResultName()                  { return "riMULTZ"; }
    static QString combinedRiMultResultName()           { return "riMULTXYZ"; }

    static QString riAreaNormTranXResultName()         { return "riTRANXbyArea"; }
    static QString riAreaNormTranYResultName()         { return "riTRANYbyArea"; }
    static QString riAreaNormTranZResultName()         { return "riTRANZbyArea"; }
    static QString combinedRiAreaNormTranResultName()  { return "riTRANXYZbyArea"; }

    static QString completionTypeResultName()          { return "Completion Type"; }

    // Mock model text identifiers
    static QString mockModelBasic()                    { return "Result Mock Debug Model Simple"; }
    static QString mockModelBasicWithResults()         { return "Result Mock Debug Model With Results"; }
    static QString mockModelLargeWithResults()         { return "Result Mock Debug Model Large With Results"; }
    static QString mockModelCustomized()               { return "Result Mock Debug Model Customized"; }
    static QString mockModelBasicInputCase()           { return "Input Mock Debug Model Simple"; }


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
        PLOT_AXIS_RIGHT
    };

    static double minimumDefaultValuePlot()         { return - 10.0; }
    static double maximumDefaultValuePlot()         { return  100.0; }
};

