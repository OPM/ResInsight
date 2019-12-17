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

#include "RimPlot.h"
#include "RiaDefines.h"

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
    explicit RimWellDistributionPlot(RiaDefines::PhaseType phase = RiaDefines::OIL_PHASE);
    ~RimWellDistributionPlot() override;

    void                        setDataSourceParameters(RimEclipseResultCase* eclipseResultCase, int timeStepIndex, QString targetWellName);
    void                        setPlotOptions(bool groupSmallContributions, double smallContributionsRelativeThreshold);

    // RimPlot implementations
    virtual RiuQwtPlotWidget*   viewer() override;
    virtual void                setAutoScaleXEnabled(bool enabled) override;
    virtual void                setAutoScaleYEnabled(bool enabled) override;
    virtual void                updateAxes() override;
    virtual void                updateLegend() override;
    virtual void                updateZoomInQwt() override;
    virtual void                updateZoomFromQwt() override;
    virtual QString             asciiDataForPlotExport() const override;
    virtual void                reattachAllCurves() override;
    virtual void                detachAllCurves() override;
    virtual caf::PdmObject*     findPdmObjectFromQwtCurve(const QwtPlotCurve* curve) const override;
    virtual void                onAxisSelected(int axis, bool toggle) override;

    // RimPlotWindow implementations
    virtual QString             description() const override;

    // RimViewWindow implementations
    virtual QWidget*            viewWidget() override;
    virtual QImage              snapshotWindowContent() override;
    virtual void                zoomAll() override;

private:
    // RimPlot implementations
    virtual void        doRemoveFromCollection() override;

    // RimViewWindow implementations
    virtual QWidget*    createViewWidget(QWidget* mainWindowParent) override;
    virtual void        deleteViewWidget() override;
    virtual void        onLoadDataAndUpdate() override;

private:
    void                fixupDependentFieldsAfterCaseChange();
    static void         populatePlotWidgetWithCurveData(const RigTofWellDistributionCalculator& calculator, const RimFlowDiagSolution& flowDiagSolution, RiuQwtPlotWidget* plotWidget);

    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    caf::PdmPtrField<RimEclipseResultCase*>             m_case;
    caf::PdmField<int>                                  m_timeStepIndex;
    caf::PdmField<QString>                              m_wellName;
    caf::PdmField< caf::AppEnum<RiaDefines::PhaseType>> m_phase;
    caf::PdmField<bool>                                 m_groupSmallContributions;
    caf::PdmField<double>                               m_smallContributionsRelativeThreshold;

    QPointer<RiuQwtPlotWidget>                          m_plotWidget;
};
