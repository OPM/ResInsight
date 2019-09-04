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
    : RiuWellLogPlot(plotDefinition, parent)
{
    Q_ASSERT(m_plotDefinition);
    
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->layout()->setSpacing(2);

    new RiuPlotObjectPicker(m_plotTitle, m_plotDefinition);

    mainLayout->addWidget(m_plotTitle, 0, Qt::AlignCenter);

    auto plotWidgetsLayout = new QHBoxLayout();
    auto rightColumnLayout = new QVBoxLayout();

    mainLayout->addLayout(plotWidgetsLayout);
    plotWidgetsLayout->addLayout(rightColumnLayout);
    
    QWidget* wellFlowWidget = m_plotDefinition->createPlotWidget();

    plotWidgetsLayout->addWidget(wellFlowWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellPltPlot::~RiuWellPltPlot()
{
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
