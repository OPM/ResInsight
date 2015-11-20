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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivCrossSectionPartMgr::RivCrossSectionPartMgr(const RimCrossSection* rimCrossSection)
    : m_rimCrossSection(rimCrossSection),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(m_rimCrossSection);

    m_nativeCrossSectionFacesTextureCoords = new cvf::Vec2fArray;

    computeData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::applySingleColorEffect()
{
    if (m_nativeCrossSectionGenerator.isNull()) return;

    m_defaultColor = cvf::Color3f::OLIVE;//m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    if (m_nativeCrossSectionGenerator.isNull()) return;

    CVF_ASSERT(cellResultColors);

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel());
    RimEclipseView* eclipseView = cellResultColors->reservoirView();
    RigCaseData* eclipseCase = eclipseView->eclipseCase()->reservoirData();

    // CrossSections
    if (m_nativeCrossSectionFaces.notNull())
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
            RivScalarMapperUtils::applyTernaryTextureResultsToPart(m_nativeCrossSectionFaces.p(), 
                                                                    m_nativeCrossSectionFacesTextureCoords.p(), 
                                                                    mapper, 
                                                                    1.0, 
                                                                    caf::FC_NONE, 
                                                                    eclipseView->isLightingDisabled());
        }
        else
        {
            CVF_ASSERT(m_nativeCrossSectionGenerator.notNull());

            const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(cellResultColors->reservoirView()->eclipseCase()->reservoirData(), 
                                                                                              0, 
                                                                                              RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel()), 
                                                                                              timeStepIndex, 
                                                                                              cellResultColors->resultVariable());

            calculateEclipseTextureCoordinates(m_nativeCrossSectionFacesTextureCoords.p(), 
                                               m_nativeCrossSectionGenerator->triangleToCellIndex(), 
                                               resultAccessor.p(), 
                                               mapper);


            RivScalarMapperUtils::applyTextureResultsToPart(m_nativeCrossSectionFaces.p(), 
                                                            m_nativeCrossSectionFacesTextureCoords.p(), 
                                                            mapper, 
                                                            1.0, 
                                                            caf::FC_NONE, 
                                                            eclipseView->isLightingDisabled());
        }
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
    if (m_nativeCrossSectionGenerator.isNull()) return;

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_nativeCrossSectionGenerator->generateSurface();
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

            m_nativeCrossSectionFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_nativeCrossSectionGenerator->createMeshDrawable();
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

            m_nativeCrossSectionGridLines = part;
        }
    }

    updatePartEffect();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::updatePartEffect()
{
    if (m_nativeCrossSectionGenerator.isNull()) return;

    // Set deCrossSection effect
    caf::SurfaceEffectGenerator geometryEffgen(m_defaultColor, caf::PO_1);
  
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_nativeCrossSectionFaces.notNull())
    {
        m_nativeCrossSectionFaces->setEffect(geometryOnlyEffect.p());
    }

    // Update mesh colors as well, in case of change
    //RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    caf::MeshEffectGenerator CrossSectionEffGen(cvf::Color3::WHITE);//prefs->defaultCrossSectionGridLineColors());
    eff = CrossSectionEffGen.generateCachedEffect();

    if (m_nativeCrossSectionGridLines.notNull())
    {
        m_nativeCrossSectionGridLines->setEffect(eff.p());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_nativeCrossSectionFaces.isNull())
    {
        generatePartGeometry();
    }

    if (m_nativeCrossSectionFaces.notNull())
    {
        m_nativeCrossSectionFaces->setTransform(scaleTransform);
        model->addPart(m_nativeCrossSectionFaces.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (m_nativeCrossSectionGridLines.isNull())
    {
        generatePartGeometry();
    }

    if (m_nativeCrossSectionGridLines.notNull())
    {
        m_nativeCrossSectionGridLines->setTransform(scaleTransform);
        model->addPart(m_nativeCrossSectionGridLines.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::computeData()
{
    RigMainGrid* m_grid = mainGrid();
    CVF_ASSERT(m_grid);

    std::vector< std::vector <cvf::Vec3d> > polyLine = m_rimCrossSection->polyLines();
    if (polyLine.size() > 0)
    {
        cvf::Vec3d direction = extrusionDirection(polyLine[0]);
        cvf::ref<RivEclipseCrossSectionGrid> eclHexGrid = new RivEclipseCrossSectionGrid(m_grid);
        m_nativeCrossSectionGenerator = new RivCrossSectionGeometryGenerator(polyLine[0], direction, eclHexGrid.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RivCrossSectionPartMgr::mainGrid()
{
    RigMainGrid* grid = NULL;

    RimEclipseView* eclipseView = NULL;
    m_rimCrossSection->firstAnchestorOrThisOfType(eclipseView);
    if (eclipseView)
    {
        grid = eclipseView->eclipseCase()->reservoirData()->mainGrid();
    }

    return grid;
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

