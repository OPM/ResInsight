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

#include "RigFaultReactivationModelGenerator.h"
#include "RigGriddedPart3d.h"
#include "RigPolyLinesData.h"

#include "RimFaultReactivationDataAccess.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModel::RigFaultReactivationModel()
    : m_isValid( false )
{
    for ( int part = 0; part < numModelParts(); part++ )
    {
        m_parts[part]         = RigFRModelPart();
        m_parts[part].texture = new cvf::TextureImage();
        m_parts[part].texture->allocate( 1, 1 );
        m_parts[part].texture->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
        m_parts[part].rect.reserve( 4 );
    }

    m_cornerIndexes[0] = { 0, 1, 7, 6 };
    m_cornerIndexes[1] = { 1, 2, 8, 7 };
    m_cornerIndexes[2] = { 2, 3, 9, 8 };
    m_cornerIndexes[3] = { 3, 4, 10, 9 };
    m_cornerIndexes[4] = { 4, 5, 11, 10 };

    for ( auto part : allGridParts() )
    {
        m_3dparts[part] = new RigGriddedPart3d();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModel::~RigFaultReactivationModel()
{
    for ( auto part : allGridParts() )
    {
        if ( m_3dparts[part] != nullptr ) delete m_3dparts[part];
        m_3dparts[part] = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFaultReactivation::GridPart> RigFaultReactivationModel::allGridParts() const
{
    return { GridPart::FW, GridPart::HW };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::reset()
{
    m_isValid = false;

    for ( int part = 0; part < numModelParts(); part++ )
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
void RigFaultReactivationModel::setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color )
{
    const int oneSidedParts = numModelParts() / 2;

    for ( int part = 0; part < oneSidedParts; part++ )
    {
        m_parts[part].texture->fill( cvf::Color4ub( part1Color.rByte(), part1Color.gByte(), part1Color.bByte(), 255 ) );
    }

    for ( int part = oneSidedParts; part < numModelParts(); part++ )
    {
        m_parts[part].texture->fill( cvf::Color4ub( part2Color.rByte(), part2Color.gByte(), part2Color.bByte(), 255 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::setGenerator( std::shared_ptr<RigFaultReactivationModelGenerator> generator )
{
    m_generator = generator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3d, cvf::Vec3d> RigFaultReactivationModel::modelLocalNormalsXY() const
{
    if ( m_generator.get() == nullptr )
    {
        return std::make_pair( cvf::Vec3d( 1.0, 0.0, 0.0 ), cvf::Vec3d( 0.0, 1.0, 0.0 ) );
    }
    return m_generator->modelLocalNormalsXY();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModel::updateGeometry( size_t startCell, cvf::StructGridInterface::FaceType startFace )
{
    reset();

    auto frontPart = m_3dparts[GridPart::FW];
    auto backPart  = m_3dparts[GridPart::HW];

    m_generator->generateGeometry( startCell, startFace, frontPart, backPart );

    if ( backPart->topHeight() > frontPart->topHeight() )
    {
        m_3dparts[GridPart::HW] = frontPart;
        m_3dparts[GridPart::FW] = backPart;
    }

    auto& frontPoints = m_generator->frontPoints();
    auto& backPoints  = m_generator->backPoints();

    const int oneSideParts = numModelParts() / 2;

    for ( int part = 0; part < oneSideParts; part++ )
    {
        for ( auto i : m_cornerIndexes[part] )
        {
            m_parts[part].rect.push_back( frontPoints[i] );
        }
    }
    for ( int part = 0; part < oneSideParts; part++ )
    {
        for ( auto i : m_cornerIndexes[part] )
        {
            m_parts[part + oneSideParts].rect.push_back( backPoints[i] );
        }
    }

    m_isValid = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigFaultReactivationModel::rect( int part ) const
{
    return m_parts.at( part ).rect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> RigFaultReactivationModel::texture( int part ) const
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
const RigGriddedPart3d* RigFaultReactivationModel::grid( RimFaultReactivation::GridPart part ) const
{
    return m_3dparts.at( part );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d RigFaultReactivationModel::faultNormal() const
{
    if ( m_generator.get() == nullptr ) return { 0.0, 0.0, 0.0 };
    return m_generator->normal();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::pair<cvf::Vec3d, cvf::Vec3d> RigFaultReactivationModel::faultTopBottom() const
{
    if ( m_generator.get() == nullptr ) return std::make_pair( cvf::Vec3d(), cvf::Vec3d() );
    return m_generator->faultTopBottomPoints();
}
