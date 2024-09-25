/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RiuPlotWidget;
class VfpPlotData;
class RimPlotAxisProperties;
class RigVfpTables;
class RimVfpTable;
class RimVfpTableData;

struct VfpTableSelection;
struct VfpTableInitialData;

namespace Opm
{
class VFPInjTable;
class VFPProdTable;
} // namespace Opm

//--------------------------------------------------------------------------------------------------
/// DEPRECATED: Vertical Flow Performance Plot
/// This class is deprecated and will be removed in a future release.
/// Use RimCustomVfpPlot instead.
//--------------------------------------------------------------------------------------------------
class RimVfpPlot_deprecated : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpPlot_deprecated();
    ~RimVfpPlot_deprecated() override;

private:
    void setDataSource( RimVfpTable* vfpTableData );
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

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

    void setDataIsImportedExternally( bool dataIsImportedExternally );
    int  tableNumber() const;

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

    VfpTableSelection tableSelection() const;
    void              initializeFromInitData( const VfpTableInitialData& table );

    RimVfpTableData*    vfpTableData() const;
    const RigVfpTables* vfpTables() const;

    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    void calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType, QList<caf::PdmOptionItemInfo>& options );

    void setFixedVariableUiEditability( caf::PdmField<int>& field, RimVfpDefines::ProductionVariableType variableType );

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

    void populatePlotWidgetWithPlotData( RiuPlotWidget* plotWidget, const VfpPlotData& plotData );

    static QString axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase );

    void connectAxisSignals( RimPlotAxisProperties* axis );
    void axisSettingsChanged( const caf::SignalEmitter* emitter );
    void axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic );
    void updatePlotWidgetFromAxisRanges() override;
    void updateAxisRangesFromPlotWidget() override;

    void onPlotZoomed();
    void curveAppearanceChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<QString>                                               m_plotTitle;
    caf::PdmPtrField<RimVfpTable*>                                       m_vfpTable;
    caf::PdmField<int>                                                   m_tableNumber;
    caf::PdmField<double>                                                m_referenceDepth;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingPhaseType>>         m_flowingPhase;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingWaterFractionType>> m_flowingWaterFraction;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingGasFractionType>>   m_flowingGasFraction;

    caf::PdmField<caf::AppEnum<RimVfpDefines::TableType>>                m_tableType;
    caf::PdmField<caf::AppEnum<RimVfpDefines::InterpolatedVariableType>> m_interpolatedVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_primaryVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_familyVariable;

    caf::PdmField<int> m_flowRateIdx;
    caf::PdmField<int> m_thpIdx;
    caf::PdmField<int> m_articifialLiftQuantityIdx;
    caf::PdmField<int> m_waterCutIdx;
    caf::PdmField<int> m_gasLiquidRatioIdx;

    caf::PdmChildField<RimPlotAxisProperties*> m_yAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_xAxisProperties;

    caf::PdmChildArrayField<RimPlotCurve*> m_plotCurves;

    QPointer<RiuPlotWidget> m_plotWidget;

    caf::PdmField<caf::FilePath> m_filePath_OBSOLETE;

    bool m_dataIsImportedExternally;
};
