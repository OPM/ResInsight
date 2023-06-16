/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimPlotCurve.h"

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotRegressionCurve : public RimPlotCurve
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

    RimGridCrossPlotRegressionCurve();
    ~RimGridCrossPlotRegressionCurve() override = default;
    void setGroupingInformation( int dataSetIndex, int groupIndex );
    void setSamples( const std::vector<double>& xValues, const std::vector<double>& yValues );
    void setRangeDefaults( const std::vector<double>& xValues, const std::vector<double>& yValues );

    void   setCurveAutoAppearance();
    int    groupIndex() const;
    size_t sampleCount() const;
    void   determineLegendIcon();
    void   setBlackAndWhiteLegendIcons( bool blackAndWhite );

    QString getRegressionTypeString() const;

    void swapAxis();

protected:
    void    updateZoomInParentPlot() override;
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;
    void    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void    defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void    defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void    fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void    updateRectAnnotation();

    std::tuple<std::vector<double>, std::vector<double>, QString>
        calculateRegression( RimGridCrossPlotRegressionCurve::RegressionType regressionType,
                             const std::vector<double>&                      xValues,
                             const std::vector<double>&                      yValues,
                             const std::vector<double>&                      outputXValues ) const;

    static std::pair<std::vector<double>, std::vector<double>> getPositiveValues( const std::vector<double>& xValues,
                                                                                  const std::vector<double>& yValues );

private:
    caf::PdmField<caf::AppEnum<RegressionType>> m_regressionType;
    caf::PdmField<double>                       m_minRangeX;
    caf::PdmField<double>                       m_maxRangeX;
    caf::PdmField<double>                       m_minRangeY;
    caf::PdmField<double>                       m_maxRangeY;
    caf::PdmField<bool>                         m_showDataSelectionInPlot;
    caf::PdmField<int>                          m_polynomialDegree;
    caf::PdmField<QString>                      m_expressionText;
    caf::PdmField<double>                       m_minExtrapolationRangeX;
    caf::PdmField<double>                       m_maxExtrapolationRangeX;

    std::pair<double, double> m_dataRangeX;
    std::pair<double, double> m_dataRangeY;

    int m_dataSetIndex;
    int m_groupIndex;
};
