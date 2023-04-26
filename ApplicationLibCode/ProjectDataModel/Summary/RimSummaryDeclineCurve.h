/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RimSummaryCurve.h"

#include "cafAppEnum.h"

#include <utility>

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryDeclineCurve : public RimSummaryCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class DeclineCurveType
    {
        EXPONENTIAL,
        HARMONIC,
        HYPERBOLIC
    };

    RimSummaryDeclineCurve();
    ~RimSummaryDeclineCurve() override;

    void setDeclineCurveType( DeclineCurveType declineCurveType );

    // Y Axis functions
    std::vector<double> valuesY() const override;
    std::vector<time_t> timeStepsY() const override;

    // X Axis functions
    std::vector<double> valuesX() const override;

private:
    QString createCurveAutoName() override;

    std::vector<time_t> timeStepsX() const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void appendFutureTimeSteps( std::vector<time_t>& timeSteps ) const;

    std::vector<double>
        createDeclineCurveValues( const std::vector<double>& values, const std::vector<time_t>& timeSteps, bool isAccumulatedResult ) const;

    std::set<QDateTime> createFutureTimeSteps( const std::vector<time_t>& timeSteps ) const;
    static void         appendTimeSteps( std::vector<time_t>& timeSteps, const std::set<QDateTime>& moreTimeSteps );

    static std::pair<double, double> computeInitialProductionAndDeclineRate( const std::vector<double>& values,
                                                                             const std::vector<time_t>& timeSteps,
                                                                             bool                       isAccumulatedResult );

    double computePredictedValue( double initialProductionRate, double initialDeclineRate, double timeSinceStart, bool isAccumulatedResult ) const;

    caf::PdmField<caf::AppEnum<DeclineCurveType>> m_declineCurveType;
    caf::PdmField<int>                            m_predictionYears;
    caf::PdmField<double>                         m_hyperbolicDeclineConstant;
};
