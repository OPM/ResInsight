/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivIntersectionPartMgr.h"

#include "RigCaseCellResultsData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim2dIntersectionView.h"
#include "RimIntersection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTernaryLegendConfig.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RivHexGridIntersectionTools.h"
#include "RivIntersectionGeometryGenerator.h"
#include "RivObjectSourceInfo.h"
#include "RivIntersectionSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryTextureCoordsCreator.h"
#include "RivWellPathSourceInfo.h"

#include "RiuGeoMechXfTensorResultAccessor.h"

#include "cafTensor3.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryTools.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfqtUtils.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfRenderState_FF.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePoint.h"
#include "cvfStructGridGeometryGenerator.h"
#include "cvfTransform.h"

#include <functional>
#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimFaultInView.h"
#include "RiaOffshoreSphericalCoords.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionPartMgr::RivIntersectionPartMgr(RimIntersection* rimCrossSection, bool isFlattened)
    : m_rimCrossSection(rimCrossSection),
    m_isFlattened(isFlattened)
{
    CVF_ASSERT(m_rimCrossSection);

    m_crossSectionFacesTextureCoords = new cvf::Vec2fArray;
    
    cvf::Vec3d flattenedPolylineStartPoint;

    std::vector< std::vector <cvf::Vec3d> > polyLines = m_rimCrossSection->polyLines(&flattenedPolylineStartPoint);
    if (polyLines.size() > 0)
    {
        cvf::Vec3d direction = m_rimCrossSection->extrusionDirection();
        cvf::ref<RivIntersectionHexGridInterface> hexGrid = createHexGridInterface();
        m_crossSectionGenerator = new RivIntersectionGeometryGenerator(m_rimCrossSection, 
                                                                       polyLines, 
                                                                       direction, 
                                                                       hexGrid.p(), 
                                                                       m_isFlattened, 
                                                                       flattenedPolylineStartPoint);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::applySingleColorEffect()
{
    if (m_crossSectionGenerator.isNull()) return;

    caf::SurfaceEffectGenerator geometryEffgen(cvf::Color3f::OLIVE, caf::PO_1);

    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_crossSectionFaces.notNull())
    {
        m_crossSectionFaces->setEffect(geometryOnlyEffect.p());
    }

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    if (m_crossSectionGridLines.notNull())
    {
        cvf::ref<cvf::Effect> eff;
        caf::MeshEffectGenerator CrossSectionEffGen(prefs->defaultGridLineColors());
        eff = CrossSectionEffGen.generateCachedEffect();

        m_crossSectionGridLines->setEffect(eff.p());
    }

    if (m_crossSectionFaultGridLines.notNull())
    {
        cvf::ref<cvf::Effect> eff;
        caf::MeshEffectGenerator CrossSectionEffGen(prefs->defaultFaultGridLineColors());
        eff = CrossSectionEffGen.generateCachedEffect();

        m_crossSectionFaultGridLines->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::updateCellResultColor(size_t timeStepIndex, 
                                                   const cvf::ScalarMapper* scalarColorMapper, 
                                                   const RivTernaryScalarMapper* ternaryColorMapper)
{
    CVF_ASSERT(scalarColorMapper);

    if (m_crossSectionGenerator.isNull()) return;

    if (!m_crossSectionGenerator->isAnyGeometryPresent()) return;

    RimEclipseView* eclipseView = nullptr;
    m_rimCrossSection->firstAncestorOrThisOfType(eclipseView);

    if (eclipseView)
    {
        RimEclipseCellColors* cellResultColors = eclipseView->cellResult();
        CVF_ASSERT(cellResultColors);
        CVF_ASSERT(ternaryColorMapper);

        RigEclipseCaseData* eclipseCase = eclipseView->eclipseCase()->eclipseCaseData();

        // CrossSections
        if (m_crossSectionFaces.notNull())
        {
            if (cellResultColors->isTernarySaturationSelected())
            {
                RivTernaryTextureCoordsCreator texturer(cellResultColors, ternaryColorMapper, timeStepIndex);
                
                texturer.createTextureCoords(m_crossSectionFacesTextureCoords.p(), m_crossSectionGenerator->triangleToCellIndex());

                RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_crossSectionFaces.p(),
                                                                       m_crossSectionFacesTextureCoords.p(),
                                                                       ternaryColorMapper,
                                                                       1.0,
                                                                       caf::FC_NONE,
                                                                       eclipseView->isLightingDisabled());
            }
            else
            {
                CVF_ASSERT(m_crossSectionGenerator.notNull());

                cvf::ref<RigResultAccessor> resultAccessor;

                if (RiaDefines::isPerCellFaceResult(cellResultColors->resultVariable()))
                {
                    resultAccessor = new RigHugeValResultAccessor;
                }
                else
                {
                    resultAccessor = RigResultAccessorFactory::createFromResultDefinition(cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData(),
                                                                                          0,
                                                                                          timeStepIndex,
                                                                                          cellResultColors);
                }

                RivIntersectionPartMgr::calculateEclipseTextureCoordinates(m_crossSectionFacesTextureCoords.p(),
                                                                            m_crossSectionGenerator->triangleToCellIndex(),
                                                                            resultAccessor.p(),
                                                                            scalarColorMapper);


                RivScalarMapperUtils::applyTextureResultsToPart(m_crossSectionFaces.p(),
                                                                m_crossSectionFacesTextureCoords.p(),
                                                                scalarColorMapper,
                                                                1.0,
                                                                caf::FC_NONE,
                                                                eclipseView->isLightingDisabled());
                }
        }
    }

    RimGeoMechView* geoView;
    m_rimCrossSection->firstAncestorOrThisOfType(geoView);

    if (geoView)
    {
        RimGeoMechCellColors* cellResultColors = geoView->cellResult();
        RigGeoMechCaseData* caseData = cellResultColors->ownerCaseData();
        
        if (!caseData) return;

        RigFemResultAddress      resVarAddress = cellResultColors->resultAddress();

        if (resVarAddress.resultPosType == RIG_ELEMENT)
        {
            const std::vector<float>& resultValues          = caseData->femPartResults()->resultValues(resVarAddress, 0, (int)timeStepIndex);
            const std::vector<size_t>& triangleToCellIdx    = m_crossSectionGenerator->triangleToCellIndex();

            RivIntersectionPartMgr::calculateElementBasedGeoMechTextureCoords(m_crossSectionFacesTextureCoords.p(),
                                                                              resultValues,
                                                                              triangleToCellIdx,
                                                                              scalarColorMapper);

        }
        else if(resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE)
        {
            // Special direction sensitive result calculation
            const cvf::Vec3fArray* triangelVxes = m_crossSectionGenerator->triangleVxes();

            if (resVarAddress.componentName == "Pazi" || resVarAddress.componentName == "Pinc")
            {
                RivIntersectionPartMgr::calculatePlaneAngleTextureCoords(m_crossSectionFacesTextureCoords.p(),
                                                                         triangelVxes,
                                                                         resVarAddress,
                                                                         scalarColorMapper);
            }
            else
            {
                const std::vector<RivIntersectionVertexWeights> &vertexWeights = m_crossSectionGenerator->triangleVxToCellCornerInterpolationWeights();

                RivIntersectionPartMgr::calculateGeoMechTensorXfTextureCoords(m_crossSectionFacesTextureCoords.p(),
                                                                              triangelVxes,
                                                                              vertexWeights,
                                                                              caseData,
                                                                              resVarAddress,
                                                                              (int)timeStepIndex,
                                                                              scalarColorMapper);
            }
        }
        else
        {
            // Do a "Hack" to show elm nodal and not nodal POR results
            if (resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar") resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

            const std::vector<float>& resultValues = caseData->femPartResults()->resultValues(resVarAddress, 0, (int)timeStepIndex);
            RigFemPart* femPart = caseData->femParts()->part(0);
            bool isElementNodalResult = !(resVarAddress.resultPosType == RIG_NODAL);
            const std::vector<RivIntersectionVertexWeights> &vertexWeights = m_crossSectionGenerator->triangleVxToCellCornerInterpolationWeights();

            RivIntersectionPartMgr::calculateNodeOrElementNodeBasedGeoMechTextureCoords(m_crossSectionFacesTextureCoords.p(),
                                                                                        vertexWeights,
                                                                                        resultValues,
                                                                                        isElementNodalResult,
                                                                                        femPart,
                                                                                        scalarColorMapper);
        }

        RivScalarMapperUtils::applyTextureResultsToPart(m_crossSectionFaces.p(), 
                                                        m_crossSectionFacesTextureCoords.p(), 
                                                        scalarColorMapper, 
                                                        1.0, 
                                                        caf::FC_NONE, 
                                                        geoView->isLightingDisabled());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::calculateNodeOrElementNodeBasedGeoMechTextureCoords(cvf::Vec2fArray* textureCoords, 
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
                size_t resIdx;
                if (isElementNodalResult)
                {
                    resIdx = vertexWeights[triangleVxIdx].vxId(wIdx);
                }
                else
                {
                    resIdx = femPart->nodeIdxFromElementNodeResultIdx(vertexWeights[triangleVxIdx].vxId(wIdx));
                }
                    
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
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::calculateElementBasedGeoMechTextureCoords(cvf::Vec2fArray* textureCoords,
                                                                       const std::vector<float> &resultValues,
                                                                       const std::vector<size_t>& triangleToCellIdx,
                                                                       const cvf::ScalarMapper* mapper)
{
    textureCoords->resize(triangleToCellIdx.size()*3);

    if (resultValues.size() == 0)
    {
        textureCoords->setAll(cvf::Vec2f(0.0, 1.0f));
    }
    else
    {
        cvf::Vec2f* rawPtr = textureCoords->ptr();

        for (size_t triangleIdx = 0; triangleIdx < triangleToCellIdx.size(); triangleIdx++)
        {
            size_t resIdx = triangleToCellIdx[triangleIdx];
            float resValue = resultValues[resIdx];

            size_t triangleVxIdx = triangleIdx * 3;
            
            if (resValue == HUGE_VAL || resValue != resValue) // a != a is true for NAN's
            {
                rawPtr[triangleVxIdx][1] = 1.0f;
                rawPtr[triangleVxIdx + 1][1] = 1.0f;
                rawPtr[triangleVxIdx + 2][1] = 1.0f;
            }
            else
            {
                rawPtr[triangleVxIdx] = mapper->mapToTextureCoord(resValue);
                rawPtr[triangleVxIdx + 1] = mapper->mapToTextureCoord(resValue);
                rawPtr[triangleVxIdx + 2] = mapper->mapToTextureCoord(resValue);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::calculateGeoMechTensorXfTextureCoords(cvf::Vec2fArray* textureCoords, 
                                                                   const cvf::Vec3fArray* triangelVertices,
                                                                   const std::vector<RivIntersectionVertexWeights> &vertexWeights, 
                                                                   RigGeoMechCaseData* caseData, 
                                                                   const RigFemResultAddress& resVarAddress, 
                                                                   int   timeStepIdx,
                                                                   const cvf::ScalarMapper* mapper)
{  

    RiuGeoMechXfTensorResultAccessor accessor(caseData->femPartResults(), resVarAddress, timeStepIdx);

    textureCoords->resize(vertexWeights.size());
    cvf::Vec2f* rawPtr = textureCoords->ptr();
    int vxCount = static_cast<int>(vertexWeights.size());
    int triCount = vxCount/3;

    #pragma omp parallel for schedule(dynamic)
    for ( int triangleIdx = 0; triangleIdx < triCount; ++triangleIdx )
    {
        int triangleVxStartIdx =  triangleIdx*3;
        float values[3];

        accessor.calculateInterpolatedValue(&((*triangelVertices)[triangleVxStartIdx]), &(vertexWeights[triangleVxStartIdx]), values );

        rawPtr[triangleVxStartIdx + 0] = (values[0] != std::numeric_limits<float>::infinity()) ? mapper->mapToTextureCoord(values[0]) : cvf::Vec2f(0.0f, 1.0f);
        rawPtr[triangleVxStartIdx + 1] = (values[1] != std::numeric_limits<float>::infinity()) ? mapper->mapToTextureCoord(values[1]) : cvf::Vec2f(0.0f, 1.0f);
        rawPtr[triangleVxStartIdx + 2] = (values[2] != std::numeric_limits<float>::infinity()) ? mapper->mapToTextureCoord(values[2]) : cvf::Vec2f(0.0f, 1.0f);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::calculatePlaneAngleTextureCoords(cvf::Vec2fArray* textureCoords,
                                                                   const cvf::Vec3fArray* triangelVertices,
                                                                   const RigFemResultAddress& resVarAddress,
                                                                   const cvf::ScalarMapper* mapper)
{

    textureCoords->resize(triangelVertices->size());
    cvf::Vec2f* rawPtr = textureCoords->ptr();
    int vxCount = static_cast<int>(triangelVertices->size());
    int triCount = vxCount/3;

    std::function<float (const RiaOffshoreSphericalCoords& )> operation;
    if (resVarAddress.componentName == "Pazi")
    {
        operation = [](const RiaOffshoreSphericalCoords& sphCoord) { return (float)sphCoord.azi();};
    }
    else if ( resVarAddress.componentName == "Pinc" )
    {
        operation = [](const RiaOffshoreSphericalCoords& sphCoord) { return (float)sphCoord.inc();};
    }

    #pragma omp parallel for schedule(dynamic)
    for ( int triangleIdx = 0; triangleIdx < triCount; ++triangleIdx )
    {
        int triangleVxStartIdx =  triangleIdx*3;
        
        const cvf::Vec3f* triangle = &((*triangelVertices)[triangleVxStartIdx]);
        cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx(triangle[1] - triangle[0], triangle[2] - triangle[0]);

        RiaOffshoreSphericalCoords sphCoord(cvf::Vec3f(rotMx.rowCol(0, 2), rotMx.rowCol(1, 2), rotMx.rowCol(2, 2))); // Use Ez from the matrix as plane normal

        float angle = cvf::Math::toDegrees( operation(sphCoord));
        cvf::Vec2f texCoord = (angle != std::numeric_limits<float>::infinity()) ? mapper->mapToTextureCoord(angle) : cvf::Vec2f(0.0f, 1.0f);
        rawPtr[triangleVxStartIdx + 0] = texCoord;
        rawPtr[triangleVxStartIdx + 1] = texCoord;
        rawPtr[triangleVxStartIdx + 2] = texCoord;
    }

}


//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture. 
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::generatePartGeometry()
{
    if (m_crossSectionGenerator.isNull()) return;

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_crossSectionGenerator->generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section");
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivIntersectionSourceInfo> si = new RivIntersectionSourceInfo(m_crossSectionGenerator.p());
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(intersectionCellFaceBit);
            part->setPriority(RivPartPriority::PartType::Intersection);

            m_crossSectionFaces = part;
        }
    }

    // Cell Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_crossSectionGenerator->createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section mesh");
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(intersectionCellMeshBit);
            part->setPriority(RivPartPriority::PartType::MeshLines);

            m_crossSectionGridLines = part;
        }
    }

    // Fault Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_crossSectionGenerator->createFaultMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section faultmesh");
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(intersectionFaultMeshBit);
            part->setPriority(RivPartPriority::PartType::FaultMeshLines);

            m_crossSectionFaultGridLines = part;
        }
    }
    createPolyLineParts(useBufferObjects);

    createExtrusionDirParts(useBufferObjects);

    if (m_isFlattened) createFaultLabelParts(m_crossSectionGenerator->faultMeshLabelAndAnchorPositions());

    applySingleColorEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::createFaultLabelParts(const std::vector<std::pair<QString, cvf::Vec3d> >& labelAndAnchors)
{
    m_faultMeshLabels = nullptr;
    m_faultMeshLabelLines = nullptr;

    if (!labelAndAnchors.size()) return;

    RimEclipseView* eclipseView = nullptr;
    m_rimCrossSection->firstAncestorOrThisOfType(eclipseView);
    RimFaultInViewCollection* faultInViewColl = eclipseView->faultCollection();

    if (!(eclipseView && faultInViewColl->showFaultLabel())) return;
    
    cvf::Color3f defWellLabelColor = faultInViewColl->faultLabelColor();
    cvf::Font* font = RiaApplication::instance()->customFont();

    std::vector<cvf::Vec3f> lineVertices;

    cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
    {
        drawableText->setFont(font);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::BASELINE);
        drawableText->setTextColor(defWellLabelColor);
    }

    cvf::BoundingBox bb = m_crossSectionFaces->boundingBox();
    double labelZOffset = bb.extent().z() / 10;
    int visibleFaultNameCount = 0;

    for (const auto& labelAndAnchorPair : labelAndAnchors)
    {
        RimFaultInView* fault = faultInViewColl->findFaultByName(labelAndAnchorPair.first);

        if (!(fault && fault->showFault())) continue;
        
        cvf::String cvfString = cvfqt::Utils::toString(labelAndAnchorPair.first);
        cvf::Vec3f textCoord(labelAndAnchorPair.second);

        textCoord.z() += labelZOffset;
        drawableText->addText(cvfString, textCoord);

        lineVertices.push_back(cvf::Vec3f(labelAndAnchorPair.second));
        lineVertices.push_back(textCoord);
        visibleFaultNameCount++;
    }

    if (visibleFaultNameCount == 0) return;

    // Labels part
    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("Fault mesh label : text ");
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::Text);
        part->updateBoundingBox();

        m_faultMeshLabels = part;
    }

    // Lines to labels part
    {
        cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
        vertices->assign(lineVertices);
        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
        geo->setVertexArray(vertices.p());

        cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_LINES);
        primSet->setStartIndex(0);
        primSet->setIndexCount(vertices->size());
        geo->addPrimitiveSet(primSet.p());

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("Anchor lines for fault mesh labels");
        part->setDrawable(geo.p());

        part->updateBoundingBox();

        caf::MeshEffectGenerator gen(RiaApplication::instance()->preferences()->defaultFaultGridLineColors());
        cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

        part->setEffect(eff.p());
        m_faultMeshLabelLines = part;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::createPolyLineParts(bool useBufferObjects)
{
    // Highlight line

    m_highlightLineAlongPolyline = nullptr;
    m_highlightPointsForPolyline = nullptr;

    if (m_rimCrossSection->type == RimIntersection::CS_POLYLINE || m_rimCrossSection->type == RimIntersection::CS_AZIMUTHLINE)
    {
        {
            cvf::ref<cvf::DrawableGeo> polylineGeo = m_crossSectionGenerator->createLineAlongPolylineDrawable();
            if (polylineGeo.notNull())
            {
                if (useBufferObjects)
                {
                    polylineGeo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
                }

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName("Cross Section Polyline");
                part->setDrawable(polylineGeo.p());

                part->updateBoundingBox();
                part->setPriority(RivPartPriority::PartType::Highlight);

                // Always show this part, also when mesh is turned off
                //part->setEnableMask(meshFaultBit);

                cvf::ref<cvf::Effect> eff;
                caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
                eff = lineEffGen.generateUnCachedEffect();

                cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
                depth->enableDepthTest(false);
                eff->setRenderState(depth.p());

                part->setEffect(eff.p());

                m_highlightLineAlongPolyline = part;
            }
        }

        cvf::ref<cvf::DrawableGeo> polylinePointsGeo = m_crossSectionGenerator->createPointsFromPolylineDrawable();
        if (polylinePointsGeo.notNull())
        {
            if (useBufferObjects)
            {
                polylinePointsGeo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section Polyline");
            part->setDrawable(polylinePointsGeo.p());

            part->updateBoundingBox();
            part->setPriority(RivPartPriority::PartType::Highlight);

            // Always show this part, also when mesh is turned off
            //part->setEnableMask(meshFaultBit);

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
            eff = lineEffGen.generateUnCachedEffect();

            cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
            depth->enableDepthTest(false);
            eff->setRenderState(depth.p());

            cvf::ref<cvf::RenderStatePoint> pointRendState = new  cvf::RenderStatePoint(cvf::RenderStatePoint::FIXED_SIZE);
            pointRendState->setSize(5.0f);
            eff->setRenderState(pointRendState.p());

            part->setEffect(eff.p());

            m_highlightPointsForPolyline = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::createExtrusionDirParts(bool useBufferObjects)
{
    m_highlightLineAlongExtrusionDir = nullptr;
    m_highlightPointsForExtrusionDir = nullptr;

    if (m_rimCrossSection->direction() == RimIntersection::CS_TWO_POINTS)
    {
        {
            cvf::ref<cvf::DrawableGeo> polylineGeo = m_crossSectionGenerator->createLineAlongExtrusionLineDrawable(m_rimCrossSection->polyLinesForExtrusionDirection());
            if (polylineGeo.notNull())
            {
                if (useBufferObjects)
                {
                    polylineGeo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
                }

                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName("Cross Section Polyline");
                part->setDrawable(polylineGeo.p());

                part->updateBoundingBox();
                part->setPriority(RivPartPriority::PartType::Highlight);

                // Always show this part, also when mesh is turned off
                //part->setEnableMask(meshFaultBit);

                cvf::ref<cvf::Effect> eff;
                caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
                eff = lineEffGen.generateUnCachedEffect();

                cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
                depth->enableDepthTest(false);
                eff->setRenderState(depth.p());

                part->setEffect(eff.p());

                m_highlightLineAlongExtrusionDir = part;
            }
        }

        cvf::ref<cvf::DrawableGeo> polylinePointsGeo = m_crossSectionGenerator->createPointsFromExtrusionLineDrawable(m_rimCrossSection->polyLinesForExtrusionDirection());
        if (polylinePointsGeo.notNull())
        {
            if (useBufferObjects)
            {
                polylinePointsGeo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section Polyline");
            part->setDrawable(polylinePointsGeo.p());

            part->updateBoundingBox();
            part->setPriority(RivPartPriority::PartType::Highlight);

            // Always show this part, also when mesh is turned off
            //part->setEnableMask(meshFaultBit);

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
            eff = lineEffGen.generateUnCachedEffect();

            cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
            depth->enableDepthTest(false);
            eff->setRenderState(depth.p());

            cvf::ref<cvf::RenderStatePoint> pointRendState = new  cvf::RenderStatePoint(cvf::RenderStatePoint::FIXED_SIZE);
            pointRendState->setSize(5.0f);
            eff->setRenderState(pointRendState.p());

            part->setEffect(eff.p());

            m_highlightPointsForExtrusionDir = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> createStdSurfacePart(cvf::DrawableGeo* geometry, 
                                                const cvf::Color3f& color, 
                                                cvf::String name, 
                                                cvf::Object* sourceInfo)
{
    if (!geometry) return nullptr;

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName(name);
    part->setDrawable(geometry);

    caf::SurfaceEffectGenerator surfaceGen(color, caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
    part->setEffect(eff.p());

    part->setSourceInfo(sourceInfo);
    part->updateBoundingBox();

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> createStdLinePart(cvf::DrawableGeo* geometry, 
                                         const cvf::Color3f& color, 
                                         cvf::String name)
{
    if ( !geometry ) return nullptr;


    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName(name);
    part->setDrawable(geometry);

    caf::MeshEffectGenerator gen(color);
    cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

    part->setEffect(eff.p());
    part->updateBoundingBox();

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_crossSectionFaces.isNull())
    {
        generatePartGeometry();
    }

    if (m_crossSectionFaces.notNull())
    {
        m_crossSectionFaces->setTransform(scaleTransform);
        model->addPart(m_crossSectionFaces.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_crossSectionGridLines.isNull())
    {
        generatePartGeometry();
    }

    if (m_crossSectionGridLines.notNull())
    {
        m_crossSectionGridLines->setTransform(scaleTransform);
        model->addPart(m_crossSectionGridLines.p());
    }

    if (m_crossSectionFaultGridLines.notNull())
    {
        m_crossSectionFaultGridLines->setTransform(scaleTransform);
        model->addPart(m_crossSectionFaultGridLines.p());
    }

    if (m_faultMeshLabelLines.notNull())
    {
        m_faultMeshLabelLines->setTransform(scaleTransform);
        model->addPart(m_faultMeshLabelLines.p());
    }

    if (m_faultMeshLabels.notNull())
    {
        m_faultMeshLabels->setTransform(scaleTransform);
        model->addPart(m_faultMeshLabels.p());
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionPartMgr::appendPolylinePartsToModel(Rim3dView &view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    Rim2dIntersectionView* curr2dView = dynamic_cast<Rim2dIntersectionView*>(&view);

    if (m_rimCrossSection->inputPolyLineFromViewerEnabled || (curr2dView && curr2dView->showDefiningPoints()))
    {
        if (m_highlightLineAlongPolyline.notNull())
        {
            m_highlightLineAlongPolyline->setTransform(scaleTransform);
            model->addPart(m_highlightLineAlongPolyline.p());
        }

        if (m_highlightPointsForPolyline.notNull())
        {
            m_highlightPointsForPolyline->setTransform(scaleTransform);
            model->addPart(m_highlightPointsForPolyline.p());
        }
    }

    if (m_rimCrossSection->inputExtrusionPointsFromViewerEnabled)
    {
        if (m_highlightLineAlongExtrusionDir.notNull())
        {
            m_highlightLineAlongExtrusionDir->setTransform(scaleTransform);
            model->addPart(m_highlightLineAlongExtrusionDir.p());
        }

        if (m_highlightPointsForExtrusionDir.notNull())
        {
            m_highlightPointsForExtrusionDir->setTransform(scaleTransform);
            model->addPart(m_highlightPointsForExtrusionDir.p());
        }
    }

    if (m_rimCrossSection->inputTwoAzimuthPointsFromViewerEnabled || (curr2dView && curr2dView->showDefiningPoints()))
    {
        if (m_highlightLineAlongPolyline.notNull())
        {
            m_highlightLineAlongPolyline->setTransform(scaleTransform);
            model->addPart(m_highlightLineAlongPolyline.p());
        }

        if (m_highlightPointsForPolyline.notNull())
        {
            m_highlightPointsForPolyline->setTransform(scaleTransform);
            model->addPart(m_highlightPointsForPolyline.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimIntersection* RivIntersectionPartMgr::intersection() const
{
    return m_rimCrossSection.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RivIntersectionPartMgr::unflattenTransformMatrix(const cvf::Vec3d& intersectionPointFlat)
{
    return m_crossSectionGenerator->unflattenTransformMatrix(intersectionPointFlat);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RivIntersectionPartMgr::createHexGridInterface()
{
    RimEclipseView* eclipseView;
    m_rimCrossSection->firstAncestorOrThisOfType(eclipseView);
    if (eclipseView)
    {
        RigMainGrid* grid = eclipseView->mainGrid();
        return new RivEclipseIntersectionGrid(grid, eclipseView->currentActiveCellInfo(), m_rimCrossSection->showInactiveCells());
    }

    RimGeoMechView* geoView;
    m_rimCrossSection->firstAncestorOrThisOfType(geoView);
    if (geoView && geoView->femParts() && geoView->femParts()->partCount())
    {
        RigFemPart* femPart = geoView->femParts()->part(0);

        return new RivFemIntersectionGrid(femPart);
    }

    return nullptr;
}

