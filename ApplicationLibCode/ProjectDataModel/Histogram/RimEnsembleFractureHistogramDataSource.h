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

#include "RimHistogramDataSource.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "RigEnsembleFractureStatisticsCalculator.h"

class RimEnsembleFractureStatistics;

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleFractureHistogramDataSource : public RimHistogramDataSource
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleFractureHistogramDataSource();
    ~RimEnsembleFractureHistogramDataSource() override;

    void setEnsembleFractureStatistics( RimEnsembleFractureStatistics* statistics );
    void setProperty( RigEnsembleFractureStatisticsCalculator::PropertyType property );

    std::string unitNameX() const override;
    std::string unitNameY() const override;

    HistogramResult compute( RimHistogramPlot::GraphType graphType, RimHistogramPlot::FrequencyType frequencyType ) const override;

    void setDefaults() override;

    std::string name() const override;

    void loadDataAndUpdate();

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmPtrField<RimEnsembleFractureStatistics*>                                   m_ensembleFractureStatistics;
    caf::PdmField<caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>> m_property;
    caf::PdmField<int>                                                                 m_numBins;
};
