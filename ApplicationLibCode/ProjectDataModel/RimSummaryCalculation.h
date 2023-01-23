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

#include "RimSummaryCalculationVariable.h"
#include "RimUserDefinedCalculation.h"

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCalculationAddress : public RimUserDefinedCalculationAddress
{
public:
    RimSummaryCalculationAddress( RifEclipseSummaryAddress summaryAddress )
        : m_summaryAddress( summaryAddress )
    {
    }

    std::string name() const override { return "Summary calculation"; }

    RifEclipseSummaryAddress address() const { return m_summaryAddress; }

private:
    RifEclipseSummaryAddress m_summaryAddress;
};

class RimSummaryCalculation : public RimUserDefinedCalculation
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculation();

    std::vector<RimUserDefinedCalculationAddress*> allAddresses() const override;

    bool calculate() override;
    void updateDependentObjects() override;
    void removeDependentObjects() override;

protected:
    RimSummaryCalculationVariable* createVariable() override;
};
