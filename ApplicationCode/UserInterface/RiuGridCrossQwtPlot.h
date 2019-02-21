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

#include "RiuInterfaceToViewWindow.h"

#include "cafPdmPointer.h"

#include "qwt_plot.h"

#include <QPointer>

class RimGridCrossPlot;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuGridCrossQwtPlot : public QwtPlot, public RiuInterfaceToViewWindow
{
    Q_OBJECT;

public:
    RiuGridCrossQwtPlot(RimGridCrossPlot* plotDefinition, QWidget* parent = nullptr);
    ~RiuGridCrossQwtPlot() override;

    RimGridCrossPlot* ownerPlotDefinition();
    RimViewWindow*    ownerViewWindow() const override;

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
private:
    caf::PdmPointer<RimGridCrossPlot> m_plotDefinition;

};
