/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaDefines.h"
#include "RiaNumberFormat.h"

#include "RigHistogramData.h"

#include "RimPlotWindow.h"

#include "RiuQtChartView.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"

#include "cvfColor3.h"

#include <QPointer>

//==================================================================================================
///
///
//==================================================================================================
class RimStatisticsPlot : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum class HistogramFrequencyType
    {
        ABSOLUTE_FREQUENCY,
        RELATIVE_FREQUENCY,
        RELATIVE_FREQUENCY_PERCENT
    };

    enum class GraphType
    {
        BAR_GRAPH,
        LINE_GRAPH
    };

    RimStatisticsPlot();
    ~RimStatisticsPlot() override;

    QWidget* viewWidget() override;
    QWidget* createPlotWidget( QWidget* mainWindowParent = nullptr );
    QString  description() const override;

    void zoomAll() override;

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void uiOrderingForHistogram( QString uiConfigName, caf::PdmUiOrdering& uiOrdering, bool showHistogramBins );

    void                 onLoadDataAndUpdate() override;
    void                 updatePlots();
    caf::PdmFieldHandle* userDescriptionField() override;

    virtual bool             hasStatisticsData() const    = 0;
    virtual RigHistogramData createStatisticsData() const = 0;
    virtual QString          createAutoName() const       = 0;

    virtual QString createXAxisTitle() const = 0;
    virtual QString createYAxisTitle() const;

    void performAutoNameUpdate();

private:
    void cleanupBeforeClose();
    void onPlotAdditionOrRemoval();
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;
    void doUpdateLayout() override;

protected:
    QPointer<RiuQtChartView> m_viewer;

    caf::PdmField<QString>                                         m_plotWindowTitle;
    caf::PdmField<int>                                             m_numHistogramBins;
    caf::PdmField<cvf::Color3f>                                    m_histogramBarColor;
    caf::PdmField<double>                                          m_histogramGapWidth;
    caf::PdmField<caf::AppEnum<HistogramFrequencyType>>            m_histogramFrequencyType;
    caf::PdmField<int>                                             m_precision;
    caf::PdmField<caf::AppEnum<RiaNumberFormat::NumberFormatType>> m_tickNumberFormat;
    caf::PdmField<caf::AppEnum<GraphType>>                         m_graphType;
};
