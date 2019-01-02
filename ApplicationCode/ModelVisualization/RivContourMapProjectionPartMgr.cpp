#include "RivContourMapProjectionPartMgr.h"

#include "RiaColorTools.h"
#include "RiaFontCache.h"
#include "RiaWeightedMeanCalculator.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivScalarMapperUtils.h"

#include "RimContourMapView.h"
#include "RimContourMapProjection.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapper.h"

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
    cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();

    std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels = createContourPolygons(displayCoordTransform);
    cvf::ref<cvf::Part> mapPart = createProjectionMapPart(displayCoordTransform);
    if (mapPart.notNull())
    {     
        model->addPart(mapPart.p());
    }

    if (m_contourMapProjection->showContourLines())
    {
        std::vector<double> tickValues;
        mapper->majorTickValues(&tickValues);

        for (size_t i = 0; i < contourDrawablesForAllLevels.size(); ++i)
        {
            std::vector<cvf::ref<cvf::Drawable>> contourDrawables = contourDrawablesForAllLevels[i];

            cvf::Color3f                         backgroundColor(mapper->mapToColor(tickValues[i]));
            cvf::Color3f                         lineColor = RiaColorTools::contrastColor(backgroundColor, true);

            for (cvf::ref<cvf::Drawable> contourDrawable : contourDrawables)
            {
                if (contourDrawable.notNull() && contourDrawable->boundingBox().isValid())
                {
                    caf::MeshEffectGenerator meshEffectGen(lineColor);
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
cvf::ref<cvf::Vec2fArray> RivContourMapProjectionPartMgr::createTextureCoords(const std::vector<double>& values) const
{
    cvf::Vec2ui patchSize = m_contourMapProjection->numberOfVerticesIJ();

    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray(values.size());

#pragma omp parallel for
    for (int i = 0; i < (int) values.size(); ++i)
    {
        if (values[i] != std::numeric_limits<double>::infinity())
        {
            cvf::Vec2f textureCoord = m_contourMapProjection->legendConfig()->scalarMapper()->mapToTextureCoord(values[i]);
            textureCoord.y()        = 0.0;
            (*textureCoords)[i] = textureCoord;
        }
        else
        {
            (*textureCoords)[i] = cvf::Vec2f(0.0, 1.0);
        }
    }
    return textureCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableText> RivContourMapProjectionPartMgr::createTextLabel(const cvf::Color3f& textColor, const cvf::Color3f& backgroundColor)
{
    auto font = RiaFontCache::getFont(RiaFontCache::FONT_SIZE_10);

    cvf::ref<cvf::DrawableText> labelDrawable = new cvf::DrawableText();
    labelDrawable->setFont(font.p());
    labelDrawable->setCheckPosVisible(true);
    labelDrawable->setUseDepthBuffer(true);
    labelDrawable->setDrawBorder(false);
    labelDrawable->setDrawBackground(false);
    labelDrawable->setBackgroundColor(backgroundColor);
    labelDrawable->setVerticalAlignment(cvf::TextDrawer::BASELINE);
    labelDrawable->setTextColor(textColor);

    return labelDrawable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivContourMapProjectionPartMgr::createProjectionMapPart(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    std::vector<cvf::Vec4d> vertices = m_contourMapProjection->generateTriangles();
    if (vertices.size() < 3u)
    {
        return cvf::ref<cvf::Part>();
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(vertices.size());
    cvf::ref<cvf::UIntArray> faceList = new cvf::UIntArray(vertices.size());
    std::vector<double> values(vertices.size());
    for (uint i = 0; i < vertices.size(); ++i)
    {
        cvf::Vec3d globalVertex = cvf::Vec3d(vertices[i].x(), vertices[i].y(), vertices[i].z()) + m_contourMapProjection->origin3d();
        cvf::Vec3f displayVertexPos (displayCoordTransform->transformToDisplayCoord(globalVertex));
        (*vertexArray)[i] = displayVertexPos;
        (*faceList)[i] = i;
        values[i] = vertices[i].w();
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLES, faceList.p());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet(indexUInt.p());
    geo->setVertexArray(vertexArray.p());

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(geo.p());

    cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();

    cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords(values);
    RivScalarMapperUtils::applyTextureResultsToPart(
        part.p(), textureCoords.p(), mapper, 1.0f, caf::FC_NONE, true, m_parentContourMap->backgroundColor());

    part->setSourceInfo(new RivObjectSourceInfo(m_contourMapProjection.p()));

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::ref<cvf::Drawable>>> RivContourMapProjectionPartMgr::createContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    m_contourMapProjection->generateContourPolygons();
    const std::vector<RimContourMapProjection::ContourPolygons>& contourPolygons = m_contourMapProjection->contourPolygons();

    const cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();
    std::vector<double> tickValues;
    mapper->majorTickValues(&tickValues);

    std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels;
    std::vector<cvf::ref<cvf::Drawable>> labelDrawables;
    for (int64_t i = (int64_t) contourPolygons.size() - 1; i > 0; --i)
    {
        std::vector<cvf::ref<cvf::Drawable>> contourDrawables;

        cvf::Color3f backgroundColor(mapper->mapToColor(tickValues[i]));
        cvf::Color3f textColor = RiaColorTools::contrastColor(backgroundColor);

        for (size_t j = 0; j < contourPolygons[i].size(); ++j)
        {
            if (contourPolygons[i][j].vertices.empty()) continue;

            size_t nVertices = contourPolygons[i][j].vertices.size();
            size_t nLabels = m_contourMapProjection->showContourLabels() ? std::max((size_t)1, nVertices / 100u) : 0u;
            for (size_t l = 0; l < nLabels; ++l)
            {
                cvf::ref<cvf::DrawableText> label = createTextLabel(textColor, backgroundColor);
                cvf::Vec3d globalVertex = contourPolygons[i][j].vertices[(nVertices * l) / nLabels] + m_contourMapProjection->origin3d();
                cvf::Vec3f displayVertex(displayCoordTransform->transformToDisplayCoord(globalVertex));
                displayVertex.z() += 3.0f;
                label->addText(cvf::String(contourPolygons[i][j].value), displayVertex);
                bool overlaps = false;
                cvf::BoundingBox bbox = label->boundingBox();
                for (cvf::ref<cvf::Drawable> existingLabel : labelDrawables)
                {
                    if (existingLabel->boundingBox().intersects(bbox))
                    {
                        overlaps = true;
                        break;
                    }
                }
                if (!overlaps)
                {
                    labelDrawables.push_back(label);
                }
            }
            cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(nVertices);
            for (size_t v = 0; v < nVertices; ++v)
            {
                cvf::Vec3d globalVertex = contourPolygons[i][j].vertices[v] + m_contourMapProjection->origin3d();
                cvf::Vec3d displayVertex1 = displayCoordTransform->transformToDisplayCoord(globalVertex);
                (*vertexArray)[v]     = cvf::Vec3f(displayVertex1);
            }

            std::vector<cvf::uint> indices;
            indices.reserve(vertexArray->size());
            for (cvf::uint k = 0; k < vertexArray->size(); ++k)
            {
                indices.push_back(k);
            }

            cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINE_LOOP);
            cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);
            indexedUInt->setIndices(indexArray.p());

            cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

            geo->addPrimitiveSet(indexedUInt.p());
            geo->setVertexArray(vertexArray.p());
            contourDrawables.push_back(geo);
        }
        for (cvf::ref<cvf::Drawable> labelDrawable : labelDrawables)
        {
            contourDrawables.push_back(labelDrawable);
        }

        contourDrawablesForAllLevels.push_back(contourDrawables);
    }

    return contourDrawablesForAllLevels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivContourMapProjectionPartMgr::createPickPointVisDrawable(const caf::DisplayCoordTransform* displayCoordTransform) const
{
    std::vector<cvf::Vec3d> pickPointPolygon = m_contourMapProjection->generatePickPointPolygon();
    if (pickPointPolygon.empty())
    {
        return nullptr;
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(pickPointPolygon.size());

    for (size_t i = 0; i < pickPointPolygon.size(); ++i)
    {
        cvf::Vec3d globalPoint = pickPointPolygon[i] + m_contourMapProjection->origin3d();
        cvf::Vec3f displayPoint(displayCoordTransform->transformToDisplayCoord(globalPoint));
        (*vertexArray)[i] = displayPoint;
    }

    cvf::ref<cvf::DrawableGeo> geo = nullptr;
    if (vertexArray->size() > 0u)
    {
        std::vector<cvf::uint>    indices;
        indices.reserve(vertexArray->size());
        for (cvf::uint j = 0; j < vertexArray->size(); ++j)
        {
            indices.push_back(j);
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);
        indexedUInt->setIndices(indexArray.p());

        geo = new cvf::DrawableGeo;

        geo->addPrimitiveSet(indexedUInt.p());
        geo->setVertexArray(vertexArray.p());
    }
    return geo;
}
