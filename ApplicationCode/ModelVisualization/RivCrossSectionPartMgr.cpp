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

#include "RivCrossSectionPartMgr.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCrossSection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimTernaryLegendConfig.h"

#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivTernaryScalarMapper.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCase.h"
#include "RigGeomechCaseData.h"
#include "RigFemPartCollection.h"
#include "RimGeoMechCellColors.h"
#include "RigFemPartResultsCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCrossSectionPartMgr::RivCrossSectionPartMgr(const RimCrossSection* rimCrossSection)
    : m_rimCrossSection(rimCrossSection),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(m_rimCrossSection);

    m_crossSectionFacesTextureCoords = new cvf::Vec2fArray;

    computeData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::applySingleColorEffect()
{
    if (m_crossSectionGenerator.isNull()) return;

    m_defaultColor = cvf::Color3f::OLIVE;//m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::updateCellResultColor(size_t timeStepIndex)
{
    RimEclipseView* eclipseView;
    m_rimCrossSection->firstAnchestorOrThisOfType(eclipseView);
    if (eclipseView)
    {
        RimEclipseCellColors* cellResultColors = eclipseView->cellResult();

        if (m_crossSectionGenerator.isNull()) return;

        CVF_ASSERT(cellResultColors);

        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel());
        RigCaseData* eclipseCase = eclipseView->eclipseCase()->reservoirData();

        // CrossSections
        if (m_crossSectionFaces.notNull())
        {
            if (cellResultColors->isTernarySaturationSelected())
            {
                //RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(),
                //    timeStepIndex,
                //    m_grid->gridIndex(),
                //    m_nativeCrossSectionGenerator->quadToCellFaceMapper());
                //
                //texturer.createTextureCoords(m_nativeCrossSectionFacesTextureCoords.p());

                CVF_ASSERT(false); // Todo

                const RivTernaryScalarMapper* mapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
                RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_crossSectionFaces.p(),
                                                                       m_crossSectionFacesTextureCoords.p(),
                                                                       mapper,
                                                                       1.0,
                                                                       caf::FC_NONE,
                                                                       eclipseView->isLightingDisabled());
            }
            else
            {
                CVF_ASSERT(m_crossSectionGenerator.notNull());

                const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

                cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(cellResultColors->reservoirView()->eclipseCase()->reservoirData(),
                                                                                                            0,
                                                                                                            RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel()),
                                                                                                            timeStepIndex,
                                                                                                            cellResultColors->resultVariable());

                calculateEclipseTextureCoordinates(m_crossSectionFacesTextureCoords.p(),
                                                   m_crossSectionGenerator->triangleToCellIndex(),
                                                   resultAccessor.p(),
                                                   mapper);


                RivScalarMapperUtils::applyTextureResultsToPart(m_crossSectionFaces.p(),
                                                                m_crossSectionFacesTextureCoords.p(),
                                                                mapper,
                                                                1.0,
                                                                caf::FC_NONE,
                                                                eclipseView->isLightingDisabled());
            }
        }
    }

    RimGeoMechView* geoView;
    m_rimCrossSection->firstAnchestorOrThisOfType(geoView);

    if (geoView)
    {
        RimGeoMechCellColors* cellResultColors = geoView->cellResult();
        RigGeoMechCaseData* caseData = cellResultColors->ownerCaseData();
        
        if (!caseData) return;

        const cvf::ScalarMapper* mapper        = cellResultColors->legendConfig()->scalarMapper();
        RigFemResultAddress      resVarAddress = cellResultColors->resultAddress();

        // Do a "Hack" to show elm nodal and not nodal POR results
        if (resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar") resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

        const std::vector<float>& resultValues = caseData->femPartResults()->resultValues(resVarAddress, 0, (int)timeStepIndex);

        const std::vector<RivVertexWeights> &vertexWeights = m_crossSectionGenerator->triangleVxToCellCornerInterpolationWeights();

        bool isNodalResult = false;
        RigFemPart* femPart = NULL;
        if (resVarAddress.resultPosType == RIG_NODAL)
        {
            isNodalResult = true;
            femPart = caseData->femParts()->part(0);
        }

        m_crossSectionFacesTextureCoords->resize(vertexWeights.size());

        if (resultValues.size() == 0)
        {
            m_crossSectionFacesTextureCoords->setAll(cvf::Vec2f(0.0, 1.0f));
        }
        else
        {
            cvf::Vec2f* rawPtr = m_crossSectionFacesTextureCoords->ptr();

            int vxCount = static_cast<int>(vertexWeights.size());

            #pragma omp parallel for schedule(dynamic)
            for (int triangleVxIdx = 0; triangleVxIdx < vxCount; ++triangleVxIdx)
            {
                float resValue = 0;
                int weightCount = vertexWeights[triangleVxIdx].size();
                for (int wIdx = 0; wIdx < weightCount; ++wIdx)
                {
                    size_t resIdx = isNodalResult ? vertexWeights[triangleVxIdx].vxId(wIdx): femPart->nodeIdxFromElementNodeResultIdx(vertexWeights[triangleVxIdx].vxId(wIdx));
                    resValue += resultValues[vertexWeights[triangleVxIdx].vxId(wIdx)] * vertexWeights[triangleVxIdx].weight(wIdx);
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

        RivScalarMapperUtils::applyTextureResultsToPart(m_crossSectionFaces.p(), 
                                                        m_crossSectionFacesTextureCoords.p(), 
                                                        mapper, 
                                                        1.0, 
                                                        caf::FC_NONE, 
                                                        geoView->isLightingDisabled());
    }
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimentional texture. 
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                          const std::vector<size_t>& triangleToCellIdxMap,
                                                          const RigResultAccessor* resultAccessor, 
                                                          const cvf::ScalarMapper* mapper) const
{
    if (!resultAccessor) return;

    size_t numVertices = triangleToCellIdxMap.size()*3;

    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double cellScalarValue;
    cvf::Vec2f texCoord;
    
    int triangleCount = static_cast<int>(triangleToCellIdxMap.size());

#pragma omp parallel for private(texCoord, cellScalarValue)
    for (int tIdx = 0; tIdx < triangleCount; tIdx++)
    {
        cellScalarValue = resultAccessor->cellScalarGlobIdx(triangleToCellIdxMap[tIdx]);
        texCoord = mapper->mapToTextureCoord(cellScalarValue);
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
void RivCrossSectionPartMgr::generatePartGeometry()
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
            part->setName("Cross Section ");
            part->setDrawable(geo.p());

            // Set mapping from triangle face index to cell index
            //cvf::ref<RivSourceInfo> si = new RivSourceInfo(m_grid->gridIndex());
            //si->m_cellFaceFromTriangleMapper = m_nativeCrossSectionGenerator->triangleToCellFaceMapper();
            //part->setSourceInfo(si.p());

            part->updateBoundingBox();
            part->setEnableMask(surfaceBit);
            part->setPriority(priCrossSectionGeo);

            m_crossSectionFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_crossSectionGenerator->createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Cross Section mesh" );
            part->setDrawable(geoMesh.p());

            part->updateBoundingBox();
            part->setEnableMask(meshSurfaceBit);
            part->setPriority(priMesh);

            m_crossSectionGridLines = part;
        }
    }

    updatePartEffect();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::updatePartEffect()
{
    if (m_crossSectionGenerator.isNull()) return;

    // Set deCrossSection effect
    caf::SurfaceEffectGenerator geometryEffgen(m_defaultColor, caf::PO_1);
  
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_crossSectionFaces.notNull())
    {
        m_crossSectionFaces->setEffect(geometryOnlyEffect.p());
    }

    // Update mesh colors as well, in case of change
    //RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    caf::MeshEffectGenerator CrossSectionEffGen(cvf::Color3::WHITE);//prefs->defaultCrossSectionGridLineColors());
    eff = CrossSectionEffGen.generateCachedEffect();

    if (m_crossSectionGridLines.notNull())
    {
        m_crossSectionGridLines->setEffect(eff.p());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
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
void RivCrossSectionPartMgr::appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::computeData()
{
    std::vector< std::vector <cvf::Vec3d> > polyLine = m_rimCrossSection->polyLines();
    if (polyLine.size() > 0)
    {
        cvf::Vec3d direction = extrusionDirection(polyLine[0]);
        cvf::ref<RivCrossSectionHexGridIntf> hexGrid = createHexGridInterface();
        m_crossSectionGenerator = new RivCrossSectionGeometryGenerator(polyLine[0], direction, hexGrid.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RivCrossSectionHexGridIntf> RivCrossSectionPartMgr::createHexGridInterface()
{

    RimEclipseView* eclipseView;
    m_rimCrossSection->firstAnchestorOrThisOfType(eclipseView);
    if (eclipseView)
    {
        RigMainGrid* grid = NULL;
        grid = eclipseView->eclipseCase()->reservoirData()->mainGrid();
        return new RivEclipseCrossSectionGrid(grid);
    }

    RimGeoMechView* geoView;
    m_rimCrossSection->firstAnchestorOrThisOfType(geoView);
    if (geoView)
    {
        RigFemPart* femPart = geoView->geoMechCase()->geoMechData()->femParts()->part(0);
        return new RivFemCrossSectionGrid(femPart);
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivCrossSectionPartMgr::extrusionDirection(const std::vector<cvf::Vec3d>& polyline) const
{
    CVF_ASSERT(m_rimCrossSection);

    cvf::Vec3d dir = cvf::Vec3d::Z_AXIS;

    if (m_rimCrossSection->direction == RimCrossSection::CS_HORIZONTAL &&
        polyline.size() > 1)
    {
        // Use first and last point of polyline to approximate orientation of polyline
        // Then cross with Z axis to find extrusion direction
        
        cvf::Vec3d polyLineDir = polyline[polyline.size() - 1] - polyline[0];
        cvf::Vec3d up = cvf::Vec3d::Z_AXIS;
        dir = polyLineDir ^ up;
    }

    return dir;
}

