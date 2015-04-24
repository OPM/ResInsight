
#pragma once

#include "cvfObject.h"
#include "cvfArray.h"

#include "RigFemPart.h"

namespace cvf
{

class DrawableGeo;
class ScalarMapper;

}

class RigFemPartScalarDataAccess;



//==================================================================================================
//
// 
//
//==================================================================================================
class RivFemPartGeometryGenerator : public cvf::Object
{
public:
    RivFemPartGeometryGenerator(const RigFemPart* part);
    ~RivFemPartGeometryGenerator();

    // Setup methods

    void                        setElementVisibility(const cvf::UByteArray* cellVisibility);

    // Access, valid after generation is done

    const RigFemPart*           activePart() { return m_part.p(); }

    void                        textureCoordinates(cvf::Vec2fArray* textureCoords, const RigFemPartScalarDataAccess* resultAccessor, const cvf::ScalarMapper* mapper) const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>  generateSurface();
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();
    cvf::ref<cvf::DrawableGeo>  createOutlineMeshDrawable(double creaseAngle);

private:
    static cvf::ref<cvf::UIntArray> 
                                lineIndicesFromQuadVertexArray(const cvf::Vec3fArray* vertexArray);
    void                        computeArrays();

private:
    // Input
    cvf::cref<RigFemPart>       m_part;                     // The part being processed
    cvf::cref<cvf::UByteArray>       m_elmVisibility;

    // Created arrays
    cvf::ref<cvf::Vec3fArray>   m_quadVertices;
    //cvf::ref<cvf::Vec3fArray>   m_triangleVertices; // If needed, we will do it like this, I think

};


