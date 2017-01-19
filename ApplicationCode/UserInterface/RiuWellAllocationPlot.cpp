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
#include "QBoxLayout"




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::RiuWellAllocationPlot(RimWellAllocationPlot* plotDefinition, QWidget* parent)
    : QFrame(parent)
{
    Q_ASSERT(plotDefinition);
    this->setLayout(new QVBoxLayout());
    this->layout()->setMargin(0);

    m_plotDefinition = plotDefinition;
    QWidget* wellFlowWidget = m_plotDefinition->accumulatedWellFlowPlot()->createViewWidget(this);
    this->layout()->addWidget(wellFlowWidget);

 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellAllocationPlot::~RiuWellAllocationPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleViewerDeletion();
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

