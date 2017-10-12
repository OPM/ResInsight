/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimCalculation.h"

#include "RimCalculationVariable.h"

CAF_PDM_SOURCE_INIT(RimCalculation, "RimCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculation::RimCalculation()
{
    CAF_PDM_InitObject("RimCalculation", ":/octave.png", "Calculation", "");

    CAF_PDM_InitFieldNoDefault(&m_expression, "Expression", "Expression", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variables, "Variables", "Variables", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_calculatedValues, "CalculatedValues", "Calculated Values", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* RimCalculation::variables()
{
    return &m_variables;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationVariable* RimCalculation::addVariable()
{
    RimCalculationVariable* v = new RimCalculationVariable;
    m_variables.push_back(v);

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculation::deleteVariable(RimCalculationVariable* calcVariable)
{
    m_variables.removeChildObject(calcVariable);

    delete calcVariable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimCalculation::values() const
{
    return m_calculatedValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculation::setCalculatedValues(const std::vector<double>& values)
{
    m_calculatedValues = values;
}
