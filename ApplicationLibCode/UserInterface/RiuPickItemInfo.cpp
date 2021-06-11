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
#include "cvfEffect.h"
#include "cvfHitItem.h"
#include "cvfHitItemCollection.h"
#include "cvfPart.h"
#include "cvfRenderState.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfTransform.h"

#include <cmath>

double RiuPickItemInfo::sm_rayDistanceTolerance = 1e-5;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPickItemInfo RiuPickItemInfo::extractPickItemInfo( const cvf::HitItem* hitItem )
{
    RiuPickItemInfo pickInfo;

    pickInfo.m_pickedPart        = hitItem->part();
    pickInfo.m_globalPickedPoint = hitItem->intersectionPoint();
    pickInfo.m_distanceAlongRay  = hitItem->distanceAlongRay();
    if ( pickInfo.m_pickedPart ) pickInfo.m_sourceInfo = pickInfo.m_pickedPart->sourceInfo();

    const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>( hitItem->detail() );
    if ( detail ) pickInfo.m_faceIdx = detail->faceIndex();

    pickInfo.m_localPickedPoint = pickInfo.m_globalPickedPoint;
    const cvf::Transform* xf    = pickInfo.m_pickedPart->transform();
    if ( xf )
    {
        pickInfo.m_localPickedPoint.transformPoint( xf->worldTransform().getInverted() );
    }

    return pickInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiuPickItemInfo> RiuPickItemInfo::convertToPickItemInfos( const cvf::HitItemCollection& hitItems,
                                                                      const cvf::Vec3d&             globalRayOrigin,
                                                                      double coincidentRayDistanceTolerance )
{
    sm_rayDistanceTolerance = coincidentRayDistanceTolerance;

    std::set<RiuPickItemInfo> pickItemInfosSorted;
    for ( size_t i = 0; i < hitItems.count(); i++ )
    {
        pickItemInfosSorted.insert( RiuPickItemInfo( hitItems.item( i ), globalRayOrigin ) );
    }

    std::vector<RiuPickItemInfo> pickItemInfos;

    pickItemInfos.insert( pickItemInfos.begin(), pickItemInfosSorted.begin(), pickItemInfosSorted.end() );

    return pickItemInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RiuPickItemInfo::polygonOffsetUnit() const
{
    float polyOffsetUnit = 0;

    if ( m_pickedPart )
    {
        cvf::Effect* eff = const_cast<cvf::Part*>( m_pickedPart )->effect();

        if ( eff )
        {
            cvf::RenderState* rendState = eff->renderStateOfType( cvf::RenderState::POLYGON_OFFSET );
            if ( rendState )
            {
                auto polyOffsetRenderState = static_cast<cvf::RenderStatePolygonOffset*>( rendState );

                polyOffsetUnit = polyOffsetRenderState->units();
            }
        }
    }
    return polyOffsetUnit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPickItemInfo::operator<( const RiuPickItemInfo& other ) const
{
    if ( fabs( m_distanceAlongRay - other.distanceAlongRay() ) > sm_rayDistanceTolerance )
    {
        return m_distanceAlongRay < other.distanceAlongRay();
    }
    else if ( this->polygonOffsetUnit() != other.polygonOffsetUnit() )
    {
        return this->polygonOffsetUnit() < other.polygonOffsetUnit();
    }
    else if ( m_faceIdx != other.faceIdx() )
    {
        return m_faceIdx < other.faceIdx();
    }
    else if ( m_pickedPart != other.pickedPart() )
    {
        return m_pickedPart < other.pickedPart();
    }

    return false;
}
