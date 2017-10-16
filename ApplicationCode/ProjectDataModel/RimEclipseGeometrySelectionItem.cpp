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

#include "RimEclipseGeometrySelectionItem.h"

#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"

#include "RiuSelectionManager.h"


CAF_PDM_SOURCE_INIT(RimEclipseGeometrySelectionItem, "EclipseGeometrySelectionItem");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem::RimEclipseGeometrySelectionItem()
{
    CAF_PDM_InitObject("Eclipse Geometry Selection Item", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseCase,            "EclipseCase",            "Eclipse Case",             "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_gridIndex,              "GridIndex",              "m_gridIndex",              "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellIndex,              "CellIndex",              "m_cellIndex",              "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_localIntersectionPoint, "LocalIntersectionPoint", "m_localIntersectionPoint", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem::~RimEclipseGeometrySelectionItem()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setFromSelectionItem(const RiuEclipseSelectionItem* selectionItem)
{
    m_gridIndex = selectionItem->m_gridIndex;
    m_cellIndex = selectionItem->m_gridLocalCellIndex;
    m_localIntersectionPoint = selectionItem->m_localIntersectionPoint;

    m_eclipseCase = selectionItem->m_view->eclipseCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseGeometrySelectionItem::geometrySelectionText() const
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
    text += RigTimeHistoryResultAccessor::geometrySelectionText(m_eclipseCase->eclipseCaseData(), m_gridIndex, m_cellIndex);

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseGeometrySelectionItem::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseGeometrySelectionItem::gridIndex() const
{
    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseGeometrySelectionItem::cellIndex() const
{
    return m_cellIndex;
}
