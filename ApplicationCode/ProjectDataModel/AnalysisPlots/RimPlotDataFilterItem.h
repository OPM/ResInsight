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

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimSummaryCaseCollection.h"

#include "RiaSummaryCurveDefinition.h"

#include <QDateTime>

class RiuSummaryQwtPlot;
class RiuGroupedBarChartBuilder;

class RimAnalysisPlotDataEntry;
class RiaSummaryCurveDefinitionAnalyser;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;

class RimSummaryAddress;

// Filter of type :
// Only [Cases/SummaryItem/Case by ensemble param] where the [Quantity]
// is [within range/top N/min N]
// considering the [Plot Source/Last/First/Last with History/all] timestep(s) [range/1,2..]

// Use only the "Summary Cases" where "FOPT" at the "plot source" time step(s) is "within range"
// Use only the "Summary Items" where "WOPT" at the "Last" time step(s) is "Top " "5"
// Use only the "Ensemble Cases" where "LGOR_FOR_EDKNRE" is "within range" "5"

class RimPlotDataFilterItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> filterChanged;

public:
    RimPlotDataFilterItem();
    ~RimPlotDataFilterItem() override;

    enum TimeStepSourceType
    {
        PLOT_SOURCE_TIMESTEPS,
        LAST_TIMESTEP,
        FIRST_TIMESTEP,
        LAST_TIMESTEP_WITH_HISTORY,
        ALL_TIMESTEPS,
        SELECT_TIMESTEPS,
        SELECT_TIMESTEP_RANGE
    };

    // Filter target
    enum FilterTarget
    {
        SUMMARY_ITEM,
        SUMMARY_CASE,
        ENSEMBLE_CASE
    };

    enum FilterOperation
    {
        TOP_N,
        BOTTOM_N,
        RANGE
    };

    bool         isActive() const { return m_isActive(); }
    FilterTarget filterTarget() const { return m_filterTarget(); }

    RifEclipseSummaryAddress summaryAddress() const;

    QString ensembleParameterName() const;

    FilterOperation           filterOperation() const { return m_filterOperation(); }
    std::pair<double, double> filterRangeMinMax() const;
    int                       topBottomN() const;

    std::vector<QString> selectedEnsembleParameterCategories() const;

    TimeStepSourceType        consideredTimeStepsType() const;
    std::pair<time_t, time_t> timeRangeMinMax() const;
    std::vector<time_t>       explicitlySelectedTimeSteps() const;
    void                      updateMaxMinAndDefaultValues( bool forceDefault );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle*          objectToggleField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    EnsembleParameter selectedEnsembleParameter() const;

    caf::PdmField<bool> m_isActive;

    caf::PdmField<caf::AppEnum<FilterTarget>> m_filterTarget;

    // Quantity

    // Complete address or quantity name only
    caf::PdmChildField<RimSummaryAddress*>  m_filterAddress;
    caf::PdmField<QString>                  m_filterEnsembleParameter;
    caf::PdmField<RifEclipseSummaryAddress> m_filterQuantityUiField;
    caf::PdmField<bool>                     m_filterQuantitySelectButton;

    // Operation and parameters

    caf::PdmField<caf::AppEnum<FilterOperation>> m_filterOperation;
    caf::PdmField<int>                           m_topBottomN;
    caf::PdmField<double>                        m_max;
    caf::PdmField<double>                        m_min;

    caf::PdmField<std::vector<QString>> m_ensembleParameterValueCategories;

    // Considered Timesteps

    caf::PdmField<caf::AppEnum<TimeStepSourceType>> m_consideredTimestepsType;
    caf::PdmField<std::vector<QDateTime>>           m_explicitlySelectedTimeSteps;

    double m_lowerLimit;
    double m_upperLimit;
};
