/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxPartMgr.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionBox.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RivIntersectionBoxSourceInfo.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryTextureCoordsCreator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfRenderState_FF.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePoint.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxPartMgr::RivIntersectionBoxPartMgr(const RimIntersectionBox* intersectionBox)
    : m_rimIntersectionBox(intersectionBox),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(m_rimIntersectionBox);

    m_intersectionBoxFacesTextureCoords = new cvf::Vec2fArray;

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = createHexGridInterface();
    m_intersectionBoxGenerator = new RivIntersectionBoxGeometryGenerator(m_rimIntersectionBox, hexGrid.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::applySingleColorEffect()
{
    m_defaultColor = cvf::Color3f::OLIVE;//m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::updateCellResultColor(size_t timeStepIndex)
{

    if (!m_intersectionBoxGenerator->isAnyGeometryPresent()) return;

    RimEclipseView* eclipseView;
    m_rimIntersectionBox->firstAncestorOrThisOfType(eclipseView);

    if (eclipseView)
    {
        RimEclipseCellColors* cellResultColors = eclipseView->cellResult();
        CVF_ASSERT(cellResultColors);

        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel());
        RigCaseData* eclipseCase = eclipseView->eclipseCase()->reservoirData();

        // CrossSections
        if (m_intersectionBoxFaces.notNull())
        {
            if (cellResultColors->isTernarySaturationSelected())
            {
                RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(), timeStepIndex);
                
                texturer.createTextureCoords(m_intersectionBoxFacesTextureCoords.p(), m_intersectionBoxGenerator->triangleToCellIndex());

                const RivTernaryScalarMapper* mapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
                RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_intersectionBoxFaces.p(),
                                                                       m_intersectionBoxFacesTextureCoords.p(),
                                                                       mapper,
                                                                       1.0,
                                                                       caf::FC_NONE,
                                                                       eclipseView->isLightingDisabled());
            }
            else
            {
                CVF_ASSERT(m_intersectionBoxGenerator.notNull());

                const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();
                cvf::ref<RigResultAccessor> resultAccessor;

                if (RimDefines::isPerCellFaceResult(cellResultColors->resultVariable()))
                {
                    resultAccessor = new RigHugeValResultAccessor;
                }
                else
                {
                    resultAccessor = RigResultAccessorFactory::createResultAccessor(cellResultColors->reservoirView()->eclipseCase()->reservoirData(),
                                                                                    0,
                                                                                    timeStepIndex,
                                                                                    cellResultColors);
                }

                RivIntersectionBoxPartMgr::calculateEclipseTextureCoordinates(m_intersectionBoxFacesTextureCoords.p(),
                                                                            m_intersectionBoxGenerator->triangleToCellIndex(),
                                                                            resultAccessor.p(),
                                                                            mapper);


                RivScalarMapperUtils::applyTextureResultsToPart(m_intersectionBoxFaces.p(),
                                                                m_intersectionBoxFacesTextureCoords.p(),
                                                                mapper,
                                                                1.0,
                                                                caf::FC_NONE,
                                                                eclipseView->isLightingDisabled());
                }
        }
    }

    RimGeoMechView* geoView;
    m_rimIntersectionBox->firstAncestorOrThisOfType(geoView);

    if (geoView)
    {
        RimGeoMechCellColors* cellResultColors = geoView->cellResult();
        RigGeoMechCaseData* caseData = cellResultColors->ownerCaseData();
        
        if (!caseData) return;

        RigFemResultAddress      resVarAddress = cellResultColors->resultAddress();

        // Do a "Hack" to show elm nodal and not nodal POR results
        if (resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar") resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

        const std::vector<RivIntersectionVertexWeights> &vertexWeights = m_intersectionBoxGenerator->triangleVxToCellCornerInterpolationWeights();
        const std::vector<float>& resultValues             = caseData->femPartResults()->resultValues(resVarAddress, 0, (int)timeStepIndex);
        bool isElementNodalResult                          = !(resVarAddress.resultPosType == RIG_NODAL);
        RigFemPart* femPart                                = caseData->femParts()->part(0);
        const cvf::ScalarMapper* mapper                    = cellResultColors->legendConfig()->scalarMapper();

        RivIntersectionBoxPartMgr::calculateGeoMechTextureCoords(m_intersectionBoxFacesTextureCoords.p(), 
                                                              vertexWeights, 
                                                              resultValues, 
                                                              isElementNodalResult, 
                                                              femPart, 
                                                              mapper);

        RivScalarMapperUtils::applyTextureResultsToPart(m_intersectionBoxFaces.p(), 
                                                        m_intersectionBoxFacesTextureCoords.p(), 
                                                        mapper, 
                                                        1.0, 
                                                        caf::FC_NONE, 
                                                        geoView->isLightingDisabled());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::calculateGeoMechTextureCoords(cvf::Vec2fArray* textureCoords, 
                                                           const std::vector<RivIntersectionVertexWeights> &vertexWeights, 
                                                           const std::vector<float> &resultValues, 
                                                           bool isElementNodalResult, 
                                                           const RigFemPart* femPart, 
                                                           const cvf::ScalarMapper* mapper)
{
    textureCoords->resize(vertexWeights.size());

    if (resultValues.size() == 0)
    {
        textureCoords->setAll(cvf::Vec2f(0.0, 1.0f));
    }
    else
    {
        cvf::Vec2f* rawPtr = textureCoords->ptr();

        int vxCount = static_cast<int>(vertexWeights.size());

#pragma omp parallel for schedule(dynamic)
        for (int triangleVxIdx = 0; triangleVxIdx < vxCount; ++triangleVxIdx)
        {
            float resValue = 0;
            int weightCount = vertexWeights[triangleVxIdx].size();
            for (int wIdx = 0; wIdx < weightCount; ++wIdx)
            {
                size_t resIdx = isElementNodalResult ? vertexWeights[triangleVxIdx].vxId(wIdx) :
                    femPart->nodeIdxFromElementNodeResultIdx(vertexWeights[triangleVxIdx].vxId(wIdx));
                resValue += resultValues[resIdx] * vertexWeights[triangleVxIdx].weight(wIdx);
            }

            if (resValue == HUGE_VAL || resValue != resValue) // a != a is true for NAN's
            {
                rawPtr[triangleVxIdx][1]       = 1.0f;
            }
            else
            {
                rawPtr[triangleVxIdx] = mapper->mapToTextureCoord(resValue);
            }
        }
    }
}



//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture. 
/// Undefined values are coded with a y-texture coordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                          const std::vector<size_t>& triangleToCellIdxMap,
                                                          const RigResultAccessor* resultAccessor, 
                                                          const cvf::ScalarMapper* mapper) 
{
    if (!resultAccessor) return;

    size_t numVertices = triangleToCellIdxMap.size()*3;

    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    int triangleCount = static_cast<int>(triangleToCellIdxMap.size());

#pragma omp parallel for
    for (int tIdx = 0; tIdx < triangleCount; tIdx++)
    {
        double cellScalarValue = resultAccessor->cellScalarGlobIdx(triangleToCellIdxMap[tIdx]);
        cvf::Vec2f texCoord = mapper->mapToTextureCoord(cellScalarValue);
        if (cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for (j = 0; j < 3; j++)
        {   
            rawPtr[tIdx*3 + j] = texCoord;
        }
    }
}

const int priCrossSectionGeo = 1;
const int priNncGeo = 2;
const int priMesh = 3;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::generatePartGeometry()
{

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_intersectionBoxGenerator->generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Intersection Box");
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivIntersectionBoxSourceInfo> si = new RivIntersectionBoxSourceInfo(m_intersectionBoxGenerator.p());
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(faultBit);
            part->setPriority(priCrossSectionGeo);

            m_intersectionBoxFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionBoxGenerator->createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Intersection box mesh");
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(meshFaultBit);
            part->setPriority(priMesh);

            m_intersectionBoxGridLines = part;
        }
    }

    updatePartEffect();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::updatePartEffect()
{
    // Set deCrossSection effect
    caf::SurfaceEffectGenerator geometryEffgen(m_defaultColor, caf::PO_1);
  
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_intersectionBoxFaces.notNull())
    {
        m_intersectionBoxFaces->setEffect(geometryOnlyEffect.p());
    }

    // Update mesh colors as well, in case of change
    //RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    caf::MeshEffectGenerator CrossSectionEffGen(cvf::Color3::WHITE);//prefs->defaultCrossSectionGridLineColors());
    eff = CrossSectionEffGen.generateCachedEffect();

    if (m_intersectionBoxGridLines.notNull())
    {
        m_intersectionBoxGridLines->setEffect(eff.p());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_intersectionBoxFaces.isNull() && m_intersectionBoxGridLines.isNull())
    {
        generatePartGeometry();
    }

    if (m_intersectionBoxFaces.notNull())
    {
        m_intersectionBoxFaces->setTransform(scaleTransform);
        model->addPart(m_intersectionBoxFaces.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_intersectionBoxFaces.isNull() && m_intersectionBoxGridLines.isNull())
    {
        generatePartGeometry();
    }

    if (m_intersectionBoxGridLines.notNull())
    {
        m_intersectionBoxGridLines->setTransform(scaleTransform);
        model->addPart(m_intersectionBoxGridLines.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RivIntersectionBoxPartMgr::createHexGridInterface()
{

    RimEclipseView* eclipseView;
    m_rimIntersectionBox->firstAncestorOrThisOfType(eclipseView);
    if (eclipseView)
    {
        RigMainGrid* grid = NULL;
        grid = eclipseView->eclipseCase()->reservoirData()->mainGrid();

        // TODO: Should flag for inactive cells be available at a centralized object?
        return new RivEclipseIntersectionGrid(grid, eclipseView->currentActiveCellInfo(), false);
    }

    RimGeoMechView* geoView;
    m_rimIntersectionBox->firstAncestorOrThisOfType(geoView);
    if (geoView)
    {
        RigFemPart* femPart = geoView->geoMechCase()->geoMechData()->femParts()->part(0);
        return new RivFemIntersectionGrid(femPart);
    }

    return NULL;
}

