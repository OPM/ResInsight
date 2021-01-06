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

#include "opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp"
#include "opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp"

class RiuQwtPlotWidget;
class VfpPlotData;

//--------------------------------------------------------------------------------------------------
/// Vertical Flow Performance Plot
//--------------------------------------------------------------------------------------------------
class RimVfpPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpPlot();
    ~RimVfpPlot() override;

    // RimPlot implementations
    RiuQwtPlotWidget* viewer() override;
    void              setAutoScaleXEnabled( bool enabled ) override;
    void              setAutoScaleYEnabled( bool enabled ) override;
    void              updateAxes() override;
    void              updateLegend() override;
    void              updateZoomInQwt() override;
    void              updateZoomFromQwt() override;
    QString           asciiDataForPlotExport() const override;
    void              reattachAllCurves() override;
    void              detachAllCurves() override;
    caf::PdmObject*   findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;
    void              onAxisSelected( int axis, bool toggle ) override;

    // RimPlotWindow implementations
    QString description() const override;

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

private:
    // RimPlot implementations
    void doRemoveFromCollection();

    // RimViewWindow implementations
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

    // PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    void                populatePlotWidgetWithCurveData( RiuQwtPlotWidget* plotWidget, const Opm::VFPInjTable& table );
    void                populatePlotWidgetWithCurveData( RiuQwtPlotWidget*                     plotWidget,
                                                         const Opm::VFPProdTable&              table,
                                                         RimVfpDefines::ProductionVariableType primaryVariable,
                                                         RimVfpDefines::ProductionVariableType familyVariable );
    std::vector<double> getProductionTableData( const Opm::VFPProdTable&              table,
                                                RimVfpDefines::ProductionVariableType variableType ) const;
    size_t              getVariableIndex( const Opm::VFPProdTable&              table,
                                          RimVfpDefines::ProductionVariableType targetVariable,
                                          RimVfpDefines::ProductionVariableType primaryVariable,
                                          size_t                                primaryValue,
                                          RimVfpDefines::ProductionVariableType familyVariable,
                                          size_t                                familyValue ) const;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType,
                                     QList<caf::PdmOptionItemInfo>&        options );

    void setFixedVariableUiEditability( caf::PdmField<int>& field, RimVfpDefines::ProductionVariableType variableType );

    void           updatePlotTitle( const QString& plotTitle );
    static QString generatePlotTitle( const QString&                          wellName,
                                      int                                     tableNumber,
                                      RimVfpDefines::TableType                tableType,
                                      RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                      RimVfpDefines::ProductionVariableType   primaryVariable,
                                      RimVfpDefines::ProductionVariableType   familyVariable );

    static QwtPlotCurve* createPlotCurve( const QString title, const QColor& color );
    static double        convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType );
    static void convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType );

    static QString getDisplayUnit( RimVfpDefines::ProductionVariableType variableType );

    static QString getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType );

    static RimVfpDefines::FlowingPhaseType         getFlowingPhaseType( const Opm::VFPProdTable& table );
    static RimVfpDefines::FlowingPhaseType         getFlowingPhaseType( const Opm::VFPInjTable& table );
    static RimVfpDefines::FlowingWaterFractionType getFlowingWaterFractionType( const Opm::VFPProdTable& table );
    static RimVfpDefines::FlowingGasFractionType   getFlowingGasFractionType( const Opm::VFPProdTable& table );

    void populatePlotData( const Opm::VFPProdTable&                table,
                           RimVfpDefines::ProductionVariableType   primaryVariable,
                           RimVfpDefines::ProductionVariableType   familyVariable,
                           RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                           VfpPlotData&                            plotData ) const;

    void populatePlotData( const Opm::VFPInjTable&                 table,
                           RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                           VfpPlotData&                            plotData ) const;

    void populatePlotWidgetWithPlotData( RiuQwtPlotWidget* plotWidget, const VfpPlotData& plotData );

private:
    caf::PdmField<QString>                                               m_plotTitle;
    caf::PdmField<caf::FilePath>                                         m_filePath;
    caf::PdmField<int>                                                   m_tableNumber;
    caf::PdmField<double>                                                m_referenceDepth;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingPhaseType>>         m_flowingPhase;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingWaterFractionType>> m_flowingWaterFraction;
    caf::PdmField<caf::AppEnum<RimVfpDefines::FlowingGasFractionType>>   m_flowingGasFraction;

    caf::PdmField<caf::AppEnum<RimVfpDefines::TableType>>                m_tableType;
    caf::PdmField<caf::AppEnum<RimVfpDefines::InterpolatedVariableType>> m_interpolatedVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_primaryVariable;
    caf::PdmField<caf::AppEnum<RimVfpDefines::ProductionVariableType>>   m_familyVariable;

    caf::PdmField<int> m_liquidFlowRateIdx;
    caf::PdmField<int> m_thpIdx;
    caf::PdmField<int> m_articifialLiftQuantityIdx;
    caf::PdmField<int> m_waterCutIdx;
    caf::PdmField<int> m_gasLiquidRatioIdx;

    QPointer<RiuQwtPlotWidget>         m_plotWidget;
    std::unique_ptr<Opm::VFPProdTable> m_prodTable;
    std::unique_ptr<Opm::VFPInjTable>  m_injectionTable;
};
