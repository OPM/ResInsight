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

namespace regression
{
class ExponentialRegression;
class LinearRegression;
class LogarithmicRegression;
class PolynomialRegression;
class PowerFitRegression;
} // namespace regression

class RimTimeAxisAnnotation;
class RimEnsembleCurveSet;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryRegressionAnalysisCurve : public RimSummaryCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class RegressionType
    {
        LINEAR,
        POLYNOMIAL,
        POWER_FIT,
        EXPONENTIAL,
        LOGARITHMIC,
        LOGISTIC
    };

    enum class ForecastUnit
    {
        DAYS,
        MONTHS,
        YEARS,
    };

    enum class DataSource
    {
        SUMMARY_ADDRESS,
        ENSEMBLE
    };

    enum class RangeType
    {
        FULL_RANGE,
        USER_DEFINED_RANGE
    };

    RimSummaryRegressionAnalysisCurve();

    void setEnsembleCurveSet( RimEnsembleCurveSet* ensembleCurveSet );

    // Y Axis functions
    std::vector<double> valuesY() const override;
    std::vector<time_t> timeStepsY() const override;

    // X Axis functions
    std::vector<double> valuesX() const override;
    std::vector<time_t> timeStepsX() const override;
    static std::vector<time_t>
        getOutputTimeSteps( const std::vector<time_t>& timeSteps, int forecastBackward, int forecastForward, ForecastUnit forecastUnit );

    bool isRegressionCurve() const override;

    std::vector<RimTimeAxisAnnotation*> createTimeAnnotations() const;

protected:
    void updateTimeAnnotations() override;

private:
    void onLoadDataAndUpdate( bool updateParentPlot ) override;

    void extractSourceCurveData();
    void updateDefaultValues();

    QString createCurveAutoName() override;
    QString curveExportDescription( const RifEclipseSummaryAddress& address ) const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    std::tuple<std::vector<time_t>, std::vector<double>, QString> computeRegressionCurve( const std::vector<time_t>& timeSteps,
                                                                                          const std::vector<double>& values ) const;

    static std::vector<double> convertToDouble( const std::vector<time_t>& timeSteps );
    static std::vector<time_t> convertToTimeT( const std::vector<double>& timeSteps );

    static std::pair<std::vector<double>, std::vector<double>> getPositiveValues( const std::vector<double>& timeSteps,
                                                                                  const std::vector<double>& values );

    static std::pair<std::vector<time_t>, std::vector<double>>
        getInRangeValues( const std::vector<time_t>& timeSteps, const std::vector<double>& values, time_t minTimeStep, time_t maxTimeStep );

    static QString generateRegressionText( const regression::LinearRegression& reg );
    static QString generateRegressionText( const regression::PolynomialRegression& reg );
    static QString generateRegressionText( const regression::PowerFitRegression& reg );
    static QString generateRegressionText( const regression::LogarithmicRegression& reg );
    static QString generateRegressionText( const regression::ExponentialRegression& reg );

    static void appendTimeSteps( std::vector<time_t>& destinationTimeSteps, const std::set<QDateTime>& sourceTimeSteps );

    std::pair<time_t, time_t> fullTimeStepRange() const;
    std::pair<time_t, time_t> selectedTimeStepRange() const;

private:
    caf::PdmField<caf::AppEnum<DataSource>>                                      m_dataSourceForRegression;
    caf::PdmPtrField<RimEnsembleCurveSet*>                                       m_ensembleCurveSet;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddressDefines::StatisticsType>> m_ensembleStatisticsType;

    caf::PdmField<caf::AppEnum<RegressionType>> m_regressionType;

    caf::PdmField<caf::AppEnum<RangeType>> m_timeRangeSelection;
    caf::PdmField<int>                     m_minTimeSliderPosition;
    caf::PdmField<int>                     m_maxTimeSliderPosition;
    caf::PdmField<bool>                    m_showTimeSelectionInPlot;

    caf::PdmField<int>                        m_polynomialDegree;
    caf::PdmField<QString>                    m_expressionText;
    caf::PdmField<int>                        m_forecastForward;
    caf::PdmField<int>                        m_forecastBackward;
    caf::PdmField<caf::AppEnum<ForecastUnit>> m_forecastUnit;

    caf::PdmField<caf::AppEnum<RangeType>>   m_xRangeSelection;
    caf::PdmField<std::pair<double, double>> m_valueRangeX;

    caf::PdmField<caf::AppEnum<RangeType>>   m_yRangeSelection;
    caf::PdmField<std::pair<double, double>> m_valueRangeY;

    std::vector<double> m_valuesX;
    std::vector<time_t> m_timeStepsX;
    std::vector<double> m_valuesY;
    std::vector<time_t> m_timeStepsY;

    // Source curve data
    std::vector<double> m_sourceValuesX;
    std::vector<time_t> m_sourceTimeStepsX;
    std::vector<double> m_sourceValuesY;
    std::vector<time_t> m_sourceTimeStepsY;
};
