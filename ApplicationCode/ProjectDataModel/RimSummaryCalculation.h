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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmField.h"

class RimCalculationVariable;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCalculation : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculation();

    void                            setDescription(const QString& description);
    QString                         description() const;

    caf::PdmChildArrayFieldHandle*  variables();
    RimCalculationVariable*         addVariable();
    void                            deleteVariable(RimCalculationVariable* calcVariable);

    const std::vector<double>&      values() const;
    const std::vector<time_t>&      timeSteps() const;

    bool                            parseExpression();
    bool                            calculate();
    void                            updateDependentCurvesAndPlots();
    
    virtual caf::PdmFieldHandle*    userDescriptionField() override;

    static QString                  findLeftHandSide(const QString& expresion);

private:
    RimCalculationVariable*         findByName(const QString& name) const;

    QString                         buildCalculationName() const;

private:
    caf::PdmField<QString>                              m_description;
    caf::PdmField<QString>                              m_expression;
    caf::PdmChildArrayField<RimCalculationVariable*>    m_variables;

    caf::PdmField<std::vector<double>>                  m_calculatedValues;
    caf::PdmField<std::vector<time_t>>                  m_timesteps;
};
