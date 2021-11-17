/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaDefines.h"
#include "RimPlot.h"

#include "cafPdmPtrField.h"

#include <QPointer>

class RimEclipseResultCase;
class RimFlowDiagSolution;
class RigTofWellDistributionCalculator;
class RiuQwtPlotWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RimWellDistributionPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    explicit RimWellDistributionPlot( RiaDefines::PhaseType phase = RiaDefines::PhaseType::OIL_PHASE );
    ~RimWellDistributionPlot() override;

    void setDataSourceParameters( RimEclipseResultCase* eclipseResultCase, int timeStepIndex, QString targetWellName );
    void setPlotOptions( bool groupSmallContributions, double smallContributionsRelativeThreshold, double maximumTof );

    RiaDefines::PhaseType phase() const;

    // RimPlot implementations
    RiuQwtPlotWidget* viewer() override;
    RiuPlotWidget*    plotWidget() override;
    void              setAutoScaleXEnabled( bool enabled ) override;
    void              setAutoScaleYEnabled( bool enabled ) override;
    void              updateAxes() override;
    void              updateLegend() override;
    void              updateZoomInParentPlot() override;
    void              updateZoomFromParentPlot() override;
    QString           asciiDataForPlotExport() const override;
    void              reattachAllCurves() override;
    void              detachAllCurves() override;
    caf::PdmObject*   findPdmObjectFromPlotCurve( const RiuPlotCurve* curve ) const override;
    void              onAxisSelected( int axis, bool toggle ) override;

    // RimPlotWindow implementations
    QString description() const override;

    // RimViewWindow implementations
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

private:
    // RimViewWindow implementations
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

private:
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent ) override;

    void        fixupDependentFieldsAfterCaseChange();
    static void populatePlotWidgetWithCurveData( const RigTofWellDistributionCalculator& calculator,
                                                 const RimFlowDiagSolution&              flowDiagSolution,
                                                 RiuQwtPlotWidget*                       plotWidget,
                                                 double                                  maximumTof );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    caf::PdmPtrField<RimEclipseResultCase*>            m_case;
    caf::PdmField<int>                                 m_timeStepIndex;
    caf::PdmField<QString>                             m_wellName;
    caf::PdmField<caf::AppEnum<RiaDefines::PhaseType>> m_phase;
    caf::PdmField<bool>                                m_groupSmallContributions;
    caf::PdmField<double>                              m_smallContributionsRelativeThreshold;
    caf::PdmField<double>                              m_maximumTof;

    QPointer<RiuQwtPlotWidget> m_plotWidget;
};
