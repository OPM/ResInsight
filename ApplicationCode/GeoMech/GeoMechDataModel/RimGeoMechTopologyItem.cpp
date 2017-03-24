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

#include "RimGeoMechTopologyItem.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"


#include "RiuSelectionManager.h"


CAF_PDM_SOURCE_INIT(RimGeoMechTopologyItem, "RimGeoMechTopologyItem");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechTopologyItem::RimGeoMechTopologyItem()
{
    CAF_PDM_InitObject("GeoMech Topology Item", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_geoMechCase, "GeoMechCase", "Geo Mech Case", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_gridIndex, "m_gridIndex", "m_gridIndex", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellIndex, "m_cellIndex", "m_cellIndex", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_elementFace, "m_elementFace", "m_elementFace", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_hasIntersectionTriangle, "m_hasIntersectionTriangle", "m_hasIntersectionTriangle", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_intersectionTriangle_0, "m_intersectionTriangle_0", "m_intersectionTriangle_0", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_intersectionTriangle_1, "m_intersectionTriangle_1", "m_intersectionTriangle_1", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_intersectionTriangle_2, "m_intersectionTriangle_2", "m_intersectionTriangle_2", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_localIntersectionPoint, "m_localIntersectionPoint", "m_localIntersectionPoint", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechTopologyItem::~RimGeoMechTopologyItem()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechTopologyItem::setFromSelectionItem(const RiuGeoMechSelectionItem* selectionItem)
{
    m_geoMechCase = selectionItem->m_view->geoMechCase();

    m_gridIndex = selectionItem->m_gridIndex;
    m_cellIndex = selectionItem->m_cellIndex;
    m_elementFace = selectionItem->m_elementFace;
    m_hasIntersectionTriangle = selectionItem->m_hasIntersectionTriangle;
    m_intersectionTriangle_0 = cvf::Vec3d(selectionItem->m_intersectionTriangle[0]);
    m_intersectionTriangle_1 = cvf::Vec3d(selectionItem->m_intersectionTriangle[1]);
    m_intersectionTriangle_2 = cvf::Vec3d(selectionItem->m_intersectionTriangle[2]);

    m_localIntersectionPoint = selectionItem->m_localIntersectionPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechTopologyItem::topologyText() const
{
    QString text;

/*
    if (m_geoMechCase)
    {
        text += m_geoMechCase->caseUserDescription();
    }
    else
    {
        text = "No case";
    }
    
    text += ", ";
    text += QString("Grid index %1").arg(m_gridIndex);
    text += ", ";
    text += RigTimeHistoryResultAccessor::topologyText(m_geoMechCase->eclipseCaseData(), m_gridIndex, m_cellIndex);
*/

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimGeoMechTopologyItem::geoMechCase() const
{
    return m_geoMechCase;
}

