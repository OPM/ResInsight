/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicGridStatisticsDialog.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "Rim3dOverlayInfoConfig.h"

#include "RiuMainPlotWindow.h"
#include "RiuSummaryQwtPlot.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_series_data.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <vector>
#include "qwt_scale_draw.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGridStatisticsDialog::RicGridStatisticsDialog(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    // Create widgets
    m_label = new QLabel();
    m_textEdit = new QTextEdit();
    m_historgramPlot = new QwtPlot();
    m_aggregatedPlot = new QwtPlot();
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Close);

    // Connect to signal
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(slotDialogFinished()));

    // Set widget properties
    m_textEdit->setReadOnly(true);
    RiuSummaryQwtPlot::setCommonPlotBehaviour(m_historgramPlot);
    RiuSummaryQwtPlot::setCommonPlotBehaviour(m_aggregatedPlot);

    // Define layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_label);
    layout->addWidget(m_textEdit);

    QVBoxLayout* plotLayout = new QVBoxLayout();
    plotLayout->setSpacing(0);
    //plotLayout->addStretch();
    plotLayout->addWidget(m_historgramPlot);
    plotLayout->addWidget(m_aggregatedPlot);
    layout->addLayout(plotLayout);

    layout->addWidget(m_buttons);
    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGridStatisticsDialog::~RicGridStatisticsDialog()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::setLabel(const QString& labelText)
{
    m_label->setText(labelText);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::setInfoText(RimView* view)
{
    Rim3dOverlayInfoConfig* overlayInfo = view->overlayInfoConfig();
    if (view && overlayInfo)
    {
        QString text;
        text = overlayInfo->caseInfoText();
        text += overlayInfo->resultInfoText(overlayInfo->histogramData());
        m_textEdit->setText(text);
        adjustTextEditHeightToContent();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::setHistogramData(RimView* view)
{
    deletePlotItems(m_historgramPlot);
    deletePlotItems(m_aggregatedPlot);

    Rim3dOverlayInfoConfig* overlayInfo = view->overlayInfoConfig();
    if (view && overlayInfo)
    {
        auto hist = new QwtPlotHistogram("Histogram");
        auto aggr = new QwtPlotCurve("Aggregated");

        hist->setBrush(QBrush(QColor(Qt::darkYellow)));
        hist->setZ(-1);
        aggr->setStyle(QwtPlotCurve::Steps);
        aggr->setCurveAttribute(QwtPlotCurve::Inverted);

        Rim3dOverlayInfoConfig::HistogramData histogramData = overlayInfo->histogramData();

        if (histogramData.isValid())
        {
            QVector<QwtIntervalSample> histSamples;
            QVector<QPointF> aggrSamples;
            double xStep = (histogramData.max - histogramData.min) / (*histogramData.histogram).size();
            double xCurr = histogramData.min;
            double aggrValue = 0.0;
            for (size_t value : *histogramData.histogram)
            {
                double xNext = xCurr + xStep;
                histSamples.push_back(QwtIntervalSample(value, xCurr, xNext));

                aggrValue += value;
                aggrSamples.push_back(QPointF(xCurr, aggrValue));

                xCurr = xNext;
            }

            // Axis
            double xAxisSize = histogramData.max - histogramData.min;
            double xAxisExtension = xAxisSize * 0.02;
            m_historgramPlot->setAxisScale(QwtPlot::xBottom, histogramData.min - xAxisExtension, histogramData.max + xAxisExtension);
            m_aggregatedPlot->setAxisScale(QwtPlot::xBottom, histogramData.min - xAxisExtension, histogramData.max + xAxisExtension);

            // Set y axis label area width
            m_historgramPlot->axisScaleDraw(QwtPlot::yLeft)->setMinimumExtent(60);
            m_aggregatedPlot->axisScaleDraw(QwtPlot::yLeft)->setMinimumExtent(60);

            // Samples
            hist->setSamples(histSamples);
            aggr->setSamples(aggrSamples);
            hist->attach(m_historgramPlot);
            aggr->attach(m_aggregatedPlot);

            // Markers
            setMarkers(histogramData, m_historgramPlot);
            setMarkers(histogramData, m_aggregatedPlot);
        }
    }

    // Refresh plot
    m_historgramPlot->replot();
    m_aggregatedPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::deletePlotItems(QwtPlot* plot)
{
    auto itemList = plot->itemList();
    for (auto item : itemList)
    {
        if (dynamic_cast<QwtPlotMarker*>(item)
            || dynamic_cast<QwtPlotCurve*>(item)
            || dynamic_cast<QwtPlotHistogram*>(item))
        {
            item->detach();
            delete item;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::setMarkers(const Rim3dOverlayInfoConfig::HistogramData& histData, QwtPlot* plot)
{
    auto scale = plot->axisScaleDiv(QwtPlot::yLeft);

    QwtPlotMarker* marker;

    if (histData.p10 != HUGE_VAL)
    {
        marker = createVerticalPlotMarker(Qt::red, histData.p10);
        marker->attach(plot);
    }

    if (histData.p90 != HUGE_VAL)
    {
        marker = createVerticalPlotMarker(Qt::red, histData.p90);
        marker->attach(plot);
    }

    if (histData.mean != HUGE_VAL)
    {
        marker = createVerticalPlotMarker(Qt::blue, histData.mean);
        marker->attach(plot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlotMarker* RicGridStatisticsDialog::createVerticalPlotMarker(const QColor& color, double xValue)
{
    QwtPlotMarker* marker = new QwtPlotMarker();
    marker->setXValue(xValue);
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->setLinePen(color, 2, Qt::SolidLine);
    return marker;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::adjustTextEditHeightToContent()
{
    int docHeight = m_textEdit->document()->size().height();
    m_textEdit->setFixedHeight(docHeight + 10);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGridStatisticsDialog::slotDialogFinished()
{
    close();
}
