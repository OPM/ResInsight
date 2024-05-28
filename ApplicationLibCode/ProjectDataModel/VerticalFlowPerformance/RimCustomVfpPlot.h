/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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
#include "RimVfpDefines.h"

#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RiuPlotWidget;
class VfpPlotData;
class RimPlotAxisProperties;
class RigVfpTables;
class RimVfpTableData;
class RimVfpTable;
class RiuPlotCurveInfoTextProvider;

struct VfpValueSelection;
struct VfpTableInitialData;

namespace Opm
{
class VFPInjTable;
class VFPProdTable;
} // namespace Opm

//--------------------------------------------------------------------------------------------------
/// Vertical Flow Performance Plot
//--------------------------------------------------------------------------------------------------
class RimCustomVfpPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimCustomVfpPlot();
    ~RimCustomVfpPlot() override;

    void selectDataSource( RimVfpTable* mainDataSource, const std::vector<RimVfpTable*>& vfpTableData );
    void setTableNumber( int tableNumber );
    void initializeObject();

    // RimPlot implementations
    RiuPlotWidget* plotWidget() override;
    bool           isCurveHighlightSupported() const override;

    void    setAutoScaleXEnabled( bool enabled ) override;
    void    setAutoScaleYEnabled( bool enabled ) override;
    void    updateAxes() override;
    void    updateLegend() override;
    QString asciiDataForPlotExport() const override;
    void    reattachAllCurves() override;
    void    detachAllCurves() override;

    // RimPlotWindow implementations
    QString description() const override;
    QString infoForCurve( RimPlotCurve* plotCurve ) const;

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

private:
    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void scheduleReplot();

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    VfpValueSelection tableSelection( RimVfpTable* table ) const;
    void              initializeFromInitData( const VfpTableInitialData& table );

    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    void calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType, QList<caf::PdmOptionItemInfo>& options );

    void setFixedVariableUiEditability( caf::PdmFieldHandle& field, RimVfpDefines::ProductionVariableType variableType );

    void           updatePlotTitle( const QString& plotTitle );
    static QString generatePlotTitle( const QString&                          wellName,
                                      int                                     tableNumber,
                                      RimVfpDefines::TableType                tableType,
                                      RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                      RimVfpDefines::ProductionVariableType   primaryVariable,
                                      RimVfpDefines::ProductionVariableType   familyVariable );

    static double  convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType );
    static void    convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType );
    static QString getDisplayUnit( RimVfpDefines::ProductionVariableType variableType );
    static QString getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType );

    void populatePlotWidgetWithPlotData( RiuPlotWidget* plotWidget, const VfpPlotData& plotData, const QColor& color );

    static QString axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase );

    void connectAxisSignals( RimPlotAxisProperties* axis );
    void axisSettingsChanged( const caf::SignalEmitter* emitter );
    void axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic );
    void updatePlotWidgetFromAxisRanges() override;
    void updateAxisRangesFromPlotWidget() override;

    void onPlotZoomed();
    void curveAppearanceChanged( const caf::SignalEmitter* emitter );

    std::vector<double> availableValues( RimVfpDefines::ProductionVariableType variableType ) const;

    static RiuPlotCurveInfoTextProvider* curveTextProvider();

private:
    caf::PdmField<QString> m_plotTitle;

    caf::PdmPtrField<RimVfpTable*>      m_mainDataSource;
    caf::PdmPtrArrayField<RimVfpTable*> m_additionalDataSources;

    caf::PdmField<caf::AppEnum<RimVfpDefines::CurveMatchingType>>     m_curveMatchingType;
    caf::PdmField<caf::AppEnum<RimVfpDefines::CurveOptionValuesType>> m_curveOptionFiltering;

    caf::PdmField<int>                                                   m_tableNumber;
    caf::PdmField<double>                                                m_referenceDepth;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingPhaseType>>         m_flowingPhase;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingWaterFractionType>> m_flowingWaterFraction;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingGasFractionType>>   m_flowingGasFraction;

    caf::PdmField<caf::AppEnum<RimVfpDefines::TableType>>                m_tableType;
    caf::PdmField<caf::AppEnum<RimVfpDefines::InterpolatedVariableType>> m_interpolatedVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_primaryVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_familyVariable;

    caf::PdmField<double> m_flowRateIdx;
    caf::PdmField<double> m_thpIdx;
    caf::PdmField<double> m_articifialLiftQuantityIdx;
    caf::PdmField<double> m_waterCutIdx;
    caf::PdmField<double> m_gasLiquidRatioIdx;

    caf::PdmField<std::vector<double>> m_familyValues;

    caf::PdmChildField<RimPlotAxisProperties*> m_yAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_xAxisProperties;

    caf::PdmChildArrayField<RimPlotCurve*> m_plotCurves;

    std::vector<VfpPlotData> m_plotData;

    QPointer<RiuPlotWidget> m_plotWidget;
};
