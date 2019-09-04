/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuPickItemInfo.h"

#include "cvfDrawableGeo.h"
#include "cvfHitItem.h"
#include "cvfHitItemCollection.h"
#include "cvfPart.h"
#include "cvfTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPickItemInfo RiuPickItemInfo::extractPickItemInfo(const cvf::HitItem* hitItem)
{
    RiuPickItemInfo pickInfo;

    pickInfo.m_pickedPart        = hitItem->part();
    pickInfo.m_globalPickedPoint = hitItem->intersectionPoint();
    if (pickInfo.m_pickedPart) pickInfo.m_sourceInfo = pickInfo.m_pickedPart->sourceInfo();

    const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(hitItem->detail());
    if (detail) pickInfo.m_faceIdx = detail->faceIndex();

    pickInfo.m_localPickedPoint = pickInfo.m_globalPickedPoint;
    const cvf::Transform* xf    = pickInfo.m_pickedPart->transform();
    if (xf)
    {
        pickInfo.m_localPickedPoint.transformPoint(xf->worldTransform().getInverted());
    }

    return pickInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiuPickItemInfo> RiuPickItemInfo::convertToPickItemInfos(const cvf::HitItemCollection& hitItems,
                                                                     const cvf::Vec3d&             globalRayOrigin)
{
    std::vector<RiuPickItemInfo> pickItemInfos;
    pickItemInfos.reserve(hitItems.count());
    for (size_t i = 0; i < hitItems.count(); i++)
    {
        pickItemInfos.emplace_back(RiuPickItemInfo(hitItems.item(i), globalRayOrigin));
    }
    return pickItemInfos;
}
