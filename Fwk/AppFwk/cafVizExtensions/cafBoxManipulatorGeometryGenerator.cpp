
#include "cafBoxManipulatorGeometryGenerator.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfStructGridGeometryGenerator.h"

using namespace cvf;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BoxManipulatorGeometryGenerator::BoxManipulatorGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BoxManipulatorGeometryGenerator::~BoxManipulatorGeometryGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoxManipulatorGeometryGenerator::setOrigin( const cvf::Vec3d& origin )
{
    m_origin = origin;

    m_vertices = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoxManipulatorGeometryGenerator::setSize( const cvf::Vec3d& size )
{
    m_size = size;

    m_vertices = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> BoxManipulatorGeometryGenerator::createBoundingBoxMeshDrawable()
{
    if ( m_vertices.isNull() )
    {
        calculateArrays();
    }

    if ( !( m_vertices.notNull() && m_vertices->size() != 0 ) ) return nullptr;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray( m_vertices.p() );

    cvf::ref<cvf::UIntArray> indices = cvf::StructGridGeometryGenerator::lineIndicesFromQuadVertexArray( m_vertices.p() );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( indices.p() );

    geo->addPrimitiveSet( prim.p() );

    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoxManipulatorGeometryGenerator::calculateArrays()
{
    BoxGenerator gen;

    cvf::Vec3d min = m_origin;
    cvf::Vec3d max = m_origin + m_size;

    gen.setMinMax( min, max );
    gen.setSubdivisions( 1, 1, 1 );
    GeometryBuilderFaceList builder;
    gen.generate( &builder );

    m_vertices = builder.vertices();

    // TODO: Rotate generated vertices
}

} // namespace caf
