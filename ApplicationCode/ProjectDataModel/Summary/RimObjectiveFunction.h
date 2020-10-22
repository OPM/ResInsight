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

#include <QString>
#include <QVariant>

#include <vector>

class RimSummaryCase;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RimObjectiveFunction
{
public:
    enum class FunctionType
    {
        M1 = 0,
        M2
    };

    QString                            uiName() const { return name; };
    QString                            name;
    RimObjectiveFunction::FunctionType functionType();

    void setTimeStepRange( time_t startTime, time_t endTime );
    void setTimeStepList( std::vector<time_t> timeSteps );

    double minValue;
    double maxValue;

    RimObjectiveFunction( const RimSummaryCaseCollection* summaryCaseCollection, FunctionType type );

    double value( size_t                                       caseIndex,
                  std::vector<const RifEclipseSummaryAddress&> vectorSummaryAddresses,
                  bool*                                        hasWarning = nullptr ) const;

    double value( RimSummaryCase*                              summaryCase,
                  std::vector<const RifEclipseSummaryAddress&> vectorSummaryAddresses,
                  bool*                                        hasWarning = nullptr ) const;

    std::pair<double, double> minMaxValues( std::vector<const RifEclipseSummaryAddress&> vectorSummaryAddresses ) const;

    std::pair<time_t, time_t> range() const;

    std::vector<double> values( std::vector<const RifEclipseSummaryAddress&> vectorSummaryAddresses ) const;

    bool isValid( std::vector<const RifEclipseSummaryAddress&> vectorSummaryAddresses ) const;

    bool operator<( const RimObjectiveFunction& other ) const;

private:
    const RimSummaryCaseCollection* m_summaryCaseCollection;

    time_t              m_startTimeStep;
    time_t              m_endTimeStep;
    std::vector<time_t> m_timeSteps;
    FunctionType        m_functionType;
};
