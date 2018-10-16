#include "Riv2dGridProjectionPartMgr.h"

#include "RivMeshLinesSourceInfo.h"
#include "RivScalarMapperUtils.h"

#include "Rim2dGridProjection.h"

#include "cafEffectGenerator.h"

#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv2dGridProjectionPartMgr::Riv2dGridProjectionPartMgr(Rim2dGridProjection* gridProjection)
{
    m_2dGridProjection = gridProjection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv2dGridProjectionPartMgr::appendProjectionToModel(cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::DrawableGeo> drawable = createDrawable(displayCoordTransform);
    if (drawable.notNull() && drawable->boundingBox().isValid())
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());

        cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords();
        cvf::ScalarMapper* mapper = m_2dGridProjection->legendConfig()->scalarMapper();
        RivScalarMapperUtils::applyTextureResultsToPart(part.p(), textureCoords.p(), mapper, 1.0, caf::FC_NONE, true);

        part->setSourceInfo(new RivMeshLinesSourceInfo(m_2dGridProjection.p()));

        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec2fArray> Riv2dGridProjectionPartMgr::createTextureCoords() const
{
    cvf::Vec2ui patchSize = m_2dGridProjection->surfaceGridSize();

    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray(m_2dGridProjection->vertexCount());

    for (uint j = 0; j < patchSize.y(); ++j)
    {
        for (uint i = 0; i < patchSize.x(); ++i)
        {
            double value = m_2dGridProjection->value(i, j);
            (*textureCoords)[i + j * patchSize.x()] =
                m_2dGridProjection->legendConfig()->scalarMapper()->mapToTextureCoord(value);
        }
    }
    return textureCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv2dGridProjectionPartMgr::removeTrianglesWithNoResult(cvf::UIntArray* vertices) const
{
    std::vector<cvf::uint> trianglesWithResult;
    
    for (size_t n = 0; n < vertices->size(); n += 3)
    {
        bool anyInvalid = false;
        for (size_t t = 0; !anyInvalid && t < 3; ++t)
        {
            cvf::uint vertexNumber = (*vertices)[n + t];
            cvf::Vec2ui ij = m_2dGridProjection->ijFromGridIndex(vertexNumber);
            if (!m_2dGridProjection->hasResultAt(ij.x(), ij.y()))
            {
                anyInvalid = true;
            }
        }
        for (size_t t = 0; !anyInvalid && t < 3; ++t)
        {
            cvf::uint vertexNumber = (*vertices)[n + t];
            trianglesWithResult.push_back(vertexNumber);
        }        
    }
    (*vertices) = cvf::UIntArray(trianglesWithResult);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv2dGridProjectionPartMgr::createDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    m_2dGridProjection->updateDefaultSampleSpacingFromGrid();
    m_2dGridProjection->extractGridData();

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray;
    m_2dGridProjection->generateVertices(vertexArray.p(), displayCoordTransform);
    cvf::Vec2ui patchSize = m_2dGridProjection->surfaceGridSize();

    // Surface
    cvf::ref<cvf::UIntArray> faceList = new cvf::UIntArray;
    cvf::GeometryUtils::tesselatePatchAsTriangles(patchSize.x(), patchSize.y(), 0u, true, faceList.p());    

    removeTrianglesWithNoResult(faceList.p());

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLES, faceList.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet(indexUInt.p());
    geo->setVertexArray(vertexArray.p());
    return geo;
}
