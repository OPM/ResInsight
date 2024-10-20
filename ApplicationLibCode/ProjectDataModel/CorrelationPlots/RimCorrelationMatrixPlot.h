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

#include "RiaCurveDataTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"

#include "cafAppEnum.h"

class RimRegularLegendConfig;
class RimSummaryAddress;

class RiuGroupedBarChartBuilder;
class RiuPlotItem;

//==================================================================================================
///
///
//==================================================================================================
template <typename KeyType, typename ValueType>
class CorrelationMatrixRowOrColumn
{
public:
    CorrelationMatrixRowOrColumn( const KeyType& key, const std::vector<double>& correlations, const std::vector<ValueType>& values )
        : m_key( key )
        , m_correlations( correlations )
        , m_values( values )
        , m_correlationSum( 0.0 )
        , m_correlationAbsSum( 0.0 )
    {
        bool anyValid = false;
        for ( auto value : correlations )
        {
            if ( RiaCurveDataTools::isValidValue( value, false ) )
            {
                m_correlationSum += value;
                m_correlationAbsSum += std::abs( value );
                anyValid = true;
            }
        }
        if ( !anyValid )
        {
            m_correlationSum    = std::numeric_limits<double>::infinity();
            m_correlationAbsSum = std::numeric_limits<double>::infinity();
        }
    }

    KeyType                m_key;
    std::vector<double>    m_correlations;
    std::vector<ValueType> m_values;
    double                 m_correlationSum;
    double                 m_correlationAbsSum;
};

using CorrelationMatrixColumn = CorrelationMatrixRowOrColumn<QString, RiaSummaryCurveDefinition>;
using CorrelationMatrixRow    = CorrelationMatrixRowOrColumn<RiaSummaryCurveDefinition, QString>;

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
    bool                    isCurveHighlightSupported() const override;
    QString                 asciiDataForPlotExport() const override;

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void onLoadDataAndUpdate() override;

    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    void updateAxes() override;

    // Private methods
    void createMatrix();
    void updatePlotTitle() override;
    void updateLegend() override;
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex ) override;

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

    std::vector<CorrelationMatrixRow> m_valuesForTextReport;
};
