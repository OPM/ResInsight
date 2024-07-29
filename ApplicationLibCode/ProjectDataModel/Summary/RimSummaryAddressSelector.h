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

#include "RimPlotAxisProperties.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryAddress;
class RimPlotAxisPropertiesInterface;

class RimSummaryAddressSelector : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> addressChanged;

public:
    RimSummaryAddressSelector();

    void setSummaryCase( RimSummaryCase* summaryCase );
    void setEnsemble( RimSummaryEnsemble* ensemble );
    void setAddress( const RifEclipseSummaryAddress& address );
    void setResamplingPeriod( RiaDefines::DateTimePeriodEnum resampling );
    void setPlotAxisProperties( RimPlotAxisPropertiesInterface* plotAxisProperties );
    void setAxisOrientation( RimPlotAxisProperties::Orientation orientation );

    void setShowDataSource( bool enable );
    void setShowResampling( bool enable );

    RimSummaryCase*                 summaryCase() const;
    RimSummaryEnsemble*             ensemble() const;
    RifEclipseSummaryAddress        summaryAddress() const;
    RiaDefines::DateTimePeriodEnum  resamplingPeriod() const;
    RimPlotAxisPropertiesInterface* plotAxisProperties() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    bool isEnsemble() const;

private:
    caf::PdmPtrField<RimSummaryCase*>                 m_summaryCase;
    caf::PdmPtrField<RimSummaryEnsemble*>             m_summaryCaseCollection;
    caf::PdmChildField<RimSummaryAddress*>            m_summaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>           m_summaryAddressUiField;
    caf::PdmField<bool>                               m_pushButtonSelectSummaryAddress;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_plotAxisProperties;
    caf::PdmField<RiaDefines::DateTimePeriodEnum>     m_resamplingPeriod;

    bool m_showDataSource;
    bool m_showResampling;

    RimPlotAxisProperties::Orientation m_plotAxisOrientation;
};
