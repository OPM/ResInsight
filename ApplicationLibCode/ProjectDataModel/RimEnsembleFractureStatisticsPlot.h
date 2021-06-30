/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaDefines.h"

#include "RigEnsembleFractureStatisticsCalculator.h"

#include "RimEnsembleFractureStatistics.h"
#include "RimStatisticsPlot.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimEnsembleFractureStatistics;

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleFractureStatisticsPlot : public RimStatisticsPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleFractureStatisticsPlot();
    ~RimEnsembleFractureStatisticsPlot() override;

protected:
    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    QString createAutoName() const override;
    QString createXAxisTitle() const override;

    void             setDefaults();
    bool             hasStatisticsData() const override;
    RigHistogramData createStatisticsData() const override;

protected:
    caf::PdmPtrField<RimEnsembleFractureStatistics*>                                   m_ensembleFractureStatistics;
    caf::PdmField<caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>> m_property;
};
