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

#include "RimObjectiveFunction.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressQMetaType.h"
#include "RifSummaryReaderInterface.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QString>

class RimEnsembleCurveSet;
class RimSummaryCase;
class RimSummaryAddress;
class RimCustomObjectiveFunction;

//==================================================================================================
///
//==================================================================================================
class RimCustomObjectiveFunctionWeight : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCustomObjectiveFunctionWeight();

    QString                         title() const;
    void                            setSummaryAddress( RifEclipseSummaryAddress address );
    RifEclipseSummaryAddress        summaryAddress() const;
    ObjectiveFunction::FunctionType objectiveFunction() const;
    double                          weightValue() const;

    RimEnsembleCurveSet* parentCurveSet() const;

private:
    caf::PdmFieldHandle* userDescriptionField() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    RimCustomObjectiveFunction* parentObjectiveFunction() const;

private:
    caf::PdmProxyValueField<QString>                             m_title;
    caf::PdmChildField<RimSummaryAddress*>                       m_objectiveValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>                      m_objectiveValuesSummaryAddressUiField;
    caf::PdmField<bool>                                          m_objectiveValuesSelectSummaryAddressPushButton;
    caf::PdmField<caf::AppEnum<ObjectiveFunction::FunctionType>> m_objectiveFunction;
    caf::PdmField<double>                                        m_weightValue;
};
