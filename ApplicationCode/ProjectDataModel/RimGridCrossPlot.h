/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmObject.h"

#include "RimViewWindow.h"

#include <QPointer>

class RimGridCrossPlotCurveSet;
class QwtPlot;

class RimGridCrossPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridCrossPlot();
    ~RimGridCrossPlot() = default;

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    void     calculateZoomRangeAndUpdateQwt();
    void     attachPlotCurvesToQwtAndReplot();
protected:
    QWidget* createViewWidget(QWidget* mainWindowParent) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;

private:
    caf::PdmChildField<RimGridCrossPlotCurveSet*> m_crossPlotCurveSet;
    QPointer<QwtPlot>                             m_qwtPlot;
};



