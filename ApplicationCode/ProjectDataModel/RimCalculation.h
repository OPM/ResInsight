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

#pragma once

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"

class RimCalculationVariable;

//==================================================================================================
///  
///  
//==================================================================================================
class RimCalculation : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCalculation();

    caf::PdmChildArrayFieldHandle*  variables();
    RimCalculationVariable*         addVariable();
    void                            deleteVariable(RimCalculationVariable* calcVariable);

    const std::vector<double>&      values() const;
    void                            setCalculatedValues(const std::vector<double>& values);

private:
    caf::PdmField<QString>                              m_expression;
    caf::PdmChildArrayField<RimCalculationVariable*>    m_variables;

    caf::PdmField<std::vector<double>>                  m_calculatedValues;
};
