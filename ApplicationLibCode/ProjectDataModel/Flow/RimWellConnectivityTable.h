/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimPlotWindow.h"

#include "RigFlowDiagResultAddress.h"

#include "RimRegularLegendConfig.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QPointer>
#include <QString>

class RimEclipseResultCase;
class RimEclipseView;
class RimFlowDiagSolution;
class RimRegularLegendConfig;
class RimSimWellInView;
class RigWellAllocationOverTime;
class RiuMatrixPlotWidget;
class RigSimWellData;
class RigAccWellFlowCalculator;

//==================================================================================================
///
//==================================================================================================
class RimWellConnectivityTable : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ViewFilterType
    {
        CALCULATE_BY_VISIBLE_CELLS,
        FILTER_BY_VISIBLE_PRODUCERS,
        FILTER_BY_VISIBLE_INJECTORS,
    };

    enum class TimeStepSelection
    {
        SINGLE_TIME_STEP,
        TIME_STEP_RANGE,
    };

    enum class TimeSampleValueType
    {
        FLOW_RATE,
        FLOW_RATE_FRACTION,
        FLOW_RATE_PERCENTAGE,
    };
    enum class TimeRangeValueType
    {
        ACCUMULATED_FLOW_VOLUME,
        ACCUMULATED_FLOW_VOLUME_FRACTION,
        ACCUMULATED_FLOW_VOLUME_PERCENTAGE,
    };

    enum class TimeStepRangeFilterMode
    {
        NONE,
        TIME_STEP_COUNT,
    };

    enum class RangeType
    {
        AUTOMATIC,
        USER_DEFINED
    };

public:
    RimWellConnectivityTable();
    ~RimWellConnectivityTable() override;

    void setFromSimulationWell( RimSimWellInView* simWell );

private:
    void cleanupBeforeClose();

    void onLoadDataAndUpdate() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // Inherited via RimPlotWindow
    QString description() const override;
    void    doRenderWindowContent( QPaintDevice* paintDevice ) override;

    // Inherited via RimViewWindow
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    int axisTitleFontSize() const;
    int axisLabelFontSize() const;
    int valueLabelFontSize() const;

private:
    std::map<QString, RigWellAllocationOverTime>
                              createProductionWellsAllocationOverTimeMap( const std::set<QString>& selectedProductionWells ) const;
    RigWellAllocationOverTime createWellAllocationOverTime( const RigSimWellData* simWellData ) const;

    void createAndEmplaceTimeStepAndCalculatorPairInMap( std::map<QDateTime, RigAccWellFlowCalculator>& rTimeStepAndCalculatorPairs,
                                                         const QDateTime                                timeStep,
                                                         int                                            timeStepIndex,
                                                         const RigSimWellData*                          simWellData ) const;

    std::set<QDateTime>  getSelectedTimeSteps( const std::vector<QDateTime>& timeSteps ) const;
    QString              dateFormatString() const;
    std::vector<QString> getProductionWellNames() const;
    std::vector<QString> getProductionWellNamesAtTimeSteps( const std::set<QDateTime>& timeSteps ) const;

    QString createTableTitle() const;

    std::pair<double, double> createLegendMinMaxValues( const double maxTableValue ) const;

    void setValidTimeStepSelectionsForCase();
    void setValidSingleTimeStepForCase();
    void setValidTimeStepRangeForCase();
    bool isTimeStepInCase( const QDateTime& timeStep ) const;

    int  getTimeStepIndex( const QDateTime timeStep, const std::vector<QDateTime> timeSteps ) const;
    void setSelectedProducersAndInjectorsForSingleTimeStep();
    void setSelectedProducersAndInjectorsForTimeStepRange();

    void                 syncSelectedInjectorsFromProducerSelection();
    void                 syncSelectedProducersFromInjectorSelection();
    void                 setProducerSelectionFromViewFilterAndSynchInjectors();
    void                 setInjectorSelectionFromViewFilterAndSynchProducers();
    void                 setWellSelectionFromViewFilter();
    std::vector<QString> getViewFilteredWellNamesFromFilterType( ViewFilterType filterType ) const;

    void onCellVisibilityChanged( const SignalEmitter* emitter );
    void connectViewCellVisibilityChangedToSlot( RimEclipseView* view );
    void disconnectViewCellVisibilityChangedFromSlots( RimEclipseView* view );

private:
    // Matrix plot for visualizing table data
    QPointer<RiuMatrixPlotWidget> m_matrixPlotWidget;

    caf::PdmPtrField<RimEclipseResultCase*>     m_case;
    caf::PdmPtrField<RimEclipseView*>           m_cellFilterView;
    caf::PdmField<caf::AppEnum<ViewFilterType>> m_viewFilterType;
    caf::PdmPtrField<RimFlowDiagSolution*>      m_flowDiagSolution;

    caf::PdmField<caf::AppEnum<TimeStepSelection>>   m_timeStepSelection;
    caf::PdmField<caf::AppEnum<TimeSampleValueType>> m_timeSampleValueType;
    caf::PdmField<caf::AppEnum<TimeRangeValueType>>  m_timeRangeValueType;

    caf::PdmField<bool> m_selectProducersAndInjectorsForTimeSteps;

    caf::PdmField<double> m_thresholdValue;

    // For single time sample
    caf::PdmField<QDateTime> m_selectedTimeStep;

    // For time step range
    caf::PdmField<QDateTime>                             m_selectedFromTimeStep;
    caf::PdmField<QDateTime>                             m_selectedToTimeStep;
    caf::PdmField<caf::AppEnum<TimeStepRangeFilterMode>> m_timeStepFilterMode;
    caf::PdmField<int>                                   m_timeStepCount;
    caf::PdmField<std::vector<QDateTime>>                m_excludeTimeSteps;
    caf::PdmField<bool>                                  m_applyTimeStepSelections;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    caf::PdmField<std::vector<QString>> m_selectedInjectorTracersUiField;
    caf::PdmField<std::vector<QString>> m_selectedProducerTracersUiField;
    caf::PdmField<bool>                 m_syncSelectedProducersFromInjectorSelection;
    caf::PdmField<bool>                 m_syncSelectedInjectorsFromProducerSelection;
    caf::PdmField<bool>                 m_applySelectedInectorProducerTracers;

    caf::PdmField<RimRegularLegendConfig::MappingEnum> m_mappingType;
    caf::PdmField<caf::AppEnum<RangeType>>             m_rangeType;

    RimFontSizeField    m_axisTitleFontSize;
    RimFontSizeField    m_axisLabelFontSize;
    RimFontSizeField    m_valueLabelFontSize;
    caf::PdmField<bool> m_showValueLabels;

    const int m_initialNumberOfTimeSteps = 10;
};
