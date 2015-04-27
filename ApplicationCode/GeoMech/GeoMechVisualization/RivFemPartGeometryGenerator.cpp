#include "RivFemPartGeometryGenerator.h"

#include "cvfBase.h"

#include "RigFemPart.h"
//#include "RigFemPartScalarDataAccess.h"

#include "cvfDebugTimer.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

#include "cvfArray.h"
#include "cvfOutlineEdgeExtractor.h"
#include <cmath>


using namespace cvf;

//==================================================================================================
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFemPartGeometryGenerator::RivFemPartGeometryGenerator(const RigFemPart* part)
:   m_part(part)
{
    CVF_ASSERT(part);
 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFemPartGeometryGenerator::~RivFemPartGeometryGenerator()
{
}


//--------------------------------------------------------------------------------------------------
/// Generate surface drawable geo from the specified region
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::generateSurface()
{
    computeArrays();

    CVF_ASSERT(m_quadVertices.notNull());

    if (m_quadVertices->size() == 0) return NULL;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray(m_quadVertices.p());

    return geo;
}



//--------------------------------------------------------------------------------------------------
/// Generates simplified mesh as line drawing
/// Must call generateSurface first 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::createMeshDrawable()
{
   
    if (!(m_quadVertices.notNull() && m_quadVertices->size() != 0)) return NULL;

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(m_quadVertices.p());
    
    ref<UIntArray> indices = lineIndicesFromQuadVertexArray(m_quadVertices.p());
    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(PT_LINES);
    prim->setIndices(indices.p());

    geo->addPrimitiveSet(prim.p());
    return geo;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> RivFemPartGeometryGenerator::createOutlineMeshDrawable(double creaseAngle)
{
    if (!(m_quadVertices.notNull() && m_quadVertices->size() != 0)) return NULL;

    cvf::OutlineEdgeExtractor ee(creaseAngle, *m_quadVertices);

    ref<UIntArray> indices = lineIndicesFromQuadVertexArray(m_quadVertices.p());
    ee.addPrimitives(4, *indices);

    ref<cvf::UIntArray> lineIndices = ee.lineIndices();
    if (lineIndices->size() == 0)
    {
        return NULL;
    }

    ref<PrimitiveSetIndexedUInt> prim = new PrimitiveSetIndexedUInt(PT_LINES);
    prim->setIndices(lineIndices.p());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(m_quadVertices.p());
    geo->addPrimitiveSet(prim.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
/// 
/// 
/// 
//--------------------------------------------------------------------------------------------------
ref<UIntArray> RivFemPartGeometryGenerator::lineIndicesFromQuadVertexArray(const Vec3fArray* vertexArray)
{
    CVF_ASSERT(vertexArray);

    size_t numVertices = vertexArray->size();
    int numQuads = static_cast<int>(numVertices/4);
    CVF_ASSERT(numVertices%4 == 0);

    ref<UIntArray> indices = new UIntArray;
    indices->resize(numQuads*8);

#pragma omp parallel for
    for (int i = 0; i < numQuads; i++)
    {        
        int idx = 8*i;
        indices->set(idx + 0, i*4 + 0);
        indices->set(idx + 1, i*4 + 1);
        indices->set(idx + 2, i*4 + 1);
        indices->set(idx + 3, i*4 + 2);
        indices->set(idx + 4, i*4 + 2);
        indices->set(idx + 5, i*4 + 3);
        indices->set(idx + 6, i*4 + 3);
        indices->set(idx + 7, i*4 + 0);
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartGeometryGenerator::computeArrays()
{
    std::vector<Vec3f> vertices;

    cvf::Vec3d offset = Vec3d::ZERO; //m_part->displayModelOffset();
    const std::vector<cvf::Vec3f>& nodeCoordinates = m_part->nodes().coordinates;

//#pragma omp parallel for schedule(dynamic)
    for (int elmIdx = 0; elmIdx < static_cast<int>(m_part->elementCount()); elmIdx++)
    {
        if (m_elmVisibility.isNull() || (*m_elmVisibility)[elmIdx])
        {
            RigElementType eType = m_part->elementType(elmIdx);
            int faceCount = elmentFaceCount(eType);
            int elmQuadCount = 0;

            const int* elmNodeIndices =  m_part->connectivities(elmIdx);

            for (int lfIdx = 0; lfIdx < faceCount; ++lfIdx)
            {
                int faceNodeCount = 0;   
                const int*  elmLocalFaceIndices = elementLocalFaceIndices(eType, lfIdx, &faceNodeCount);
                if (faceNodeCount == 4)
                {
                #if 0
                   ++elmQuadCount; 
                   int quad[4];
                   quad[0] = elmNodeIndices[elmLocalFaceIndices[0]];
                   quad[1] = elmNodeIndices[elmLocalFaceIndices[1]];
                   quad[2] = elmNodeIndices[elmLocalFaceIndices[2]];
                   quad[3] = elmNodeIndices[elmLocalFaceIndices[3]];
                #endif
                   // Needs to get rid of opposite faces

                   vertices.push_back(nodeCoordinates[ elmNodeIndices[elmLocalFaceIndices[0]] ]);
                   vertices.push_back(nodeCoordinates[ elmNodeIndices[elmLocalFaceIndices[1]] ]);
                   vertices.push_back(nodeCoordinates[ elmNodeIndices[elmLocalFaceIndices[2]] ]);
                   vertices.push_back(nodeCoordinates[ elmNodeIndices[elmLocalFaceIndices[3]] ]);
                }
                else
                {
                    // Handle triangles and 6 node and 8 node faces
                }

            }
        }

    }

    m_quadVertices = new cvf::Vec3fArray;
    m_quadVertices->assign(vertices);
}



//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimentional texture. 
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivFemPartGeometryGenerator::textureCoordinates(Vec2fArray* textureCoords, const RigFemPartScalarDataAccess* resultAccessor, const ScalarMapper* mapper) const
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartGeometryGenerator::setElementVisibility(const cvf::UByteArray* cellVisibility)
{
    m_elmVisibility = cellVisibility;
}

