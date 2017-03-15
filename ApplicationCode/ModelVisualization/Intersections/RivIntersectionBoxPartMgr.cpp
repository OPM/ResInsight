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
#include "RivIntersectionPartMgr.h"
#include "RivPartPriority.h"
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
#include "cvfStructGridGeometryGenerator.h"



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
        RigEclipseCaseData* eclipseCase = eclipseView->eclipseCase()->eclipseCaseData();

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
                    resultAccessor = RigResultAccessorFactory::createFromResultDefinition(cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData(),
                                                                                          0,
                                                                                          timeStepIndex,
                                                                                          cellResultColors);
                }

                RivIntersectionPartMgr::calculateEclipseTextureCoordinates(m_intersectionBoxFacesTextureCoords.p(),
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

        const std::vector<RivIntersectionVertexWeights> &vertexWeights = m_intersectionBoxGenerator->triangleVxToCellCornerInterpolationWeights();
        const cvf::ScalarMapper* mapper                    = cellResultColors->legendConfig()->scalarMapper();
        
        if (!(resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE) )
        {
            // Do a "Hack" to show elm nodal and not nodal POR results
            if ( resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar" ) resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

            const std::vector<float>& resultValues             = caseData->femPartResults()->resultValues(resVarAddress, 0, (int)timeStepIndex);
            RigFemPart* femPart                                = caseData->femParts()->part(0);
            bool isElementNodalResult                          = !(resVarAddress.resultPosType == RIG_NODAL);

            RivIntersectionPartMgr::calculateGeoMechTextureCoords(m_intersectionBoxFacesTextureCoords.p(),
                                                                  vertexWeights,
                                                                  resultValues,
                                                                  isElementNodalResult,
                                                                  femPart,
                                                                  mapper);
        }
        else
        {
            // Special direction sensitive result calculation
            const cvf::Vec3fArray* triangelVxes = m_intersectionBoxGenerator->triangleVxes();

            if ( resVarAddress.componentName == "Pazi" || resVarAddress.componentName == "Pinc" )
            {
                RivIntersectionPartMgr::calculatePlaneAngleTextureCoords(m_intersectionBoxFacesTextureCoords.p(),
                                                                         triangelVxes,
                                                                         resVarAddress,
                                                                         mapper);
            }
            else
            {
                RivIntersectionPartMgr::calculateGeoMechTensorXfTextureCoords(m_intersectionBoxFacesTextureCoords.p(),
                                                                          triangelVxes,
                                                                          vertexWeights,
                                                                          caseData,
                                                                          resVarAddress,
                                                                          (int)timeStepIndex,
                                                                          mapper);
            }
        }

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
            part->setPriority(RivPartPriority::PartType::Intersection);

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
            part->setPriority(RivPartPriority::PartType::MeshLines);

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
        RigMainGrid* grid = eclipseView->mainGrid();

        return new RivEclipseIntersectionGrid(grid, eclipseView->currentActiveCellInfo(), m_rimIntersectionBox->showInactiveCells());
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

