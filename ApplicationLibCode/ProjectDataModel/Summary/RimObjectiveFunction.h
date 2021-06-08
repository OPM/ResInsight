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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>
#include <QVariant>

#include <vector>

class RimSummaryCase;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

class ObjectiveFunctionTimeConfig
{
public:
    time_t              m_startTimeStep;
    time_t              m_endTimeStep;
    std::vector<time_t> m_timeSteps;
};

//==================================================================================================
///
//==================================================================================================
class RimObjectiveFunction : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> filterChanged;

public:
    enum class FunctionType
    {
        F1 = 0,
        F2
    };

    QString                            uiName() const;
    RimObjectiveFunction::FunctionType functionType();

    /*
        void setTimeStepRange( time_t startTime, time_t endTime );
        void setTimeStepList( std::vector<time_t> timeSteps );
    */

    RimObjectiveFunction();
    void setDefaultValues( const RimSummaryCaseCollection* summaryCaseCollection );
    void setFunctionType( RimObjectiveFunction::FunctionType functionType );

    //     double value( size_t                                caseIndex,
    //                   std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses,
    //                   bool*                                 hasWarning = nullptr ) const;

    double value( RimSummaryCase*                              summaryCase,
                  const std::vector<RifEclipseSummaryAddress>& vectorSummaryAddresses,
                  const ObjectiveFunctionTimeConfig&           timeConfig,
                  bool*                                        hasWarning = nullptr ) const;

    std::pair<double, double> minMaxValues( const std::vector<RimSummaryCase*>&          summaryCases,
                                            const std::vector<RifEclipseSummaryAddress>& vectorSummaryAddresses,
                                            const ObjectiveFunctionTimeConfig&           timeConfig ) const;

    //    std::pair<time_t, time_t> range() const;

    // std::vector<double> values( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const;

    // bool isValid( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const;

    QString formulaString( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses );

    bool operator<( const RimObjectiveFunction& other ) const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    /*
        time_t              m_startTimeStep;
        time_t              m_endTimeStep;
        std::vector<time_t> m_timeSteps;
    */

    caf::PdmField<caf::AppEnum<RimObjectiveFunction::FunctionType>> m_functionType;
    caf::PdmField<bool>                                             m_divideByNumberOfObservations;
    caf::PdmField<double>                                           m_errorEstimatePercentage;
    caf::PdmField<bool>                                             m_useSquaredError;
};
