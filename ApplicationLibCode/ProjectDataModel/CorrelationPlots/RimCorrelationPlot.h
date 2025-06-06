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

#include "Summary/RiaSummaryCurveDefinition.h" // use of caf::Signal requires this include

#include "RimAbstractCorrelationPlot.h"

#include "cafPdmFieldCvfColor.h"

#include <QDateTime>

class RimSummaryAddress;
class RiuGroupedBarChartBuilder;
class RiuPlotItem;

//==================================================================================================
///
///
//==================================================================================================
class RimCorrelationPlot : public RimAbstractCorrelationPlot
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<std::pair<QString, RiaSummaryCurveDefinition>> tornadoItemSelected;

public:
    RimCorrelationPlot();
    ~RimCorrelationPlot() override;

    bool showAbsoluteValues() const;
    void setShowAbsoluteValues( bool showAbsoluteValues );

    bool sortByAbsoluteValues() const;
    void setSortByAbsoluteValues( bool sortByAbsoluteValues );
    void selectAllParameters();

    void setShowOnlyTopNCorrelations( bool showOnlyTopNCorrelations );
    void setTopNFilterCount( int filterCount );

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void    onLoadDataAndUpdate() override;
    void    updateAxes() override;
    QString asciiDataForPlotExport() const override;

    // Private methods
    void addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder ) const;
    void updatePlotTitle() override;
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex ) override;

private:
    caf::PdmField<bool>                 m_showAbsoluteValues;
    caf::PdmField<bool>                 m_sortByAbsoluteValues;
    caf::PdmField<bool>                 m_excludeParametersWithoutVariation;
    caf::PdmField<bool>                 m_showOnlyTopNCorrelations;
    caf::PdmField<int>                  m_topNFilterCount;
    caf::PdmField<std::vector<QString>> m_selectedParametersList;
    caf::PdmField<cvf::Color3f>         m_barColor;
};
