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

#include "RiuPvtPlotPanel.h"
#include "RiuSummaryQwtPlot.h"

#include "RigFlowDiagSolverInterface.h"

#include "cvfBase.h"
#include "cvfAssert.h"
//#include "cvfTrace.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_legend.h"

#include <QDockWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>



//==================================================================================================
//
//
//
//==================================================================================================
class MyQwtPlot : public QwtPlot
{
public:
    MyQwtPlot(QWidget* parent) : QwtPlot(parent) {}
    virtual QSize sizeHint() const { return QSize(100, 100); }
    virtual QSize minimumSizeHint() const { return QSize(0, 0); }
};



//==================================================================================================
///
/// \class RiuPvtPlotPanel
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::RiuPvtPlotPanel(QDockWidget* parent)
:   QWidget(parent)
{
    m_phaseComboBox = new QComboBox(this);
    m_phaseComboBox->setEditable(false);
    m_phaseComboBox->addItem("Oil", QVariant("oil"));
    m_phaseComboBox->addItem("Gas", QVariant("gas"));
    m_phaseComboBox->addItem("Water", QVariant("water"));

    QHBoxLayout* comboLayout = new QHBoxLayout();
    comboLayout->addWidget(new QLabel("Phase"));
    comboLayout->addWidget(m_phaseComboBox);
    comboLayout->addStretch(1);

    m_fvfPlot = new MyQwtPlot(this);
    m_viscosityPlot = new MyQwtPlot(this);
    setPlotDefaults(m_fvfPlot);
    setPlotDefaults(m_viscosityPlot);

    m_fvfPlot->setTitle("Formation Volume Factor - N/A");
    m_viscosityPlot->setTitle("Viscosity - N/A");

    QHBoxLayout* plotLayout = new QHBoxLayout();
    plotLayout->addWidget(m_fvfPlot);
    plotLayout->addWidget(m_viscosityPlot);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(comboLayout);
    mainLayout->addLayout(plotLayout);

    setLayout(mainLayout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotPanel::~RiuPvtPlotPanel()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::setPlotDefaults(QwtPlot* plot)
{
    RiuSummaryQwtPlot::setCommonPlotBehaviour(plot);

    plot->enableAxis(QwtPlot::xBottom, true);
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->enableAxis(QwtPlot::xTop, false);
    plot->enableAxis(QwtPlot::yRight, false);

    plot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    plot->setAxisMaxMinor(QwtPlot::yLeft, 3);

    QwtLegend* legend = new QwtLegend(plot);
    plot->insertLegend(legend, QwtPlot::BottomLegend);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::setPlotData(QString cellReferenceText)
{
    //cvf::Trace::show("RiuPvtPlotPanel::setPlotData()");

    m_cellReferenceText = cellReferenceText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotPanel::clearPlot()
{
    //cvf::Trace::show("RiuPvtPlotPanel::clearPlot()");

    if (m_cellReferenceText.isEmpty())
    {
        return;
    }

    m_cellReferenceText.clear();

    //m_fvfPlot->detachItems(QwtPlotItem::Rtti_PlotItem, true);
    //m_viscosityPlot->detachItems(QwtPlotItem::Rtti_PlotItem, true);

    //m_fvfPlot->replot();
    //m_viscosityPlot->replot();
}

