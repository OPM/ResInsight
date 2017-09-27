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

#include "qwt_plot.h"

#include "cafPdmPointer.h"

#include <QPointer>
#include <QFrame>

#include "RiuInterfaceToViewWindow.h"

class RimWellRftPlot;
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
class RiuWellRftPlot : public QFrame, public RiuInterfaceToViewWindow
{
    Q_OBJECT;
public:
    RiuWellRftPlot(RimWellRftPlot* plotDefinition, QWidget* parent = NULL);
    virtual ~RiuWellRftPlot();

    RimWellRftPlot*          ownerPlotDefinition();
    virtual RimViewWindow*          ownerViewWindow() const override;

    void                            showTitle(const QString& title);
    void                            hideTitle();
    void                            showLegend(bool doShow);
    void                            addLegendItem(const QString& name, const cvf::Color3f& color, float value);
    void                            clearLegend();


protected:
    virtual QSize                   sizeHint() const override;
    virtual QSize                   minimumSizeHint() const override;

    virtual void                    contextMenuEvent(QContextMenuEvent *) override;

private:
    void                            setDefaults();

private:
    caf::PdmPointer<RimWellRftPlot> m_plotDefinition;
    QPointer<RiuNightchartsWidget> m_legendWidget;
    QPointer<QLabel> m_titleLabel;
};
