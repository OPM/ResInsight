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

#include "RimEclipseTopologyItem.h"

#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "RiuSelectionManager.h"


CAF_PDM_SOURCE_INIT(RimEclipseTopologyItem, "RimEclipseTopologyItem");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseTopologyItem::RimEclipseTopologyItem()
{
    CAF_PDM_InitObject("Eclipse Topoloty Item", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseCase, "EclipseCase", "Eclipse Case", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_gridIndex, "m_gridIndex", "m_gridIndex", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellIndex, "m_cellIndex", "m_cellIndex", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_localIntersectionPoint, "m_localIntersectionPoint", "m_localIntersectionPoint", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseTopologyItem::~RimEclipseTopologyItem()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseTopologyItem::setFromSelectionItem(const RiuEclipseSelectionItem* selectionItem)
{
    m_gridIndex = selectionItem->m_gridIndex;
    m_cellIndex = selectionItem->m_cellIndex;
    m_localIntersectionPoint = selectionItem->m_localIntersectionPoint;

    m_eclipseCase = selectionItem->m_view->eclipseCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseTopologyItem::topologyText() const
{
    QString text;

    if (m_eclipseCase)
    {
        text += m_eclipseCase->caseUserDescription();
    }
    else
    {
        text = "No case";
    }
    
    text += ", ";
    text += QString("Grid index %1").arg(m_gridIndex);
    text += ", ";
    text += RigTimeHistoryResultAccessor::topologyText(m_eclipseCase->eclipseCaseData(), m_gridIndex, m_cellIndex);

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseTopologyItem::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseTopologyItem::gridIndex() const
{
    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseTopologyItem::cellIndex() const
{
    return m_cellIndex;
}
