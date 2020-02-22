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

#pragma once

#include "cvfVector3.h"
#include <vector>

namespace cvf
{
class Part;
class Object;
class HitItem;
class HitItemCollection;
class Ray;
} // namespace cvf

class RiuPickItemInfo
{
public:
    RiuPickItemInfo()
        : m_distanceAlongRay( std::numeric_limits<double>::infinity() )
        , m_pickedPart( nullptr )
        , m_globalPickedPoint( cvf::Vec3d::UNDEFINED )
        , m_localPickedPoint( cvf::Vec3d::UNDEFINED )
        , m_globalRayOrigin( cvf::Vec3d::UNDEFINED )
        , m_sourceInfo( nullptr )
        , m_faceIdx( -1 )
    {
    }

    explicit RiuPickItemInfo( const cvf::HitItem* hitItem, const cvf::Vec3d& globalRayOrigin )
        : m_pickedPart( nullptr )
        , m_globalPickedPoint( cvf::Vec3d::UNDEFINED )
        , m_localPickedPoint( cvf::Vec3d::UNDEFINED )
        , m_globalRayOrigin( cvf::Vec3d::UNDEFINED )
        , m_sourceInfo( nullptr )
        , m_faceIdx( -1 )
    {
        *this             = extractPickItemInfo( hitItem );
        m_globalRayOrigin = globalRayOrigin;
    }

    const cvf::Part*   pickedPart() const { return m_pickedPart; }
    cvf::Vec3d         globalPickedPoint() const { return m_globalPickedPoint; }
    cvf::Vec3d         localPickedPoint() const { return m_localPickedPoint; }
    const cvf::Object* sourceInfo() const { return m_sourceInfo; }
    cvf::uint          faceIdx() const { return m_faceIdx; }
    double             distanceAlongRay() const { return m_distanceAlongRay; }
    cvf::Vec3d         globalRayOrigin() const { return m_globalRayOrigin; }

    float polygonOffsetUnit() const;

    bool operator<( const RiuPickItemInfo& other ) const;

    static RiuPickItemInfo              extractPickItemInfo( const cvf::HitItem* hitItem );
    static std::vector<RiuPickItemInfo> convertToPickItemInfos( const cvf::HitItemCollection& hitItems,
                                                                const cvf::Vec3d&             globalRayOrigin,
                                                                double coincidentRayDistanceTolerance = 1e-3 );

private:
    double             m_distanceAlongRay;
    const cvf::Part*   m_pickedPart;
    cvf::Vec3d         m_globalPickedPoint;
    cvf::Vec3d         m_localPickedPoint;
    cvf::Vec3d         m_globalRayOrigin;
    const cvf::Object* m_sourceInfo;
    cvf::uint          m_faceIdx;

    static double sm_rayDistanceTolerance;
};
