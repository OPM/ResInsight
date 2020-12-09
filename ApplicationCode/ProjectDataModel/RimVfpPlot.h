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

#include "cafPdmPtrField.h"

#include <QPointer>

#include "opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp"
#include "opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp"

class RimEclipseResultCase;
class RimFlowDiagSolution;
class RigTofWellDistributionCalculator;
class RiuQwtPlotWidget;

//--------------------------------------------------------------------------------------------------
/// Vertical Flow Performance Plot
//--------------------------------------------------------------------------------------------------
class RimVfpPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum class TableType
    {
        INJECTION,
        PRODUCTION
    };

    enum class ProductionVariableType
    {
        LIQUID_FLOW_RATE,
        THP,
        ARTIFICIAL_LIFT_QUANTITY,
        WATER_CUT,
        GAS_LIQUID_RATIO
    };

    RimVfpPlot();
    ~RimVfpPlot() override;

    void setDataSourceParameters( RimEclipseResultCase* eclipseResultCase, QString targetWellName );
    // void setPlotOptions( bool groupSmallContributions, double smallContributionsRelativeThreshold, double maximumTof );

    RiaDefines::PhaseType phase() const;

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

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    void fixupDependentFieldsAfterCaseChange();
    void populatePlotWidgetWithCurveData( RiuQwtPlotWidget* plotWidget, const std::vector<Opm::VFPInjTable>& tables );
    void populatePlotWidgetWithCurveData( RiuQwtPlotWidget*                  plotWidget,
                                          const Opm::VFPProdTable&           table,
                                          RimVfpPlot::ProductionVariableType primaryVariable,
                                          RimVfpPlot::ProductionVariableType familyVariable );
    std::vector<double> getProductionTableData( const Opm::VFPProdTable&           table,
                                                RimVfpPlot::ProductionVariableType variableType ) const;
    size_t              getVariableIndex( const Opm::VFPProdTable&           table,
                                          RimVfpPlot::ProductionVariableType targetVariable,
                                          RimVfpPlot::ProductionVariableType primaryVariable,
                                          size_t                             primaryValue,
                                          RimVfpPlot::ProductionVariableType familyVariable,
                                          size_t                             familyValue ) const;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void calculateTableValueOptions( RimVfpPlot::ProductionVariableType variableType,
                                     QList<caf::PdmOptionItemInfo>&     options );

private:
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmField<QString>                  m_wellName;

    caf::PdmField<caf::AppEnum<RimVfpPlot::TableType>>              m_tableType;
    caf::PdmField<caf::AppEnum<RimVfpPlot::ProductionVariableType>> m_primaryVariable;
    caf::PdmField<caf::AppEnum<RimVfpPlot::ProductionVariableType>> m_familyVariable;

    caf::PdmField<int> m_liquidFlowRateIdx;
    caf::PdmField<int> m_thpIdx;
    caf::PdmField<int> m_articifialLiftQuantityIdx;
    caf::PdmField<int> m_waterCutIdx;
    caf::PdmField<int> m_gasLiquidRatioIdx;

    QPointer<RiuQwtPlotWidget>         m_plotWidget;
    std::unique_ptr<Opm::VFPProdTable> m_prodTable;
};
