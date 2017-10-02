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

#include "RiuWellRftPlot.h"

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellRftPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimTofAccumulatedPhaseFractionsPlot.h"

#include "RiuContextMenuLauncher.h"
#include "RiuNightchartsWidget.h"
#include "RiuPlotObjectPicker.h"

#include "cvfColor3.h"

#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellRftPlot::RiuWellRftPlot(RimWellRftPlot* plotDefinition, QWidget* parent) 
    :   m_plotDefinition(plotDefinition),
        QFrame(parent)
{
    Q_ASSERT(m_plotDefinition);
    
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->layout()->setSpacing(2);

    m_titleLabel = new QLabel(this);
    new RiuPlotObjectPicker(m_titleLabel, m_plotDefinition->wellLogPlot());

    QFont font = m_titleLabel->font();
    font.setPointSize(14);
    font.setBold(true);
    m_titleLabel->setFont(font);

    // White background
    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    auto plotWidgetsLayout = new QHBoxLayout();
    auto rightColumnLayout = new QVBoxLayout();

    mainLayout->addLayout(plotWidgetsLayout);
    plotWidgetsLayout->addLayout(rightColumnLayout);
    
    //m_legendWidget = new RiuNightchartsWidget(this);
    //new RiuPlotObjectPicker(m_legendWidget, m_plotDefinition->plotLegend());

    //QStringList commandIds;
    //commandIds << "RicShowTotalAllocationDataFeature";
    //new RiuContextMenuLauncher(m_legendWidget, commandIds);

    //rightColumnLayout->addWidget(m_legendWidget);
    //m_legendWidget->showPie(false);

    //QWidget* totalFlowAllocationWidget = m_plotDefinition->totalWellFlowPlot()->createViewWidget(this);
    //new RiuPlotObjectPicker(totalFlowAllocationWidget, m_plotDefinition->totalWellFlowPlot());
    //new RiuContextMenuLauncher(totalFlowAllocationWidget, commandIds);

    //rightColumnLayout->addWidget(totalFlowAllocationWidget, Qt::AlignTop);
    //rightColumnLayout->addWidget(m_plotDefinition->tofAccumulatedPhaseFractionsPlot()->createViewWidget(this), Qt::AlignTop);
    //rightColumnLayout->addStretch();

    QWidget* wellFlowWidget = m_plotDefinition->wellLogPlot()->createViewWidget(this);

    plotWidgetsLayout->addWidget(wellFlowWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellRftPlot::~RiuWellRftPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleMdiWindowClosed();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellRftPlot* RiuWellRftPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuWellRftPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellRftPlot::showTitle(const QString& title)
{
    m_titleLabel->show();

    m_titleLabel->setText(title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellRftPlot::hideTitle()
{
    m_titleLabel->hide();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//void RiuWellRftPlot::showLegend(bool doShow)
//{
//    if (doShow)
//        m_legendWidget->show();
//    else
//        m_legendWidget->hide();
//}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//void RiuWellRftPlot::addLegendItem(const QString& name, const cvf::Color3f& color, float value)
//{
//    QColor sliceColor(color.rByte(), color.gByte(), color.bByte());
//
//    m_legendWidget->addItem(name, sliceColor, value);
//    m_legendWidget->updateGeometry();
//    m_legendWidget->update();
//}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//void RiuWellRftPlot::clearLegend()
//{
//    m_legendWidget->clear();
//}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellRftPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellRftPlot::contextMenuEvent(QContextMenuEvent* event)
{
    //QMenu menu;
    //QStringList commandIds;

    //commandIds << "RicShowContributingWellsFromPlotFeature";

    //RimContextCommandBuilder::appendCommandsToMenu(commandIds, &menu);

    //if (menu.actions().size() > 0)
    //{
    //    menu.exec(event->globalPos());
    //}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellRftPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellRftPlot::setDefaults()
{
}

