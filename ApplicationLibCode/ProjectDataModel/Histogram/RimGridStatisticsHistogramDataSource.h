/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RigHistogramData.h"
#include "RimHistogramDataSource.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimSummaryEnsemble;
class RimCase;
class RimGridView;
class RimEclipseResultDefinition;
class RimEclipseView;

//==================================================================================================
///
///
//==================================================================================================
class RimGridStatisticsHistogramDataSource : public RimHistogramDataSource
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridStatisticsHistogramDataSource();
    ~RimGridStatisticsHistogramDataSource() override;

    std::string unitNameX() const override;
    std::string unitNameY() const override;

    HistogramResult compute( RimHistogramPlot::GraphType graphType, RimHistogramPlot::FrequencyType frequencyType ) const override;

    std::string name() const override;

    void setDefaults() override;

    void cellFilterViewUpdated();

    void loadDataAndUpdate();

    void setPropertiesFromView( RimEclipseView* view );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

    RigHistogramData createStatisticsData() const;

    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimGridView*>                  m_cellFilterView;
    caf::PdmChildField<RimEclipseResultDefinition*> m_property;
    caf::PdmField<int>                              m_numBins;
};
