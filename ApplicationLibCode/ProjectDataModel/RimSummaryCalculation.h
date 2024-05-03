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

#include <optional>

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCalculationAddress
{
public:
    RimSummaryCalculationAddress( RifEclipseSummaryAddress summaryAddress )
        : m_summaryAddress( summaryAddress )
    {
    }

    RifEclipseSummaryAddress address() const { return m_summaryAddress; }

private:
    RifEclipseSummaryAddress m_summaryAddress;
};

struct SummaryCalculationVariable
{
    QString                  name;
    RimSummaryCase*          summaryCase;
    RifEclipseSummaryAddress summaryAddress;
};

class RimSummaryCalculation : public RimUserDefinedCalculation
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculation();

    std::vector<RimSummaryCalculationAddress> allAddressesForSummaryCase( RimSummaryCase* summaryCase ) const;

    std::vector<double> values( RimSummaryCase* summaryCase, const RimSummaryCalculationAddress& addr );
    std::vector<time_t> timeSteps( RimSummaryCase* summaryCase, const RimSummaryCalculationAddress& addr );

    bool calculate() override;
    void updateDependentObjects() override;
    void removeDependentObjects() override;

    QString buildCalculationName() const override;

    void setDistributeToOtherItems( bool enable );
    void setDistributeToAllCases( bool enable );

    bool isDistributeToOtherItems() const;
    bool isDistributeToAllCases() const;

protected:
    RimSummaryCalculationVariable* createVariable() override;

    static std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
        calculateResult( const QString&                                 expression,
                         const std::vector<SummaryCalculationVariable>& variables,
                         RimSummaryCase*                                summaryCaseForSubstitution );

    std::optional<std::pair<std::vector<double>, std::vector<time_t>>> calculateWithSubstitutions( RimSummaryCase* summaryCase,
                                                                                                   const RifEclipseSummaryAddress& addr );

    static void substituteVariables( std::vector<SummaryCalculationVariable>& vars, const RifEclipseSummaryAddress& address );

    std::vector<RimSummaryCalculationAddress> allAddressesForCategory( RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                       const std::set<RifEclipseSummaryAddress>& allResultAddresses ) const;

    RimSummaryCalculationAddress singleAddressesForCategory( const RifEclipseSummaryAddress& address ) const;

    std::optional<std::vector<SummaryCalculationVariable>> getVariables() const;

    bool checkVariables() const;
    bool detectCyclicCalculation( int id, std::set<int>& ids ) const;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<bool> m_distributeToOtherItems;
    caf::PdmField<bool> m_distributeToAllCases;
};
