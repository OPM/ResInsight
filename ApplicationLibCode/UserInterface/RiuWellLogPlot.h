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
#include "RiuMultiPlotPage.h"

class RiuQwtPlotWidget;
class RimDepthTrackPlot;

class RiuWellLogPlot : public RiuMultiPlotPage
{
    Q_OBJECT
public:
    RiuWellLogPlot( RimDepthTrackPlot* plotDefinition, QWidget* parent );

    RimViewWindow* ownerViewWindow() const override;

    void updateVerticalScrollBar( double minVisible, double maxVisible, double minAvailable, double maxAvailable ) override;
    void renderTo( QPaintDevice* paintDevice ) override;

protected:
    bool showYAxis( int row, int column ) const override;

    void reinsertScrollbar();
    void alignScrollbar( int offset );

private:
    RimDepthTrackPlot* wellLogPlotDefinition();

private slots:
    void slotSetMinDepth( int value );
    void performUpdate() override;

private:
    QPointer<QVBoxLayout> m_trackScrollBarLayout;
    QScrollBar*           m_trackScrollBar;
};
