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

#include "RiaDefines.h"
#include "cafAppEnum.h"


namespace caf
{
    template<>
    void caf::AppEnum< RiaDefines::ResultCatType >::setUp()
    {
        addItem(RiaDefines::DYNAMIC_NATIVE, "DYNAMIC_NATIVE",   "Dynamic");
        addItem(RiaDefines::STATIC_NATIVE,  "STATIC_NATIVE",    "Static");
        addItem(RiaDefines::GENERATED,      "GENERATED",        "Generated");
        addItem(RiaDefines::INPUT_PROPERTY, "INPUT_PROPERTY",   "Input Property");
        addItem(RiaDefines::FORMATION_NAMES, "FORMATION_NAMES", "Formation Names");
        addItem(RiaDefines::FLOW_DIAGNOSTICS, "FLOW_DIAGNOSTICS", "Flow Diagnostics");
        setDefault(RiaDefines::DYNAMIC_NATIVE);
    }

    template<>
    void caf::AppEnum< RiaDefines::DepthUnitType >::setUp()
    {
        addItem(RiaDefines::UNIT_METER,  "UNIT_METER",   "Meter");
        addItem(RiaDefines::UNIT_FEET,   "UNIT_FEET",    "Feet");
        addItem(RiaDefines::UNIT_NONE,   "UNIT_NONE",    "None");


        setDefault(RiaDefines::UNIT_METER);
    }

    template<>
    void caf::AppEnum< RiaDefines::PlotAxis >::setUp()
    {
        addItem(RiaDefines::PLOT_AXIS_LEFT,  "PLOT_AXIS_LEFT",  "Left");
        addItem(RiaDefines::PLOT_AXIS_RIGHT, "PLOT_AXIS_RIGHT", "Right");

        setDefault(RiaDefines::PLOT_AXIS_LEFT);
    }

    template<>
    void caf::AppEnum< RiaDefines::CompletionType >::setUp()
    {
        addItem(RiaDefines::WELL_PATH, "WELL_PATH", "Well Path");
        addItem(RiaDefines::PERFORATION_INTERVAL, "PERFORATION_INTERVAL", "Perforation Interval");
        addItem(RiaDefines::FISHBONES, "FISHBONES", "Fishbones");

        setDefault(RiaDefines::WELL_PATH);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isPerCellFaceResult(const QString& resultName)
{
    if (resultName.compare(RiaDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RiaDefines::combinedMultResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RiaDefines::ternarySaturationResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RiaDefines::combinedRiTranResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RiaDefines::combinedRiMultResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RiaDefines::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::undefinedResultName()
{
    return "None";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::undefinedGridFaultName()
{
    return "Undefined Grid Faults";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::undefinedGridFaultWithInactiveName()
{
    return "Undefined Grid Faults With Inactive";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedTransmissibilityResultName()
{
    return "TRANXYZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::ternarySaturationResultName()
{
    return "TERNARY";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedMultResultName()
{
    return "MULTXYZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranXResultName()
{
    return "riTRANX";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranYResultName()
{
    return "riTRANY";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riTranZResultName()
{
    return "riTRANZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiTranResultName()
{
    return "riTRANXYZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultXResultName()
{
    return "riMULTX";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultYResultName()
{
    return "riMULTY";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riMultZResultName()
{
    return "riMULTZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiMultResultName()
{
    return "riMULTXYZ";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranXResultName()
{
    return "riTRANXbyArea";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranYResultName()
{
    return "riTRANYbyArea";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::riAreaNormTranZResultName()
{
    return "riTRANZbyArea";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::combinedRiAreaNormTranResultName()
{
    return "riTRANXYZbyArea";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::completionTypeResultName()
{
    return "Completion Type";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasic()
{
    return "Result Mock Debug Model Simple";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasicWithResults()
{
    return "Result Mock Debug Model With Results";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelLargeWithResults()
{
    return "Result Mock Debug Model Large With Results";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelCustomized()
{
    return "Result Mock Debug Model Customized";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaDefines::mockModelBasicInputCase()
{
    return "Input Mock Debug Model Simple";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RiaDefines::minimumDefaultValuePlot()
{
    return -10.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RiaDefines::maximumDefaultValuePlot()
{
    return  100.0;
}
