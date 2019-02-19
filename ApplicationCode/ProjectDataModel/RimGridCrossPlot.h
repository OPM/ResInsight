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

class RimGridCrossPlotCurve;
class QwtPlot;

class RimGridCrossPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridCrossPlot();
    ~RimGridCrossPlot() = default;

    virtual QWidget* viewWidget() override;
    virtual QImage snapshotWindowContent() override;
    virtual void zoomAll() override;

protected:
    virtual QWidget* createViewWidget(QWidget* mainWindowParent) override;
    virtual void deleteViewWidget() override;
    virtual void onLoadDataAndUpdate() override;

private:
    caf::PdmChildField<RimGridCrossPlotCurve*> m_crossPlotCurve;
    QPointer<QwtPlot>                          m_qwtPlot;
};



