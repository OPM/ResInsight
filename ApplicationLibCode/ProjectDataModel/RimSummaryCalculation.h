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

    std::vector<double>        values( const RimUserDefinedCalculationAddress& addr ) override;
    const std::vector<time_t>& timeSteps( const RimUserDefinedCalculationAddress& addr );

    bool calculate() override;
    void updateDependentObjects() override;
    void removeDependentObjects() override;

    QString buildCalculationName() const override;

protected:
    RimSummaryCalculationVariable* createVariable() override;

    static std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
        calculateResult( const QString& expression, const std::vector<RimSummaryCalculationVariable*>& variables );

    std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
        calculateWithSubstitutions( const RifEclipseSummaryAddress& addr );

    static void substituteVariables( std::vector<RimSummaryCalculationVariable*>& vars,
                                     const RifEclipseSummaryAddress&              address );

    std::optional<std::vector<RimSummaryCalculationVariable*>> getVariables( bool showError = true ) const;

    std::map<RifEclipseSummaryAddress, std::vector<double>> m_cachedResults;
    std::map<RifEclipseSummaryAddress, std::vector<time_t>> m_cachedTimesteps;
};
