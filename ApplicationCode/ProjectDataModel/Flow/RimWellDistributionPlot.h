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

#include "RimPlotWindow.h"
#include "RimPlotInterface.h"


//==================================================================================================
//
//
//
//==================================================================================================
class RimWellDistributionPlot : public RimPlotWindow, public RimPlotInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellDistributionPlot();
    ~RimWellDistributionPlot() override;

    // RimViewWindow/RimPlotWindow implementations/overrides
    virtual QWidget*            viewWidget() override;
    virtual QImage              snapshotWindowContent() override;
    virtual void                zoomAll() override;
    virtual void                detachAllCurves() override;
    virtual void                updateLayout() override;

    // RimPlotInterface implementation
    virtual RiuQwtPlotWidget*   viewer() override;
    virtual bool                isChecked() const override;
    virtual void                setChecked(bool checked) override;
    virtual QString             description() const override;
    virtual bool                hasCustomFontSizes(RiaDefines::FontSettingType fontSettingType, int defaultFontSize) const override;
    virtual bool                applyFontSize(RiaDefines::FontSettingType fontSettingType, int oldFontSize, int fontSize, bool forceChange = false) override;
    virtual void                setAutoScaleXEnabled(bool enabled) override;
    virtual void                setAutoScaleYEnabled(bool enabled) override;
    virtual void                updateAxes() override;
    virtual void                updateZoomInQwt() override;
    virtual void                updateZoomFromQwt() override;
    virtual void                createPlotWidget() override;
    //virtual void                detachAllCurves() override;
    virtual caf::PdmObject*     findPdmObjectFromQwtCurve(const QwtPlotCurve* curve) const override;
    virtual void                loadDataAndUpdate() override;

private:
    // RimViewWindow/RimPlotWindow implementations/overrides
    virtual QWidget*    createViewWidget(QWidget* mainWindowParent) override;
    virtual void        deleteViewWidget() override;
    virtual void        onLoadDataAndUpdate() override;
    virtual void        updatePlotTitle() override;
};
