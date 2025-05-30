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

#include "RimPlot.h"
#include "RimPlotDataFilterItem.h"
#include "RimTimeStepFilter.h"

#include "RigEnsembleParameter.h"

#include "cafPdmPtrField.h"

#include <QDateTime>

class RiuSummaryQwtPlot;
class RiuGroupedBarChartBuilder;

class RimAnalysisPlotDataEntry;
class RiaSummaryCurveDefinitionAnalyser;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RimPlotDataFilterCollection;
class RiaSummaryCurveDefinition;
class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimAnalysisPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum class BarOrientation
    {
        BARS_HORIZONTAL,
        BARS_VERTICAL
    };

    enum class SortGroupType
    {
        NONE,
        SUMMARY_ITEM,
        VECTOR,
        CASE,
        ENSEMBLE,
        VALUE,
        ABS_VALUE,
        OTHER_VALUE,
        ABS_OTHER_VALUE,
        TIME_STEP,
    };
    using SortGroupAppEnum = caf::AppEnum<SortGroupType>;

    using TimeStepFilterEnum = caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>;

public:
    RimAnalysisPlot();
    ~RimAnalysisPlot() override;

    void updateCaseNameHasChanged();

    RimPlotDataFilterCollection* plotDataFilterCollection() const;

    void setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    void setTimeSteps( const std::vector<time_t>& timeSteps );

    std::set<RifEclipseSummaryAddress> unfilteredAddresses() const;
    std::set<RigEnsembleParameter>     ensembleParameters() const;
    RigEnsembleParameter               ensembleParameter( const QString& ensembleParameterName ) const;

    void maxMinValueFromAddress( const RifEclipseSummaryAddress&           address,
                                 RimPlotDataFilterItem::TimeStepSourceType timeStepSourceType,
                                 const std::vector<QDateTime>&             timeRangeOrSelection,
                                 bool                                      useAbsValue,
                                 double*                                   min,
                                 double*                                   max ) const;

    std::vector<time_t> selectedTimeSteps() const;
    QString             description() const override;
    QString             asciiDataForPlotExport() const override;

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    std::set<time_t> allAvailableTimeSteps() const;

    std::set<RimSummaryCase*> timestepDefiningSourceCases() const;
    std::set<RimSummaryCase*> allSourceCases() const;

    void onFiltersChanged( const caf::SignalEmitter* emitter );

    // RimViewWindow overrides

    QWidget* viewWidget() override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     zoomAll() override {}
    QImage   snapshotWindowContent() override;

    // RimPlotWindow overrides

    void doUpdateLayout() override {}

    // RimPlot Overrides

    RiuPlotWidget*    doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;
    RiuQwtPlotWidget* viewer();
    RiuPlotWidget*    plotWidget() override;

    void detachAllCurves() override;

    void reattachAllCurves() override {}
    void updateAxes() override;
    void onAxisSelected( RiuPlotAxis axis, bool toggle ) override;
    void setAutoScaleXEnabled( bool enabled ) override {}
    void setAutoScaleYEnabled( bool enabled ) override {}
    void updateLegend() override {};

    // Private methods

    void cleanupBeforeClose();
    void addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder ) const;
    void updatePlotTitle();

    QString assignGroupingText( RimAnalysisPlot::SortGroupType  sortGroup,
                                const RiaSummaryCurveDefinition dataEntry,
                                const QString&                  timestepString ) const;

    RiaSummaryCurveDefinitionAnalyser*     updateAndGetCurveAnalyzer() const;
    std::vector<RiaSummaryCurveDefinition> curveDefinitions() const;
    std::vector<RiaSummaryCurveDefinition> filteredCurveDefs() const;
    void                                   applyFilter( const RimPlotDataFilterItem*        filter,
                                                        std::set<RimSummaryCase*>*          filteredSumCases,
                                                        std::set<RifEclipseSummaryAddress>* filteredSummaryItems ) const;

    static std::vector<size_t> findTimestepIndices( std::vector<time_t> selectedTimesteps, const std::vector<time_t>& timesteps );

    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

    void connectAxisSignals( RimPlotAxisProperties* axis );
    void axisSettingsChanged( const caf::SignalEmitter* emitter );
    void axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic );

    void buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder ) const;

    int  barTextFontSize() const;
    void initAfterRead() override;

private:
    void onCaseRemoved( const SignalEmitter* emitter, RimSummaryCase* summaryCase );
    void connectAllCaseSignals();

private:
    std::unique_ptr<RiaSummaryCurveDefinitionAnalyser> m_analyserOfSelectedCurveDefs;

    QPointer<RiuQwtPlotWidget> m_plotWidget;

    // Fields

    caf::PdmField<QString> m_selectedVarsUiField;
    caf::PdmField<bool>    m_selectVariablesButtonField;

    caf::PdmChildArrayField<RimAnalysisPlotDataEntry*> m_analysisPlotDataSelection;

    caf::PdmField<TimeStepFilterEnum>     m_timeStepFilter;
    caf::PdmField<std::vector<QDateTime>> m_selectedTimeSteps;

    caf::PdmPtrField<RimSummaryCase*> m_referenceCase;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    caf::PdmField<caf::AppEnum<BarOrientation>> m_barOrientation;

    caf::PdmField<SortGroupAppEnum> m_majorGroupType;
    caf::PdmField<SortGroupAppEnum> m_mediumGroupType;
    caf::PdmField<SortGroupAppEnum> m_minorGroupType;
    caf::PdmField<SortGroupAppEnum> m_valueSortOperation;

    caf::PdmField<bool> m_useTopBarsFilter;
    caf::PdmField<int>  m_maxBarCount;

    caf::PdmField<SortGroupAppEnum> m_sortGroupForColors;

    caf::PdmField<bool>                             m_useBarText;
    caf::PdmField<bool>                             m_useCaseInBarText;
    caf::PdmField<bool>                             m_useEnsembleInBarText;
    caf::PdmField<bool>                             m_useSummaryItemInBarText;
    caf::PdmField<bool>                             m_useTimeStepInBarText;
    caf::PdmField<bool>                             m_useVectorNameInBarText;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_barTextFontSize;

    caf::PdmChildField<RimPlotAxisProperties*>       m_valueAxisProperties;
    caf::PdmChildField<RimPlotDataFilterCollection*> m_plotDataFilterCollection;
};
