#include "RivContourMapProjectionPartMgr.h"

#include "RiaWeightedMeanCalculator.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivScalarMapperUtils.h"

#include "RimContourMapView.h"
#include "RimContourMapProjection.h"

#include "cafEffectGenerator.h"

#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapProjectionPartMgr::RivContourMapProjectionPartMgr(RimContourMapProjection* contourMapProjection, RimContourMapView* contourMap)
{
    m_contourMapProjection = contourMapProjection;
    m_parentContourMap = contourMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendProjectionToModel(cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::DrawableGeo> drawable = createProjectionMapDrawable(displayCoordTransform);
    if (drawable.notNull() && drawable->boundingBox().isValid())
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());

        cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords();
        cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();
        RivScalarMapperUtils::applyTextureResultsToPart(part.p(), textureCoords.p(), mapper, 1.0f, caf::FC_NONE, true, m_parentContourMap->backgroundColor());

        part->setSourceInfo(new RivObjectSourceInfo(m_contourMapProjection.p()));

        model->addPart(part.p());
    }

    if (m_contourMapProjection->showContourLines())
    {
        std::vector<cvf::ref<cvf::DrawableGeo>> contourDrawables = createContourPolygons(displayCoordTransform);
        for (cvf::ref<cvf::DrawableGeo> contourDrawable : contourDrawables)
        {
            if (contourDrawable.notNull() && contourDrawable->boundingBox().isValid())
            {
                caf::MeshEffectGenerator meshEffectGen(cvf::Color3::BLACK);
                meshEffectGen.setLineWidth(1.0f);
                meshEffectGen.createAndConfigurePolygonOffsetRenderState(caf::PO_1);
                cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setDrawable(contourDrawable.p());
                part->setEffect(effect.p());
                part->setSourceInfo(new RivMeshLinesSourceInfo(m_contourMapProjection.p()));

                model->addPart(part.p());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendPickPointVisToModel(cvf::ModelBasicList*              model,
                                                               const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::DrawableGeo> drawable = createPickPointVisDrawable(displayCoordTransform);
    if (drawable.notNull() && drawable->boundingBox().isValid())
    {
        caf::MeshEffectGenerator meshEffectGen(cvf::Color3::MAGENTA);
        meshEffectGen.setLineWidth(1.0f);
        meshEffectGen.createAndConfigurePolygonOffsetRenderState(caf::PO_2);
        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(drawable.p());
        part->setEffect(effect.p());
        part->setSourceInfo(new RivMeshLinesSourceInfo(m_contourMapProjection.p()));

        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec2fArray> RivContourMapProjectionPartMgr::createTextureCoords() const
{
    cvf::Vec2ui patchSize = m_contourMapProjection->vertexGridSize();

    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray(m_contourMapProjection->numberOfVertices());

#pragma omp parallel for
    for (int j = 0; j < static_cast<int>(patchSize.y()); ++j)
    {
        for (int i = 0; i < static_cast<int>(patchSize.x()); ++i)
        {
            if (m_contourMapProjection->hasResultAtVertex(i, j))
            {
                double value = m_contourMapProjection->valueAtVertex(i, j);
                cvf::Vec2f textureCoord = m_contourMapProjection->legendConfig()->scalarMapper()->mapToTextureCoord(value);
                textureCoord.y() = 0.0;
                (*textureCoords)[i + j * patchSize.x()] = textureCoord;
            }
            else
            {
                RiaWeightedMeanCalculator<double> calc;
                for (int jj = j - 1; jj <= j + 1; ++jj)
                {
                    for (int ii = i - 1; ii <= i + 1; ++ii)
                    {
                        if (jj >= 0 && ii >= 0 && jj < static_cast<int>(patchSize.y()) && ii < static_cast<int>(patchSize.x()))
                        {
                            if (!(ii == i && jj == j) && m_contourMapProjection->hasResultAtVertex(ii, jj))
                            {
                                double value = m_contourMapProjection->valueAtVertex(ii, jj);
                                calc.addValueAndWeight(value, 1. / std::sqrt((i - ii)*(i - ii) + (j - jj)*(j - jj)));
                            }
                        }
                    }
                }
                if (calc.validAggregatedWeight())
                {
                    const double maxTheoreticalWeightSum = 4.0 + 4.0 / std::sqrt(2.0);
                    double value                      = calc.weightedMean();
                    cvf::Vec2f textureCoord = m_contourMapProjection->legendConfig()->scalarMapper()->mapToTextureCoord(value);
                    textureCoord.y() = 1.0 - calc.aggregatedWeight() / maxTheoreticalWeightSum;
                    (*textureCoords)[i + j * patchSize.x()] = textureCoord;
                }
                else
                {
                    (*textureCoords)[i + j * patchSize.x()] = cvf::Vec2f(0.0, 1.0);
                }
            }
        }
    }
    return textureCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivContourMapProjectionPartMgr::createProjectionMapDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray;
    m_contourMapProjection->generateVertices(vertexArray.p(), displayCoordTransform);
    cvf::Vec2ui patchSize = m_contourMapProjection->vertexGridSize();

    // Surface
    cvf::ref<cvf::UIntArray> faceList = new cvf::UIntArray;
    cvf::GeometryUtils::tesselatePatchAsTriangles(patchSize.x(), patchSize.y(), 0u, true, faceList.p());    

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLES, faceList.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet(indexUInt.p());
    geo->setVertexArray(vertexArray.p());
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<cvf::DrawableGeo>> RivContourMapProjectionPartMgr::createContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    RimContourMapProjection::ContourPolygons contourPolygons = m_contourMapProjection->generateContourPolygons(displayCoordTransform);

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivContourMapProjectionPartMgr::createPickPointVisDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    cvf::ref<cvf::DrawableGeo> geo = nullptr;

    cvf::ref<cvf::Vec3fArray> pickPointPolygon = m_contourMapProjection->generatePickPointPolygon(displayCoordTransform);
    if (pickPointPolygon.notNull() && pickPointPolygon->size() > 0u)
    {
        std::vector<cvf::uint>    indices;
        indices.reserve(pickPointPolygon->size());
        for (cvf::uint j = 0; j < pickPointPolygon->size(); ++j)
        {
            indices.push_back(j);
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);
        indexedUInt->setIndices(indexArray.p());

        geo = new cvf::DrawableGeo;

        geo->addPrimitiveSet(indexedUInt.p());
        geo->setVertexArray(pickPointPolygon.p());
    }
    return geo;
}
