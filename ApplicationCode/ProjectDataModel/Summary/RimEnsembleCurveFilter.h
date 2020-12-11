/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimObjectiveFunction.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class EnsembleParameter;
class RimEnsembleCurveSet;
class RimSummaryCase;
class RimSummaryAddress;
class RimSummaryPlot;
class RimEnsembleCurveFilterCollection;
class RimCustomObjectiveFunction;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class FilterMode
    {
        BY_ENSEMBLE_PARAMETER = 0,
        BY_OBJECTIVE_FUNCTION,
        BY_CUSTOM_OBJECTIVE_FUNCTION
    };

    RimEnsembleCurveFilter();
    RimEnsembleCurveFilter( const QString& ensembleParameterName );

    bool                                  isActive() const;
    double                                minValue() const;
    double                                maxValue() const;
    std::set<QString>                     categories() const;
    QString                               ensembleParameterName() const;
    QString                               filterId() const;
    QString                               description() const;
    std::vector<RifEclipseSummaryAddress> summaryAddresses() const;
    void                                  setSummaryAddresses( std::vector<RifEclipseSummaryAddress> addresses );

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          updateAddressesUiField();
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle*          userDescriptionField() override;
    void                          updateIcon();
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    std::vector<RimSummaryCase*> applyFilter( const std::vector<RimSummaryCase*>& allSumCases );

    void              loadDataAndUpdate();
    EnsembleParameter selectedEnsembleParameter() const;

    RimEnsembleCurveSet* parentCurveSet() const;

protected:
    caf::PdmFieldHandle* objectToggleField() override;

private:
    RimEnsembleCurveFilterCollection* parentCurveFilterCollection() const;
    void                              updateMaxMinAndDefaultValues( bool forceDefault );

private:
    caf::PdmProxyValueField<QString>                                m_filterTitle;
    caf::PdmField<bool>                                             m_active;
    caf::PdmField<caf::AppEnum<FilterMode>>                         m_filterMode;
    caf::PdmField<QString>                                          m_ensembleParameterName;
    caf::PdmChildArrayField<RimSummaryAddress*>                     m_objectiveValuesSummaryAddresses;
    caf::PdmField<QString>                                          m_objectiveValuesSummaryAddressesUiField;
    caf::PdmField<bool>                                             m_objectiveValuesSelectSummaryAddressPushButton;
    caf::PdmField<caf::AppEnum<RimObjectiveFunction::FunctionType>> m_objectiveFunction;
    caf::PdmPtrField<RimCustomObjectiveFunction*>                   m_customObjectiveFunction;
    caf::PdmField<double>                                           m_minValue;
    caf::PdmField<double>                                           m_maxValue;
    caf::PdmField<std::vector<QString>>                             m_categories;

    double m_lowerLimit;
    double m_upperLimit;
};
