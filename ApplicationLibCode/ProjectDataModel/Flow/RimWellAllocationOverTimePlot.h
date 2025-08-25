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

#include <QDateTime>
#include <QPointer>

#include <map>
#include <set>
#include <vector>

class RigAccWellFlowCalculator;
class RimEclipseResultCase;
class RimFlowDiagSolution;
class RigWellAllocationOverTime;
class RimSimWellInView;
class RiuPlotWidget;
class RiuQwtPlotWidget;

namespace cvf
{
class Color3f;
}

class RimWellAllocationOverTimePlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum class FlowValueType
    {
        FLOW_RATE,
        FLOW_RATE_PERCENTAGE,
        FLOW_VOLUME,
        ACCUMULATED_FLOW_VOLUME,
        ACCUMULATED_FLOW_VOLUME_PERCENTAGE,
    };
    enum class TimeStepFilterMode
    {
        NONE,
        TIME_STEP_COUNT,
    };

public:
    RimWellAllocationOverTimePlot();
    ~RimWellAllocationOverTimePlot() override;

    void setDescription( const QString& description );
    void setFromSimulationWell( RimSimWellInView* simWell );
    void setWellName( const QString& wellName );

    // RimPlot implementations
    RiuPlotWidget* plotWidget() override;
    void           setAutoScaleXEnabled( bool enabled ) override {};
    void           setAutoScaleYEnabled( bool enabled ) override {};
    void           updateAxes() override {};
    void           updateLegend() override {};
    QString        asciiDataForPlotExport() const override;
    void           reattachAllCurves() override {};
    void           detachAllCurves() override {};

    // RimPlotWindow implementations
    QString description() const override;

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override {};

private:
    // RimPlot implementations
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    // RimViewWindow implementations
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

    // PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void doUpdateLayout() override;

    void                      updateFromWell();
    RigWellAllocationOverTime createWellAllocationOverTime() const;
    std::set<QString>         findSortedWellNames();
    cvf::Color3f              getTracerColor( const QString& tracerName );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    QString                       getValueTypeText() const;
    QString                       dateFormatString() const;

    void setValidTimeStepRangeForCase();

    int axisTitleFontSize() const;
    int axisValueFontSize() const;

    std::set<QDateTime> getSelectedTimeSteps( const std::vector<QDateTime>& timeSteps ) const;

private:
    caf::PdmField<QString>                  m_userName;
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmField<QString>                  m_wellName;

    caf::PdmField<QDateTime>                        m_selectedFromTimeStep;
    caf::PdmField<QDateTime>                        m_selectedToTimeStep;
    caf::PdmField<caf::AppEnum<TimeStepFilterMode>> m_timeStepFilterMode;
    caf::PdmField<int>                              m_timeStepCount;
    caf::PdmField<std::vector<QDateTime>>           m_excludeTimeSteps;
    caf::PdmField<bool>                             m_applyTimeStepSelections;

    caf::PdmPtrField<RimFlowDiagSolution*>     m_flowDiagSolution;
    caf::PdmField<caf::AppEnum<FlowValueType>> m_flowValueType;
    caf::PdmField<bool>                        m_groupSmallContributions;
    caf::PdmField<double>                      m_smallContributionsThreshold;

    QPointer<RiuQwtPlotWidget> m_plotWidget;
    RimFontSizeField           m_axisTitleFontSize;
    RimFontSizeField           m_axisValueFontSize;

    const int m_initialNumberOfTimeSteps = 10;
};
