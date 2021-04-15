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
#include "RiuPlotAnnotationTool.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmPointer.h"

#include <QPointer>

#include <memory>

class RimGridCrossPlot;
class RimGridCrossPlotDataSet;
class RimPlotAxisPropertiesInterface;
class RimPlot;
class RiuCvfOverlayItemWidget;
class RiuDraggableOverlayFrame;
class RiuPlotAnnotationTool;
class RiuQwtPlotZoomer;

namespace caf
{
class TitledOverlayFrame;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuGridCrossQwtPlot : public RiuQwtPlotWidget
{
    Q_OBJECT;

public:
    RiuGridCrossQwtPlot( RimGridCrossPlot* crossPlot, QWidget* parent = nullptr );
    ~RiuGridCrossQwtPlot() override;

    RiuGridCrossQwtPlot( const RiuGridCrossQwtPlot& ) = delete;

    void updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties );

    void setLegendFontSize( int fontSize );
    void setInternalQwtLegendVisible( bool visible );

protected:
    bool curveText( const QwtPlotCurve* curve, QString* curveTitle, QString* xParamName, QString* yParamName ) const;
    bool isZoomerActive() const override;
    void endZoomOperations() override;

private slots:
    void onZoomedSlot();
    void onPlotItemSelected( QwtPlotItem* selectedItem, bool toggleItem, int sampleIndex );

private:
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
    QwtPlotMarker*                         m_selectedPointMarker;

    QPointer<RiuQwtPlotZoomer> m_zoomerLeft;
    QPointer<RiuQwtPlotZoomer> m_zoomerRight;
};
