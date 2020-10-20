
#pragma once

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf
{
class DrawableGeo;
}

namespace caf
{
//==================================================================================================
//
//
//==================================================================================================
class BoxManipulatorGeometryGenerator : public cvf::Object
{
public:
    BoxManipulatorGeometryGenerator();
    ~BoxManipulatorGeometryGenerator() override;

    void setOrigin( const cvf::Vec3d& origin );
    void setSize( const cvf::Vec3d& size );

    cvf::ref<cvf::DrawableGeo> createBoundingBoxMeshDrawable();

private:
    void calculateArrays();

    static cvf::ref<cvf::UIntArray> lineIndicesFromQuadVertexArray( const cvf::Vec3fArray* vertexArray );

private:
    cvf::Vec3d m_origin;
    cvf::Vec3d m_size;

    cvf::ref<cvf::Vec3fArray> m_vertices;
};

} // namespace caf
