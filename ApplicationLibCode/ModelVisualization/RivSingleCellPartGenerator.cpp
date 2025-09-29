/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivSingleCellPartGenerator.h"

#include "RiaPreferencesGrid.h"

#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigGridBase.h"
#include "RigReservoirGridTools.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivPartPriority.h"

#include "cafEffectGenerator.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRenderStateDepth.h"
#include "cvfStructGridGeometryGenerator.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSingleCellPartGenerator::RivSingleCellPartGenerator( RigEclipseCaseData* rigCaseData,
                                                        size_t              gridIndex,
                                                        size_t              cellIndex,
                                                        const cvf::Vec3d&   displayModelOffset )
    : m_rigCaseData( rigCaseData )
    , m_gridIndex( gridIndex )
    , m_cellIndex( cellIndex )
    , m_geoMechCase( nullptr )
    , m_displayModelOffset( displayModelOffset )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSingleCellPartGenerator::RivSingleCellPartGenerator( RimGeoMechCase*   rimGeoMechCase,
                                                        size_t            gridIndex,
                                                        size_t            cellIndex,
                                                        const cvf::Vec3d& displayModelOffset )
    : m_geoMechCase( rimGeoMechCase )
    , m_gridIndex( gridIndex )
    , m_cellIndex( cellIndex )
    , m_rigCaseData( nullptr )
    , m_displayModelOffset( displayModelOffset )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSingleCellPartGenerator::setShowLgrMeshLines( bool enable )
{
    m_showLgrMeshLines = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivSingleCellPartGenerator::createPart( const cvf::Color3f color )
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( cvf::String( "Highlight part for cell index " ) + cvf::String( (cvf::int64)m_cellIndex ) );
    part->setDrawable( createMeshDrawable().p() );

    cvf::ref<cvf::Effect>    eff;
    caf::MeshEffectGenerator effGen( color );
    eff = effGen.generateUnCachedEffect();

    cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
    depth->enableDepthTest( false );
    eff->setRenderState( depth.p() );

    part->setEffect( eff.p() );

    part->setPriority( RivPartPriority::PartType::Highlight );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSingleCellPartGenerator::createMeshLinesOfParentGridCells( RigGridBase const*   grid,
                                                                                         std::vector<size_t>& localGridCellIndices,
                                                                                         const cvf::Vec3d&    displayModelOffset )
{
    if ( !grid ) return nullptr;

    // create a map with parent cell index as key and vector of local grid cell indices as value
    std::map<size_t, std::vector<size_t>> parentCellToLocalGridCellsMap;
    for ( auto cellIndex : localGridCellIndices )
    {
        const auto cell            = grid->cell( cellIndex );
        auto       parentCellIndex = cell.parentCellIndex();
        parentCellToLocalGridCellsMap[parentCellIndex].push_back( cellIndex );
    }

    std::vector<cvf::Vec3d> quadVertices;
    std::vector<cvf::Vec3d> lineVertexPairs;

    for ( const auto& [cellIndexInMainGrid, lgrCellIndices] : parentCellToLocalGridCellsMap )
    {
        for ( auto cellIndex : lgrCellIndices )
        {
            std::array<cvf::Vec3d, 8> cornerVerts = grid->cellCornerVertices( cellIndex );

            cvf::ubyte faceConn[4];

            // Show grid lines for vertical lines along the radius of the cylinder segment
            RigGridBase::cellFaceVertexIndices( cvf::StructGridInterface::NEG_I, faceConn );

            lineVertexPairs.push_back( cornerVerts[faceConn[1]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[2]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[3]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[0]] );

            // Show grid lines for vertical lines along the radius of the cylinder segment
            RigGridBase::cellFaceVertexIndices( cvf::StructGridInterface::POS_I, faceConn );

            lineVertexPairs.push_back( cornerVerts[faceConn[0]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[1]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[2]] );
            lineVertexPairs.push_back( cornerVerts[faceConn[3]] );

            // Show quad lines for min/max J faces
            if ( cellIndex == lgrCellIndices.back() )
            {
                RigGridBase::cellFaceVertexIndices( cvf::StructGridInterface::POS_J, faceConn );

                quadVertices.push_back( cornerVerts[faceConn[0]] );
                quadVertices.push_back( cornerVerts[faceConn[1]] );
                quadVertices.push_back( cornerVerts[faceConn[2]] );
                quadVertices.push_back( cornerVerts[faceConn[3]] );
            }

            if ( cellIndex == lgrCellIndices.front() )
            {
                RigGridBase::cellFaceVertexIndices( cvf::StructGridInterface::NEG_J, faceConn );

                quadVertices.push_back( cornerVerts[faceConn[0]] );
                quadVertices.push_back( cornerVerts[faceConn[1]] );
                quadVertices.push_back( cornerVerts[faceConn[2]] );
                quadVertices.push_back( cornerVerts[faceConn[3]] );
            }
        }
    }

    std::vector<cvf::Vec3f> displayVertices;

    for ( const auto& v : quadVertices )
    {
        displayVertices.push_back( cvf::Vec3f( v - displayModelOffset ) );
    }

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray;
    cvfVertices->assign( displayVertices );

    if ( !cvfVertices.notNull() || cvfVertices->size() == 0 ) return nullptr;

    ref<UIntArray> indices = StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( cvfVertices.p() );

    // Add vertex pairs as lines
    const auto quadVertexCount = cvfVertices->size();
    const auto indicesCount    = indices->size();

    cvfVertices->resize( quadVertexCount + lineVertexPairs.size() );
    indices->resize( indicesCount + lineVertexPairs.size() );

    size_t iIndex = indicesCount;
    size_t vIndex = quadVertexCount;

    for ( const auto& v : lineVertexPairs )
    {
        indices->set( iIndex++, static_cast<cvf::uint>( vIndex ) );
        cvfVertices->set( vIndex++, cvf::Vec3f( v - displayModelOffset ) );
    }

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray( cvfVertices.p() );

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt( PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSingleCellPartGenerator::createMeshDrawable()
{
    if ( m_rigCaseData && m_cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        auto grid = m_rigCaseData->grid( m_gridIndex );
        if ( !grid ) return nullptr;

        if ( m_showLgrMeshLines && RiaPreferencesGrid::current()->radialGridMode() == RiaGridDefines::RadialGridMode::CYLINDRICAL )
        {
            return createMeshDrawableFromLgrGridCells();
        }
        else
        {
            return cvf::StructGridGeometryGenerator::createMeshDrawableFromSingleCell( grid, m_cellIndex, m_displayModelOffset );
        }
    }
    else if ( m_geoMechCase && m_cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        CVF_ASSERT( m_geoMechCase->geoMechData() );
        CVF_ASSERT( m_geoMechCase->geoMechData()->femParts()->partCount() > static_cast<int>( m_gridIndex ) );

        RigFemPart* femPart = m_geoMechCase->geoMechData()->femParts()->part( m_gridIndex );
        CVF_ASSERT( femPart );

        return RivFemPartGeometryGenerator::createMeshDrawableFromSingleElement( femPart, m_cellIndex, m_displayModelOffset );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSingleCellPartGenerator::createMeshDrawableFromLgrGridCells()
{
    if ( !m_rigCaseData ) return nullptr;
    if ( m_cellIndex == cvf::UNDEFINED_SIZE_T ) return nullptr;

    auto mainGrid = m_rigCaseData->mainGrid();
    if ( !mainGrid ) return nullptr;

    if ( mainGrid->gridCount() < 2 ) return nullptr;

    auto lgrGrid = m_rigCaseData->grid( 1 );
    if ( !lgrGrid ) return nullptr;

    std::vector<size_t> lgrCellIndices;
    for ( size_t i = 0; i < lgrGrid->cellCount(); i++ )
    {
        const auto cell            = lgrGrid->cell( i );
        auto       parentCellIndex = cell.parentCellIndex();

        if ( parentCellIndex == m_cellIndex )
        {
            lgrCellIndices.push_back( i );
        }
    }

    return createMeshLinesOfParentGridCells( lgrGrid, lgrCellIndices, m_displayModelOffset );
}
