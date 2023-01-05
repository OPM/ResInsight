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

#include "RimPlot.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <qdatetime.h>

#include <QPointer>
#include <map>
#include <set>

class RigAccWellFlowCalculator;
class RimEclipseResultCase;
class RimFlowDiagSolution;
class RimHistoryWellFlowDataCollection;
class RimSimWellInView;
class RiuPlotWidget;
class RiuQwtPlotWidget;

namespace cvf
{
class Color3f;
}

class RimHistoryWellAllocationPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum FlowValueType
    {
        PERCENTAGE,
        FLOW_RATE,
        FLOW_VOLUME,
        ACCUMULATED_FLOW_VOLUME,
    };

public:
    RimHistoryWellAllocationPlot();
    ~RimHistoryWellAllocationPlot() override;

    void setDescription( const QString& description );
    void setFromSimulationWell( RimSimWellInView* simWell );

    // RimPlot implementations
    RiuPlotWidget* plotWidget() override;
    void           setAutoScaleXEnabled( bool enabled ) override{};
    void           setAutoScaleYEnabled( bool enabled ) override{};
    void           updateAxes() override{};
    void           updateLegend() override{};
    QString        asciiDataForPlotExport() const override;
    void           reattachAllCurves() override{};
    void           detachAllCurves() override{};

    // RimPlotWindow implementations
    QString description() const override;

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override{};

private:
    // RimPlot implementations
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    // RimViewWindow implementations
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

    // PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void                             updateFromWell();
    RimHistoryWellFlowDataCollection createHistoryWellFlowDataCollection();
    std::set<QString>                findSortedWellNames();
    cvf::Color3f                     getTracerColor( const QString& tracerName );

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    QString                       getYAxisTitleFromValueType() const;

private:
    caf::PdmField<QString>                  m_userName;
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmField<QString>                  m_wellName;
    caf::PdmField<bool>                     m_branchDetection;

    caf::PdmPtrField<RimFlowDiagSolution*>     m_flowDiagSolution;
    caf::PdmField<caf::AppEnum<FlowValueType>> m_flowValueType;
    caf::PdmField<bool>                        m_groupSmallContributions;
    caf::PdmField<double>                      m_smallContributionsThreshold;

    QPointer<RiuQwtPlotWidget> m_plotWidget;

    // TODO: Add options? See: RimWellAllocationPlot
};
