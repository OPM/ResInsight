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
#include "regression-analysis/src/PowerFitRegression.hpp"

#include <utility>

namespace regression
{
class LinearRegression;
class PolynominalRegression;
class PowerFitRegression;
} // namespace regression

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
        POLYNOMINAL,
        POWER_FIT
    };

    RimSummaryRegressionAnalysisCurve();
    ~RimSummaryRegressionAnalysisCurve() override;

    // Y Axis functions
    std::vector<double> valuesY() const override;
    std::vector<time_t> timeStepsY() const override;

    // X Axis functions
    std::vector<double> valuesX() const override;
    std::vector<time_t> timeStepsX() const override;

private:
    void onLoadDataAndUpdate( bool updateParentPlot ) override;

    QString createCurveAutoName() override;
    QString curveExportDescription( const RifEclipseSummaryAddress& address ) const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    std::tuple<std::vector<time_t>, std::vector<double>, QString> computeRegressionCurve( const std::vector<time_t>& timeSteps,
                                                                                          const std::vector<double>& values ) const;

    static QString generateRegressionText( const regression::LinearRegression& reg );
    static QString generateRegressionText( const regression::PolynominalRegression& reg );
    static QString generateRegressionText( const regression::PowerFitRegression& reg );

    static QString formatDouble( double v );

    caf::PdmField<caf::AppEnum<RegressionType>> m_regressionType;
    caf::PdmField<int>                          m_polynominalDegree;
    caf::PdmField<QString>                      m_expressionText;

    std::vector<double> m_valuesX;
    std::vector<time_t> m_timeStepsX;
    std::vector<double> m_valuesY;
    std::vector<time_t> m_timeStepsY;
};
