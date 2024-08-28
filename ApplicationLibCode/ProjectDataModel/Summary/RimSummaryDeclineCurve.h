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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RimSummaryCurve.h"

#include <utility>

class RimTimeAxisAnnotation;

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

protected:
    void updateTimeAnnotations() override;

private:
    QString createCurveAutoName() override;
    QString curveExportDescription( const RifEclipseSummaryAddress& address ) const override;

    std::vector<time_t> timeStepsX() const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void appendFutureTimeSteps( std::vector<time_t>& timeSteps ) const;

    std::vector<double> createDeclineCurveValues( const std::vector<double>&                 values,
                                                  const std::vector<time_t>&                 timeSteps,
                                                  time_t                                     minTimeStep,
                                                  time_t                                     maxTimeStep,
                                                  RifEclipseSummaryAddressDefines::CurveType curveType ) const;

    static std::pair<std::vector<time_t>, std::vector<double>>
        getInRangeValues( const std::vector<time_t>& timeSteps, const std::vector<double>& values, time_t minTimeStep, time_t maxTimeStep );
    static std::vector<time_t> getTimeStepsInRange( const std::vector<time_t>& timeSteps, time_t minTimeStep, time_t maxTimeStep );

    std::set<QDateTime> createFutureTimeSteps( const std::vector<time_t>& timeSteps ) const;
    static void         appendTimeSteps( std::vector<time_t>& timeSteps, const std::set<QDateTime>& moreTimeSteps );

    static std::pair<double, double> computeInitialProductionAndDeclineRate( const std::vector<double>&                 values,
                                                                             const std::vector<time_t>&                 timeSteps,
                                                                             RifEclipseSummaryAddressDefines::CurveType curveType );

    double computePredictedValue( double                                     initialProductionRate,
                                  double                                     initialDeclineRate,
                                  double                                     timeSinceStart,
                                  RifEclipseSummaryAddressDefines::CurveType curveType ) const;

    std::pair<time_t, time_t> fullTimeStepRange() const;
    std::pair<time_t, time_t> selectedTimeStepRange() const;

private:
    caf::PdmField<caf::AppEnum<DeclineCurveType>> m_declineCurveType;
    caf::PdmField<int>                            m_predictionYears;
    caf::PdmField<double>                         m_hyperbolicDeclineConstant;

    // Time step range defined in the range [0..100] as time_t can hold values that do not fit into int used by QSpinBox
    caf::PdmField<int> m_minTimeSliderPosition;
    caf::PdmField<int> m_maxTimeSliderPosition;

    caf::PdmField<bool> m_showTimeSelectionInPlot;

    caf::PdmPointer<RimTimeAxisAnnotation> m_timeRangeAnnotation;
};
