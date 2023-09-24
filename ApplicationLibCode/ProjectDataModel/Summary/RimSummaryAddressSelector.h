/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaDateTimeDefines.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressQMetaType.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryAddress;
class RimPlotAxisPropertiesInterface;

class RimSummaryAddressSelector : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SummaryDataSource
    {
        SINGLE_CASE,
        ENSEMBLE
    };

    caf::Signal<> addressChanged;

public:
    RimSummaryAddressSelector();

    void setSummaryCase( RimSummaryCase* summaryCase );
    void setEnsemble( RimSummaryCaseCollection* ensemble );
    void setAddress( const RifEclipseSummaryAddress& address );
    void setResamplingPeriod( RiaDefines::DateTimePeriodEnum resampling );
    void setPlotAxisProperties( RimPlotAxisPropertiesInterface* plotAxisProperties );
    void setShowDataSource( bool enable );

    RimSummaryCase*                 summaryCase() const;
    RimSummaryCaseCollection*       ensemble() const;
    RifEclipseSummaryAddress        summaryAddress() const;
    RiaDefines::DateTimePeriodEnum  resamplingPeriod() const;
    RimPlotAxisPropertiesInterface* plotAxisProperties() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmPtrField<RimSummaryCase*>                 m_summaryCase;
    caf::PdmPtrField<RimSummaryCaseCollection*>       m_summaryCaseCollection;
    caf::PdmChildField<RimSummaryAddress*>            m_summaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>           m_summaryAddressUiField;
    caf::PdmField<bool>                               m_pushButtonSelectSummaryAddress;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_plotAxisProperties;
    caf::PdmField<RiaDefines::DateTimePeriodEnum>     m_resamplingPeriod;

    SummaryDataSource m_dataSource;
    bool              m_showDataSource;
};
