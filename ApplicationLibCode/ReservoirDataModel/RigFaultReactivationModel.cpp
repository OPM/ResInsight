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

#include "RigGriddedPart3d.h"
#include "RigPolyLinesData.h"

#include "RimFaultReactivationDataAccess.h"

#include "cafAssert.h"

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
    , m_thickness( 1.0 )

{
    for ( auto part : allModelParts() )
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

    for ( auto part : allGridParts() )
    {
        m_3dparts[part] = std::make_shared<RigGriddedPart3d>( part == GridPart::PART2 );

        m_cellIndexAdjustmentMap[part] = {};
    }
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
std::vector<RimFaultReactivation::ModelParts> RigFaultReactivationModel::allModelParts() const
{
    return { ModelParts::HiPart1, ModelParts::MidPart1, ModelParts::LowPart1, ModelParts::HiPart2, ModelParts::MidPart2, ModelParts::LowPart2 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFaultReactivation::GridPart> RigFaultReactivationModel::allGridParts() const
{
    return { GridPart::PART1, GridPart::PART2 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::reset()
{
    m_isValid = false;
    for ( auto part : allModelParts() )
    {
        m_parts[part].rect.clear();
        m_parts[part].rect.reserve( 4 );
    }

    for ( auto part : allGridParts() )
    {
        m_3dparts[part]->reset();
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

    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setThickness( double thickness )
{
    m_thickness = thickness;

    reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setLocalCoordTransformation( cvf::Mat4d transform )
{
    m_localCoordTransform = transform;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setUseLocalCoordinates( bool useLocalCoordinates )
{
    m_3dparts[GridPart::PART1]->setUseLocalCoordinates( useLocalCoordinates );
    m_3dparts[GridPart::PART2]->setUseLocalCoordinates( useLocalCoordinates );
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
void RigFaultReactivationModel::updateGeometry()
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

    for ( auto part : allModelParts() )
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
cvf::Vec3d RigFaultReactivationModel::normal() const
{
    return m_planeNormal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigFaultReactivationModel::rect( RimFaultReactivation::ModelParts part ) const
{
    return m_parts.at( part ).rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> RigFaultReactivationModel::texture( RimFaultReactivation::ModelParts part ) const
{
    return m_parts.at( part ).texture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RigFaultReactivationModel::meshLines( RimFaultReactivation::GridPart part ) const
{
    return m_3dparts.at( part )->meshLines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::generateGrids( cvf::Vec3dArray points )
{
    m_3dparts[GridPart::PART1]->generateGeometry( { points[0], points[1], points[2], points[3], points[4], points[5], points[6], points[7] },
                                                  m_cellCountHorzPart1,
                                                  m_cellCountVertLower,
                                                  m_cellCountVertMiddle,
                                                  m_cellCountVertUpper,
                                                  m_thickness );
    m_3dparts[GridPart::PART2]->generateGeometry( { points[8], points[9], points[10], points[11], points[4], points[5], points[6], points[7] },
                                                  m_cellCountHorzPart2,
                                                  m_cellCountVertLower,
                                                  m_cellCountVertMiddle,
                                                  m_cellCountVertUpper,
                                                  m_thickness );

    m_3dparts[GridPart::PART1]->generateLocalNodes( m_localCoordTransform );
    m_3dparts[GridPart::PART2]->generateLocalNodes( m_localCoordTransform );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigGriddedPart3d> RigFaultReactivationModel::grid( RimFaultReactivation::GridPart part ) const
{
    return m_3dparts.at( part );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::generateElementSets( const RimFaultReactivationDataAccess* dataAccess, const RigMainGrid* grid )
{
    for ( auto part : allGridParts() )
    {
        m_3dparts[part]->generateElementSets( dataAccess, grid );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::generateCellIndexMapping( const RigMainGrid* grid )
{
    m_cellIndexAdjustmentMap.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, size_t> RigFaultReactivationModel::cellIndexAdjustment( GridPart part ) const
{
    auto it = m_cellIndexAdjustmentMap.find( part );
    if ( it != m_cellIndexAdjustmentMap.end() )
        return it->second;
    else
        return {};
}
