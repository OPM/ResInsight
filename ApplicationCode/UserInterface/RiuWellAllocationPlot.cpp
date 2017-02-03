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

#include "RiuWellAllocationPlot.h"

#include "RiaApplication.h"

#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimTotalWellAllocationPlot.h"

#include <QBoxLayout>
#include <QLabel>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::RiuWellAllocationPlot(RimWellAllocationPlot* plotDefinition, QWidget* parent) 
    :   m_plotDefinition(plotDefinition),
        QFrame(parent)
{
    Q_ASSERT(m_plotDefinition);
    
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->layout()->setSpacing(2);

    m_titleLabel = new QLabel(this);

    QFont font = m_titleLabel->font();
    font.setPointSize(14);
    font.setBold(true);
    m_titleLabel->setFont(font);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    QHBoxLayout* plotWidgetsLayout =  new QHBoxLayout();
    mainLayout->addLayout(plotWidgetsLayout, 10);
    
    QWidget* totalFlowAllocationWidget = m_plotDefinition->totalWellFlowPlot()->createViewWidget(this);
    plotWidgetsLayout->addWidget(totalFlowAllocationWidget);

    QWidget* wellFlowWidget = m_plotDefinition->accumulatedWellFlowPlot()->createViewWidget(this);
    plotWidgetsLayout->addWidget(wellFlowWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::~RiuWellAllocationPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleMdiWindowClosed();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RiuWellAllocationPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::showTitle(const QString& title)
{
    m_titleLabel->show();

    m_titleLabel->setText(title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::hideTitle()
{
    m_titleLabel->hide();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellAllocationPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellAllocationPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellAllocationPlot::setDefaults()
{
}

