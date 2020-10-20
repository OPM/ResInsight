
#include "cafBoxManipulatorGeometryGenerator.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfPrimitiveSetIndexedUInt.h"

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

    cvf::ref<cvf::UIntArray>               indices = lineIndicesFromQuadVertexArray( m_vertices.p() );
    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim    = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UIntArray> BoxManipulatorGeometryGenerator::lineIndicesFromQuadVertexArray( const cvf::Vec3fArray* vertexArray )
{
    CVF_ASSERT( vertexArray );

    size_t numVertices = vertexArray->size();
    int    numQuads    = static_cast<int>( numVertices / 4 );
    CVF_ASSERT( numVertices % 4 == 0 );

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->resize( numQuads * 8 );

#pragma omp parallel for
    for ( int i = 0; i < numQuads; i++ )
    {
        int idx = 8 * i;
        indices->set( idx + 0, i * 4 + 0 );
        indices->set( idx + 1, i * 4 + 1 );
        indices->set( idx + 2, i * 4 + 1 );
        indices->set( idx + 3, i * 4 + 2 );
        indices->set( idx + 4, i * 4 + 2 );
        indices->set( idx + 5, i * 4 + 3 );
        indices->set( idx + 6, i * 4 + 3 );
        indices->set( idx + 7, i * 4 + 0 );
    }

    return indices;
}

} // namespace caf
