/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RigFaultReactivationModel.h"

#include "RigGriddedPlane.h"
#include "RigPolyLinesData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModel::RigFaultReactivationModel()
    : m_maxZ( 0 )
    , m_minZ( 0 )
    , m_maxHorzExtent( 0 )
    , m_isValid( false )
    , m_cellCountHorzPart1( 1 )
    , m_cellCountHorzPart2( 1 )
    , m_cellCountVertUpper( 1 )
    , m_cellCountVertMiddle( 1 )
    , m_cellCountVertLower( 1 )

{
    for ( auto part : allParts() )
    {
        m_parts[part]         = RigFRModelPart();
        m_parts[part].texture = new cvf::TextureImage();
        m_parts[part].texture->allocate( 1, 1 );
        m_parts[part].texture->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
        m_parts[part].rect.reserve( 4 );
    }

    m_cornerIndexes[ModelParts::HiPart1]  = { 2, 3, 7, 6 };
    m_cornerIndexes[ModelParts::MidPart1] = { 1, 2, 6, 5 };
    m_cornerIndexes[ModelParts::LowPart1] = { 0, 1, 5, 4 };

    m_cornerIndexes[ModelParts::HiPart2]  = { 6, 7, 11, 10 };
    m_cornerIndexes[ModelParts::MidPart2] = { 5, 6, 10, 9 };
    m_cornerIndexes[ModelParts::LowPart2] = { 4, 5, 9, 8 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModel::~RigFaultReactivationModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFaultReactivationModel::ModelParts> RigFaultReactivationModel::allParts() const
{
    return { ModelParts::HiPart1, ModelParts::MidPart1, ModelParts::LowPart1, ModelParts::HiPart2, ModelParts::MidPart2, ModelParts::LowPart2 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::reset()
{
    m_isValid = false;
    for ( auto part : allParts() )
    {
        m_parts[part].rect.clear();
        m_parts[part].rect.reserve( 4 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFaultReactivationModel::isValid() const
{
    return m_isValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setPlane( cvf::Vec3d anchorPoint, cvf::Vec3d normal )
{
    m_planeAnchor = anchorPoint;
    m_planeNormal = normal;
    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color )
{
    for ( auto part : { ModelParts::HiPart1, ModelParts::MidPart1, ModelParts::LowPart1 } )
    {
        m_parts[part].texture->fill( cvf::Color4ub( part1Color.rByte(), part1Color.gByte(), part1Color.bByte(), 255 ) );
    }

    for ( auto part : { ModelParts::HiPart2, ModelParts::MidPart2, ModelParts::LowPart2 } )
    {
        m_parts[part].texture->fill( cvf::Color4ub( part2Color.rByte(), part2Color.gByte(), part2Color.bByte(), 255 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setMaxExtentFromAnchor( double maxExtentHorz, double minZ, double maxZ )
{
    m_maxHorzExtent = maxExtentHorz;
    m_minZ          = minZ;
    m_maxZ          = maxZ;
    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setFaultPlaneIntersect( cvf::Vec3d faultPlaneTop, cvf::Vec3d faultPlaneBottom )
{
    m_faultPlaneIntersectBottom = faultPlaneBottom;
    m_faultPlaneIntersectTop    = faultPlaneTop;

    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setCellCounts( int horzPart1, int horzPart2, int vertUpper, int vertMiddle, int vertLower )
{
    m_cellCountHorzPart1  = horzPart1;
    m_cellCountHorzPart2  = horzPart2;
    m_cellCountVertUpper  = vertUpper;
    m_cellCountVertMiddle = vertMiddle;
    m_cellCountVertLower  = vertLower;
}

//--------------------------------------------------------------------------------------------------
///                  7
///       3----------|----------- 11
///        |         |          |
///        |         |          |
///        |         |          |
///       2|---------|----------| 10
///        |         \6         |
///        |          X Anchor  |
///        |           \        |
///       1-------------|------- 9
///        |           5|       |
///        |            |       |
///        |            |       |
///        |            |       |
///       0-------------|-------- 8
///                     4
///
///
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::updateRects()
{
    reset();

    if ( ( m_maxHorzExtent <= 0.0 ) || ( m_minZ == m_maxZ ) )
    {
        return;
    }

    cvf::Vec3d zDir( 0, 0, 1 );

    auto alongPlane = m_planeNormal ^ zDir;
    alongPlane.normalize();

    // how far from anchor point we should stop
    const double extHorz = m_maxHorzExtent / 2.0;
    auto         mr      = m_planeAnchor + alongPlane * extHorz;
    auto         ml      = m_planeAnchor - alongPlane * extHorz;

    cvf::Vec3dArray points;
    points.resize( 12 );

    points[0]     = ml;
    points[0].z() = -m_maxZ;

    points[1]     = ml;
    points[1].z() = m_faultPlaneIntersectBottom.z();

    points[2]     = ml;
    points[2].z() = m_faultPlaneIntersectTop.z();

    points[3]     = ml;
    points[3].z() = -m_minZ;

    points[4]     = m_faultPlaneIntersectBottom;
    points[4].z() = -m_maxZ;

    points[5] = m_faultPlaneIntersectBottom;
    points[6] = m_faultPlaneIntersectTop;

    points[7]     = m_faultPlaneIntersectTop;
    points[7].z() = -m_minZ;

    points[8]     = mr;
    points[8].z() = -m_maxZ;

    points[9]     = mr;
    points[9].z() = m_faultPlaneIntersectBottom.z();

    points[10]     = mr;
    points[10].z() = m_faultPlaneIntersectTop.z();

    points[11]     = mr;
    points[11].z() = -m_minZ;

    for ( auto part : allParts() )
    {
        for ( auto i : m_cornerIndexes[part] )
        {
            m_parts[part].rect.push_back( points[i] );
        }
    }

    m_isValid = true;

    generateGrids( points );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigFaultReactivationModel::rect( ModelParts part ) const
{
    return m_parts.at( part ).rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> RigFaultReactivationModel::texture( ModelParts part ) const
{
    return m_parts.at( part ).texture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RigFaultReactivationModel::meshLines( ModelParts part ) const
{
    return m_gridParts.at( part )->meshLines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RigFaultReactivationModel::cellCountHorzVert( ModelParts part ) const
{
    switch ( part )
    {
        case RigFaultReactivationModel::ModelParts::HiPart1:
            return std::make_pair( m_cellCountHorzPart1, m_cellCountVertUpper );
        case RigFaultReactivationModel::ModelParts::MidPart1:
            return std::make_pair( m_cellCountHorzPart1, m_cellCountVertMiddle );
        case RigFaultReactivationModel::ModelParts::LowPart1:
            return std::make_pair( m_cellCountHorzPart1, m_cellCountVertLower );

        case RigFaultReactivationModel::ModelParts::HiPart2:
            return std::make_pair( m_cellCountHorzPart2, m_cellCountVertUpper );
        case RigFaultReactivationModel::ModelParts::MidPart2:
            return std::make_pair( m_cellCountHorzPart2, m_cellCountVertMiddle );
        case RigFaultReactivationModel::ModelParts::LowPart2:
            return std::make_pair( m_cellCountHorzPart2, m_cellCountVertLower );
        default:
            break;
    }
    return std::make_pair( 1, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::generateGrids( cvf::Vec3dArray points )
{
    for ( auto part : allParts() )
    {
        auto [cellsHorz, cellsVert] = cellCountHorzVert( part );

        m_gridParts[part] = std::make_unique<RigGriddedPlane>();
        m_gridParts[part]->generateGeometry( m_parts[part].rect, cellsHorz, cellsVert );
    }
}
