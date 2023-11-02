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
    for ( auto part : allModelParts() )
    {
        m_parts[part]         = RigFRModelPart();
        m_parts[part].texture = new cvf::TextureImage();
        m_parts[part].texture->allocate( 1, 1 );
        m_parts[part].texture->fill( cvf::Color4ub( 0, 0, 0, 0 ) );
        m_parts[part].rect.reserve( 4 );
    }

    // m_cornerIndexes[ModelParts::HiPart1]  = { 2, 3, 7, 6 };
    // m_cornerIndexes[ModelParts::MidPart1] = { 1, 2, 6, 5 };
    // m_cornerIndexes[ModelParts::LowPart1] = { 0, 1, 5, 4 };

    // m_cornerIndexes[ModelParts::HiPart2]  = { 6, 7, 11, 10 };
    // m_cornerIndexes[ModelParts::MidPart2] = { 5, 6, 10, 9 };
    // m_cornerIndexes[ModelParts::LowPart2] = { 4, 5, 9, 8 };

    // for ( auto part : allGridParts() )
    //{
    //     m_3dparts[part] = std::make_shared<RigGriddedPart3d>( part == GridPart::PART2 );
    // }
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
    m_generator->generateGeometry( startCell, startFace );
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
