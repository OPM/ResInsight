/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuWellLogPlot.h"

#include "cafPdmPointer.h"

#include "qwt_plot.h"

#include <QPointer>
#include <QFrame>

class RimWellAllocationPlot;
class RiuNightchartsWidget;

class QLabel;

namespace cvf {
    class Color3f;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuWellAllocationPlot : public RiuWellLogPlot
{
    Q_OBJECT;
public:
    RiuWellAllocationPlot(RimWellAllocationPlot* plotDefinition, QWidget* parent = nullptr);
    ~RiuWellAllocationPlot() override;

    void                            showLegend(bool doShow);
    void                            addLegendItem(const QString& name, const cvf::Color3f& color, float value);
    void                            clearLegend();

protected:
    QSize                   sizeHint() const override;
    QSize                   minimumSizeHint() const override;

    void                    contextMenuEvent(QContextMenuEvent *) override;

private:
    QPointer<RiuNightchartsWidget> m_legendWidget;
};
