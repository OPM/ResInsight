#include "Riv2dGridProjectionPartMgr.h"

#include "RiaWeightedMeanCalculator.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivScalarMapperUtils.h"

#include "Rim2dEclipseView.h"
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
Riv2dGridProjectionPartMgr::Riv2dGridProjectionPartMgr(Rim2dGridProjection* gridProjection, Rim2dEclipseView* contourMap)
{
    m_2dGridProjection = gridProjection;
    m_parentContourMap = contourMap;
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
        RivScalarMapperUtils::applyTextureResultsToPart(part.p(), textureCoords.p(), mapper, 1.0f, caf::FC_NONE, true, m_parentContourMap->backgroundColor());

        part->setSourceInfo(new RivMeshLinesSourceInfo(m_2dGridProjection.p()));

        model->addPart(part.p());
    }

    std::vector<cvf::ref<cvf::DrawableGeo>> contourDrawables = createContourPolygons(displayCoordTransform);
    for (cvf::ref<cvf::DrawableGeo> contourDrawable : contourDrawables)
    {
        if (contourDrawable.notNull() && contourDrawable->boundingBox().isValid())
        {
            caf::MeshEffectGenerator meshEffectGen(cvf::Color3::BLACK);
            meshEffectGen.setLineWidth(1.0f);
            meshEffectGen.createAndConfigurePolygonOffsetRenderState(caf::PO_2);
            cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setDrawable(contourDrawable.p());
            part->setEffect(effect.p());
            part->setSourceInfo(new RivMeshLinesSourceInfo(m_2dGridProjection.p()));

            model->addPart(part.p());
        }
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
            if (m_2dGridProjection->hasResultAt(i, j))
            {
                double value = m_2dGridProjection->value(i, j);
                (*textureCoords)[i + j * patchSize.x()] =
                    m_2dGridProjection->legendConfig()->scalarMapper()->mapToTextureCoord(value);
            }
            else
            {
                // Perform weighted averaging of the valid neighbors
                RiaWeightedMeanCalculator<double> calc;
                for (int dj = -1; dj <= 1; ++dj)
                {
                    int fullJ = static_cast<int>(j) + dj;
                    if (fullJ >= 0 && fullJ < static_cast<int>(patchSize.y()))
                    {
                        for (int di = -1; di <= 1; ++di)
                        {
                            if (di == 0 && dj == 0) continue;

                            int fullI = static_cast<int>(i) + di;
                            if (fullI >= 0 && fullI < static_cast<int>(patchSize.x()))
                            {
                                if (m_2dGridProjection->hasResultAt(fullI, fullJ))
                                {
                                    double value = m_2dGridProjection->value(fullI, fullJ);
                                    calc.addValueAndWeight(value, 1.0 / std::sqrt(di*di + dj * dj));
                                }
                            }
                        }
                    }
                }
                if (calc.validAggregatedWeight())
                {
                    (*textureCoords)[i + j * patchSize.x()] =
                        m_2dGridProjection->legendConfig()->scalarMapper()->mapToTextureCoord(calc.weightedMean());
                }
                else
                {
                    (*textureCoords)[i + j * patchSize.x()] = cvf::Vec2f(1.0, 1.0);
                }
            }
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
        bool anyValid = false;
        for (size_t t = 0; !anyValid && t < 3; ++t)
        {
            cvf::uint vertexNumber = (*vertices)[n + t];
            cvf::Vec2ui ij = m_2dGridProjection->ijFromGridIndex(vertexNumber);
            if (m_2dGridProjection->hasResultAt(ij.x(), ij.y()))
            {
                anyValid = true;
            }
        }
        for (size_t t = 0; anyValid && t < 3; ++t)
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<cvf::DrawableGeo>> Riv2dGridProjectionPartMgr::createContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    Rim2dGridProjection::ContourPolygons contourPolygons = m_2dGridProjection->generateContourPolygons(displayCoordTransform);

    std::vector<cvf::ref<cvf::DrawableGeo>> contourDrawables;
    contourDrawables.reserve(contourPolygons.size());
    for (size_t i = 0; i < contourPolygons.size(); ++i)
    {
        cvf::ref<cvf::Vec3fArray> vertexArray = contourPolygons[i];
        std::vector<cvf::uint> indices;
        indices.reserve(contourPolygons[i]->size());
        for (cvf::uint j = 0; j < contourPolygons[i]->size(); ++j)
        {
            indices.push_back(j);
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);
        indexedUInt->setIndices(indexArray.p());

        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

        geo->addPrimitiveSet(indexedUInt.p());
        geo->setVertexArray(vertexArray.p());
        contourDrawables.push_back(geo);
    }
    return contourDrawables;
}
