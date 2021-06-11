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

#include "cafAppEnum.h"

#include <QDateTime>

class RimRegularLegendConfig;
class RimSummaryAddress;
class RiuGroupedBarChartBuilder;

//==================================================================================================
///
///
//==================================================================================================
class RimCorrelationMatrixPlot : public RimAbstractCorrelationPlot
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<std::pair<QString, RiaSummaryCurveDefinition>> matrixCellSelected;

public:
    enum class Sorting
    {
        NO_SORTING,
        ROWS,
        COLUMNS,
        BOTH,
    };
    using SortingEnum = caf::AppEnum<Sorting>;

public:
    RimCorrelationMatrixPlot();
    ~RimCorrelationMatrixPlot() override;

    bool                    showAbsoluteValues() const;
    bool                    sortByAbsoluteValues() const;
    RimRegularLegendConfig* legendConfig();
    void                    selectAllParameters();
    bool                    showTopNCorrelations() const;
    int                     topNFilterCount() const;

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void onLoadDataAndUpdate() override;

    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    void updateAxes() override;

    // Private methods
    void createMatrix();
    void updatePlotTitle() override;
    void updateLegend() override;
    void onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex ) override;

private:
    caf::PdmField<bool>                 m_showAbsoluteValues;
    caf::PdmField<SortingEnum>          m_sortByValues;
    caf::PdmField<bool>                 m_sortByAbsoluteValues;
    caf::PdmField<bool>                 m_excludeParametersWithoutVariation;
    caf::PdmField<bool>                 m_showOnlyTopNCorrelations;
    caf::PdmField<int>                  m_topNFilterCount;
    caf::PdmField<std::vector<QString>> m_selectedParametersList;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    std::map<size_t, QString> m_paramLabels;
    std::map<size_t, QString> m_resultLabels;
};
