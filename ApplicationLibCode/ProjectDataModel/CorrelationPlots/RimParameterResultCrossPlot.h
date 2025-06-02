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

#include "RimAbstractCorrelationPlot.h"

#include <array>
#include <map>

class RiuQwtPlotRectAnnotation;

//==================================================================================================
///
///
//==================================================================================================
class RimParameterResultCrossPlot : public RimAbstractCorrelationPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimParameterResultCrossPlot();
    ~RimParameterResultCrossPlot() override;
    void    setEnsembleParameter( const QString& ensembleParameter );
    QString ensembleParameter() const;

    std::vector<RimSummaryCase*> summaryCasesExcludedByFilter() const;
    void                         appendFilterFields( caf::PdmUiOrdering& uiOrdering );

    void detachAllCurves() override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void    onLoadDataAndUpdate() override;
    void    updateAxes() override;
    QString asciiDataForPlotExport() const override;
    void    updatePlotTitle() override;
    void    createPoints();

    void updateValueRanges();
    void updateFilterRanges();

    QString excludedCasesText() const;

    struct CaseData
    {
        double          parameterValue;
        double          summaryValue;
        RimSummaryCase* summaryCase;
    };

    std::vector<CaseData> createCaseData() const;

    size_t hashFromCurrentData() const;
    void   writeDataToCache();

private:
    caf::PdmField<QString> m_ensembleParameter;

    caf::PdmField<bool>                      m_useParameterFilter;
    caf::PdmField<std::pair<double, double>> m_summaryFilterRange;
    caf::PdmField<std::pair<double, double>> m_parameterFilterRange;
    caf::PdmProxyValueField<QString>         m_excludedCasesText;

    std::pair<double, double> m_xValueRange;
    std::pair<double, double> m_yValueRange;

    std::unique_ptr<RiuQwtPlotRectAnnotation> m_rectAnnotation;

    std::map<size_t, std::array<double, 4>> m_filterValueRangeCache;
};
