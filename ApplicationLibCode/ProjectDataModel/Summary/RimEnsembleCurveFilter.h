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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RifEclipseSummaryAddress;
class RigEnsembleParameter;
class RimEnsembleCurveSet;
class RimSummaryCase;
class RimSummaryAddress;
class RimSummaryPlot;
class RimEnsembleCurveFilterCollection;
class RimCustomObjectiveFunction;
class RimSummaryAddressSelector;
class RimObjectiveFunction;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class FilterMode
    {
        ENSEMBLE_PARAMETER,
        OBJECTIVE_FUNCTION,
        CUSTOM_OBJECTIVE_FUNCTION,
        SUMMARY_VALUE
    };

    RimEnsembleCurveFilter();
    RimEnsembleCurveFilter( const QString& ensembleParameterName );

    bool                                  isActive() const;
    void                                  setActive( bool active );
    double                                minValue() const;
    double                                maxValue() const;
    std::set<QString>                     categories() const;
    QString                               ensembleParameterName() const;
    QString                               filterId() const;
    QString                               description() const;
    std::vector<RifEclipseSummaryAddress> summaryAddresses() const;
    void                                  setSummaryAddresses( std::vector<RifEclipseSummaryAddress> addresses );
    FilterMode                            filterMode() const;
    RigEnsembleParameter                  selectedEnsembleParameter() const;

    std::vector<RimSummaryCase*> applyFilter( const std::vector<RimSummaryCase*>& allSumCases );

    RimEnsembleCurveSet* parentCurveSet() const;

    void updateMaxMinAndDefaultValuesFromParent();

    void loadDataAndUpdate();
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void                 appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void                 initAfterRead() override;

    void onObjectionFunctionChanged( const caf::SignalEmitter* emitter );

    RimEnsembleCurveFilterCollection* parentCurveFilterCollection() const;
    void                              updateMaxMinAndDefaultValues( bool forceDefault );

    void updateAddressesUiField();
    void updateIcon();
    void updateCurves();

private:
    caf::PdmProxyValueField<QString>        m_filterTitle;
    caf::PdmField<bool>                     m_active;
    caf::PdmField<caf::AppEnum<FilterMode>> m_filterMode;
    caf::PdmField<QString>                  m_ensembleParameterName;

    caf::PdmChildArrayField<RimSummaryAddress*>   m_objectiveValuesSummaryAddresses;
    caf::PdmField<QString>                        m_objectiveValuesSummaryAddressesUiField;
    caf::PdmField<bool>                           m_objectiveValuesSelectSummaryAddressPushButton;
    caf::PdmChildField<RimObjectiveFunction*>     m_objectiveFunction;
    caf::PdmPtrField<RimCustomObjectiveFunction*> m_customObjectiveFunction;

    caf::PdmChildField<RimSummaryAddressSelector*> m_addressSelector;

    caf::PdmField<std::pair<double, double>> m_valueRange;
    caf::PdmField<double>                    m_minValue_OBSOLETE;
    caf::PdmField<double>                    m_maxValue_OBSOLETE;

    caf::PdmField<std::vector<QString>> m_categories;

    caf::PdmField<QString> m_realizationFilter;

    double m_lowerLimit;
    double m_upperLimit;
};
