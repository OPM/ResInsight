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
#include "RiuQwtPlot.h"

#include "cafPdmPointer.h"

#include <QPointer>

#include <memory>

class RimGridCrossPlotDataSet;
class RimPlotAxisProperties;
class RiuCvfOverlayItemWidget;
class RiuDraggableOverlayFrame;
class RiuPlotAnnotationTool;

namespace caf
{
class TitledOverlayFrame;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuGridCrossQwtPlot : public RiuQwtPlot
{
    Q_OBJECT;

public:
    RiuGridCrossQwtPlot(RimViewWindow* ownerViewWindow, QWidget* parent = nullptr);
    ~RiuGridCrossQwtPlot();
    void addOrUpdateDataSetLegend(RimGridCrossPlotDataSet* dataSetToShowLegendFor);
    void removeDataSetLegend(RimGridCrossPlotDataSet* dataSetToShowLegendFor);
    void updateLegendSizesToMatchPlot();
    void updateAnnotationObjects(RimPlotAxisProperties* axisProperties);

protected:
    void updateLayout() override;
    void updateInfoBoxLayout();
    void updateLegendLayout();
    void resizeEvent(QResizeEvent* e) override;
    bool resizeOverlayItemToFitPlot(caf::TitledOverlayFrame* overlayItem);
    void contextMenuEvent(QContextMenuEvent*) override;

    void selectSample(QwtPlotCurve* curve, int sampleNumber) override;
    void clearSampleSelection() override;
    bool curveText(const QwtPlotCurve* curve, QString* curveTitle, QString* xParamName, QString* yParamName) const;
    void applyFontSizeToOverlayItem(caf::TitledOverlayFrame* overlayItem);
private:
    typedef caf::PdmPointer<RimGridCrossPlotDataSet> DataSetPtr;
    typedef QPointer<RiuCvfOverlayItemWidget> LegendPtr;
    typedef QPointer<RiuDraggableOverlayFrame> InfoBoxPtr;

    InfoBoxPtr                             m_infoBox;
    std::map<DataSetPtr, LegendPtr>        m_legendWidgets;
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
    QwtPlotMarker*                         m_selectedPointMarker;

};
