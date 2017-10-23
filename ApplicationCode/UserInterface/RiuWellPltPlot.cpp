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

#include "RiuWellPltPlot.h"

#include "RiaApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellPltPlot.h"
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
RiuWellPltPlot::RiuWellPltPlot(RimWellPltPlot* plotDefinition, QWidget* parent) 
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
    
    QWidget* wellFlowWidget = m_plotDefinition->wellLogPlot()->createViewWidget(this);

    plotWidgetsLayout->addWidget(wellFlowWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellPltPlot::~RiuWellPltPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleMdiWindowClosed();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPltPlot* RiuWellPltPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuWellPltPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellPltPlot::showTitle(const QString& title)
{
    m_titleLabel->show();

    m_titleLabel->setText(title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellPltPlot::hideTitle()
{
    m_titleLabel->hide();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellPltPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellPltPlot::contextMenuEvent(QContextMenuEvent* event)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuWellPltPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellPltPlot::setDefaults()
{
}
