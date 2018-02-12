/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RivTensorResultPartMgr.h"

#include "RiaApplication.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimTensorResults.h"

#include "RiuViewer.h"

#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemTypes.h"
#include "RigGeoMechCaseData.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechVizLogic.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafTensor3.h"

#include "cvfArrowGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableVectors.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfObject.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfPart.h"
#include "cvfShaderProgram.h"
#include "cvfStructGridGeometryGenerator.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTensorResultPartMgr::RivTensorResultPartMgr(RimGeoMechView* reservoirView)
{
    m_rimReservoirView = reservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTensorResultPartMgr::~RivTensorResultPartMgr() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex) const
{
    if (m_rimReservoirView.isNull()) return;
    if (!m_rimReservoirView->geoMechCase()) return;
    if (!m_rimReservoirView->geoMechCase()->geoMechData()) return;

    if (!m_rimReservoirView->tensorResults()->showTensors()) return;

    RigFemPartCollection* femParts = m_rimReservoirView->geoMechCase()->geoMechData()->femParts();
    if (!femParts) return;

    std::vector<TensorVisualization> tensorVisualizations;

    RimTensorResults::TensorColors tensorColor = m_rimReservoirView->tensorResults()->vectorColors();
    cvf::Color3f                   color1, color2, color3;

    assignColorVectors(tensorColor, &color1, &color2, &color3);

    RigFemResultAddress address = m_rimReservoirView->tensorResults()->selectedTensorResult();
    if (!isTensorAddress(address)) return;

    RigFemPartResultsCollection* resultCollection = m_rimReservoirView->geoMechCase()->geoMechData()->femPartResults();
    if (!resultCollection) return;

    for (int partIdx = 0; partIdx < femParts->partCount(); partIdx++)
    {
        std::vector<caf::Ten3f> tensors = resultCollection->tensors(address, partIdx, (int)frameIndex);

        const RigFemPart*       part     = femParts->part(partIdx);
        size_t                  elmCount = part->elementCount();
        std::vector<caf::Ten3f> elmTensors;
        elmTensors.resize(elmCount);

        for (int elmIdx = 0; elmIdx < elmCount; elmIdx++)
        {
            if (RigFemTypes::elmentNodeCount(part->elementType(elmIdx)) == 8)
            {
                caf::Ten3f tensorSumOfElmNodes = tensors[part->elementNodeResultIdx(elmIdx, 0)];
                for (int i = 1; i < 8; i++)
                {
                    tensorSumOfElmNodes = tensorSumOfElmNodes + tensors[part->elementNodeResultIdx(elmIdx, i)];
                }

                elmTensors[elmIdx] = tensorSumOfElmNodes * (1.0 / 8.0);
            }
        }

        std::array<std::vector<float>, 3>      elmPrincipals;
        std::vector<std::array<cvf::Vec3f, 3>> elmPrincipalDirections;

        elmPrincipals[0].resize(elmCount);
        elmPrincipals[1].resize(elmCount);
        elmPrincipals[2].resize(elmCount);

        elmPrincipalDirections.resize(elmCount);

        for (size_t nIdx = 0; nIdx < elmCount; ++nIdx)
        {
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principalValues = elmTensors[nIdx].calculatePrincipals(principalDirs);

            elmPrincipals[0][nIdx] = principalValues[0];
            elmPrincipals[1][nIdx] = principalValues[1];
            elmPrincipals[2][nIdx] = principalValues[2];

            elmPrincipalDirections[nIdx][0] = principalDirs[0];
            elmPrincipalDirections[nIdx][1] = principalDirs[1];
            elmPrincipalDirections[nIdx][2] = principalDirs[2];
        }

        std::vector<RivGeoMechPartMgrCache::Key> partKeys =
            m_rimReservoirView->vizLogic()->keysToVisiblePartMgrs((int)frameIndex);

        RigFemPartNodes nodes = part->nodes();

        double min, max;
        resultCollection->minMaxScalarValuesOverAllTensorComponents(address, (int)frameIndex, &min, &max);

        if (max == 0) max = 1;
        float arrowConstantScaling = 0.5 * m_rimReservoirView->tensorResults()->sizeScale() * part->characteristicElementSize();
        float arrowResultScaling   = arrowConstantScaling / cvf::Math::abs(max);

        cvf::ref<RivGeoMechPartMgrCache> partMgrCache = m_rimReservoirView->vizLogic()->partMgrCache();

        for (const RivGeoMechPartMgrCache::Key& partKey : partKeys)
        {
            const RivGeoMechPartMgr* partMgr = partMgrCache->partMgr(partKey);

            for (auto mgr : partMgr->femPartMgrs())
            {
                const RivFemPartGeometryGenerator* surfaceGenerator     = mgr->surfaceGenerator();
                const std::vector<size_t>& quadVerticesToNodeIdxMapping = surfaceGenerator->quadVerticesToNodeIdxMapping();
                const std::vector<size_t>& quadVerticesToElmIdx         = surfaceGenerator->quadVerticesToGlobalElmIdx();

                for (int quadIdx = 0; quadIdx < quadVerticesToNodeIdxMapping.size(); quadIdx = quadIdx + 4)
                {
                    cvf::Vec3f center = nodes.coordinates.at(quadVerticesToNodeIdxMapping[quadIdx]) +
                                        nodes.coordinates.at(quadVerticesToNodeIdxMapping[quadIdx + 2]);

                    cvf::Vec3d center3d(center / 2);

                    cvf::Vec3d displayCoord = m_rimReservoirView->displayCoordTransform()->transformToDisplayCoord(center3d);

                    size_t elmIdx = quadVerticesToElmIdx[quadIdx];

                    cvf::Vec3f result1, result2, result3;

                    if (m_rimReservoirView->tensorResults()->scaleMethod() == RimTensorResults::RESULT)
                    {
                        result1.set(elmPrincipalDirections[elmIdx][0] * arrowResultScaling * elmPrincipals[0][elmIdx]);
                        result2.set(elmPrincipalDirections[elmIdx][1] * arrowResultScaling * elmPrincipals[1][elmIdx]);
                        result3.set(elmPrincipalDirections[elmIdx][2] * arrowResultScaling * elmPrincipals[2][elmIdx]);
                    }
                    else
                    {
                        result1.set(elmPrincipalDirections[elmIdx][0] * arrowConstantScaling);
                        result2.set(elmPrincipalDirections[elmIdx][1] * arrowConstantScaling);
                        result3.set(elmPrincipalDirections[elmIdx][2] * arrowConstantScaling);
                    }

                    if (isDrawable(result1, m_rimReservoirView->tensorResults()->showPrincipal1()))
                    {
                        tensorVisualizations.push_back(
                            TensorVisualization(cvf::Vec3f(displayCoord), result1, color1, isPressure(elmPrincipals[0][elmIdx])));
                        tensorVisualizations.push_back(TensorVisualization(
                            cvf::Vec3f(displayCoord), -result1, color1, isPressure(elmPrincipals[0][elmIdx])));
                    }

                    if (isDrawable(result2, m_rimReservoirView->tensorResults()->showPrincipal2()))
                    {
                        tensorVisualizations.push_back(
                            TensorVisualization(cvf::Vec3f(displayCoord), result2, color2, isPressure(elmPrincipals[1][elmIdx])));
                        tensorVisualizations.push_back(TensorVisualization(
                            cvf::Vec3f(displayCoord), -result2, color2, isPressure(elmPrincipals[1][elmIdx])));
                    }
                    if (isDrawable(result3, m_rimReservoirView->tensorResults()->showPrincipal3()))
                    {
                        tensorVisualizations.push_back(
                            TensorVisualization(cvf::Vec3f(displayCoord), result3, color3, isPressure(elmPrincipals[2][elmIdx])));
                        tensorVisualizations.push_back(TensorVisualization(
                            cvf::Vec3f(displayCoord), -result3, color3, isPressure(elmPrincipals[2][elmIdx])));
                    }
                }
            }
        }
    }

    if (!tensorVisualizations.empty())
    {
        cvf::ref<cvf::Part> partIdx = createPart(tensorVisualizations);
        model->addPart(partIdx.p());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivTensorResultPartMgr::createPart(std::vector<TensorVisualization>& tensorVisualizations) const
{
    cvf::ref<cvf::Vec3fArray>   vertices = new cvf::Vec3fArray;
    cvf::ref<cvf::Vec3fArray>   vecRes   = new cvf::Vec3fArray;
    cvf::ref<cvf::Color3fArray> colors   = new cvf::Color3fArray;

    size_t numVecs = tensorVisualizations.size();
    vertices->reserve(numVecs);
    vecRes->reserve(numVecs);
    colors->reserve(numVecs);

    for (TensorVisualization tensorVisualization : tensorVisualizations)
    {
        if (tensorVisualization.isPressure)
        {
            vertices->add(tensorVisualization.vertex - tensorVisualization.result);
        }
        else
        {
            vertices->add(tensorVisualization.vertex);
        }
        vecRes->add(tensorVisualization.result);
        colors->add(tensorVisualization.color);
    }

    cvf::ref<cvf::DrawableVectors> vectorDrawable;
    if (RiaApplication::instance()->useShaders())
    {
        // NOTE: Drawable vectors must be rendered using shaders when the rest of the application is rendered using shaders
        // Drawing vectors using fixed function when rest of the application uses shaders causes visual artifacts
        vectorDrawable = new cvf::DrawableVectors("u_transformationMatrix", "u_color");
    }
    else
    {
        vectorDrawable = new cvf::DrawableVectors();
    }

    // Create the arrow glyph for the vector drawer
    cvf::GeometryBuilderTriangles arrowBuilder;
    cvf::ArrowGenerator           gen;
    gen.setShaftRelativeRadius(0.020f);
    gen.setHeadRelativeRadius(0.05f);
    gen.setHeadRelativeLength(0.1f);
    gen.setNumSlices(30);
    gen.generate(&arrowBuilder);

    vectorDrawable->setVectors(vertices.p(), vecRes.p());
    vectorDrawable->setColors(colors.p());
    vectorDrawable->setGlyph(arrowBuilder.trianglesUShort().p(), arrowBuilder.vertices().p());

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable(vectorDrawable.p());

    cvf::ref<cvf::Effect> eff = new cvf::Effect;
    if (RiaApplication::instance()->useShaders())
    {
        if (m_rimReservoirView->viewer())
        {
            cvf::ref<cvf::OpenGLContext> oglContext      = m_rimReservoirView->viewer()->cvfOpenGLContext();
            cvf::OpenGLResourceManager*  resourceManager = oglContext->resourceManager();
            cvf::ref<cvf::ShaderProgram> vectorProgram   = resourceManager->getLinkedVectorDrawerShaderProgram(oglContext.p());

            eff->setShaderProgram(vectorProgram.p());
        }
    }

    part->setEffect(eff.p());

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::assignColorVectors(RimTensorResults::TensorColors tensorColor,
                                                cvf::Color3f*                  color1,
                                                cvf::Color3f*                  color2,
                                                cvf::Color3f*                  color3)
{
    if (tensorColor == RimTensorResults::WHITE_GRAY_BLACK)
    {
        *color1 = cvf::Color3f(cvf::Color3::WHITE);
        *color2 = cvf::Color3f(cvf::Color3::GRAY);
        *color3 = cvf::Color3f(cvf::Color3::BLACK);
    }
    else if (tensorColor == RimTensorResults::MAGENTA_BROWN_BLACK)
    {
        *color1 = cvf::Color3f(cvf::Color3::MAGENTA);
        *color2 = cvf::Color3f(cvf::Color3::BROWN);
        *color3 = cvf::Color3f(cvf::Color3::BLACK);
    }
    else
    {
        *color1 = cvf::Color3f(cvf::Color3::BLACK);
        *color2 = cvf::Color3f(cvf::Color3::BLACK);
        *color3 = cvf::Color3f(cvf::Color3::BLACK);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isTensorAddress(RigFemResultAddress address)
{
    if (!(address.resultPosType == RIG_ELEMENT_NODAL || address.resultPosType == RIG_INTEGRATION_POINT))
    {
        return false;
    }
    if (!(address.fieldName == "SE" || address.fieldName == "ST" || address.fieldName == "E"))
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isValid(cvf::Vec3f resultVector)
{
    // nan
    if (resultVector.x() != resultVector.x() || resultVector.y() != resultVector.y() || resultVector.z() != resultVector.z())
    {
        return false;
    }

    // inf
    if (resultVector.x() == HUGE_VAL || resultVector.y() == HUGE_VAL || resultVector.z() == HUGE_VAL ||
        resultVector.x() == -HUGE_VAL || resultVector.y() == -HUGE_VAL || resultVector.z() == -HUGE_VAL)
    {
        return false;
    }

    // zero
    if (resultVector == cvf::Vec3f::ZERO)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isPressure(float principalValue)
{
    if (principalValue < 0)
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isDrawable(cvf::Vec3f resultVector, bool showPrincipal) const
{
    if (!showPrincipal)
    {
        return false;
    }

    if (!isValid(resultVector))
    {
        return false;
    }

    if (resultVector.length() <= m_rimReservoirView->tensorResults()->threshold())
    {
        return false;
    }

    return true;
}
