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
#include "RimGridCrossPlot.h"

#include "RimGridCrossPlotCurveSet.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT(RimGridCrossPlot, "RimGridCrossPlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlot::RimGridCrossPlot()
{
    CAF_PDM_InitObject("Grid Cross Plot", ":/SummaryXPlotLight16x16.png", "", "");

    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Show Legend", "", "", "");
    CAF_PDM_InitField(&m_legendFontSize, "LegendFontSize", 10, "Legend Font Size", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurveSet, "CrossPlotCurve", "Cross Plot Data Set", "", "", "");

    m_crossPlotCurveSet.uiCapability()->setUiHidden(true);
    m_crossPlotCurveSet = new RimGridCrossPlotCurveSet();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::viewWidget()
{
    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimGridCrossPlot::snapshotWindowContent()
{
    QImage image;

    if (m_qwtPlot)
    {
        QPixmap pix = QPixmap::grabWidget(m_qwtPlot);
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::calculateZoomRangeAndUpdateQwt()
{
    // this->calculateXZoomRange();
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::reattachCurvesToQwtAndReplot()
{
    m_crossPlotCurveSet->setParentQwtPlotNoReplot(m_qwtPlot);
    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGridCrossPlot::createViewWidget(QWidget* mainWindowParent)
{
    if (!m_qwtPlot)
    {
        m_qwtPlot = new QwtPlot(QString("Grid Cross Plot"), mainWindowParent);
    }

    return m_qwtPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::deleteViewWidget()
{
    if (m_qwtPlot)
    {
        m_qwtPlot->deleteLater();
        m_qwtPlot = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    CVF_ASSERT(m_qwtPlot);

    m_crossPlotCurveSet->loadDataAndUpdate(false);
    m_crossPlotCurveSet->setParentQwtPlotNoReplot(m_qwtPlot);
    m_qwtPlot->setTitle("Grid Cross Plot");
    m_qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    m_qwtPlot->setAxisAutoScale(QwtPlot::xBottom);

    if (m_showLegend())
    {
        // Will be released in plot destructor or when a new legend is set
        QwtLegend* legend = new QwtLegend(m_qwtPlot);

        auto font = legend->font();
        font.setPixelSize(m_legendFontSize());
        legend->setFont(font);
        m_qwtPlot->insertLegend(legend, QwtPlot::BottomLegend);
    }
    else
    {
        m_qwtPlot->insertLegend(nullptr);
    }

    m_qwtPlot->replot();
    m_qwtPlot->show();
    this->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_showLegend);

    if (m_showLegend())
    {
        uiOrdering.add(&m_legendFontSize);
    }
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue)
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCrossPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_legendFontSize)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(10);
        fontSizes.push_back(12);
        fontSizes.push_back(14);
        fontSizes.push_back(16);
        

        for (int value : fontSizes)
        {
            QString text = QString("%1").arg(value);
            options.push_back(caf::PdmOptionItemInfo(text, value));
        }
    }
    return options;
}
