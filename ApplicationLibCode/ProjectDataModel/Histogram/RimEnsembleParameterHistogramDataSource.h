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

class RimSummaryEnsemble;

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleParameterHistogramDataSource : public RimHistogramDataSource
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleParameterHistogramDataSource();
    ~RimEnsembleParameterHistogramDataSource() override;

    void    setEnsembleParameter( const QString& ensembleParameter );
    QString ensembleParameter() const;

    void                setEnsemble( RimSummaryEnsemble* ensemble );
    RimSummaryEnsemble* ensemble() const;

    std::string unitNameX() const override;
    std::string unitNameY() const override;

    HistogramResult compute( RimHistogramPlot::GraphType graphType, RimHistogramPlot::FrequencyType frequencyType ) const override;

    void setDefaults() override;

    std::string name() const override;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmPtrField<RimSummaryEnsemble*> m_ensemble;
    caf::PdmField<QString>                m_parameter;
    caf::PdmField<int>                    m_numBins;
};
