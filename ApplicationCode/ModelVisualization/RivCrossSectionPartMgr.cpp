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

//#include "RiaApplication.h"
//#include "RiaPreferences.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimTernaryLegendConfig.h"

//#include "RimCrossSectionCollection.h"

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
RivCrossSectionPartMgr::RivCrossSectionPartMgr( const RigMainGrid* grid, 
                                                const RimCrossSectionCollection* rimCrossSectionCollection, 
                                                const RimCrossSection* rimCrossSection,
                                                const std::vector<cvf::Vec3d>& polyLine)
    :   m_grid(grid),
        m_rimCrossSectionCollection(rimCrossSectionCollection),
        m_rimCrossSection(rimCrossSection),
        m_defaultColor(cvf::Color3::WHITE)
{

    m_nativeCrossSectionGenerator = new RivCrossSectionGeometryGenerator(polyLine, cvf::Vec3d(0.0,0,1.0), grid );

    m_nativeCrossSectionFacesTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::applySingleColorEffect()
{
    m_defaultColor = cvf::Color3f::OLIVE;//m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCrossSectionPartMgr::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
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
            const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(cellResultColors->reservoirView()->eclipseCase()->reservoirData(), 
                                                                                              0, 
                                                                                              RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultColors->porosityModel()), 
                                                                                              timeStepIndex, 
                                                                                              cellResultColors->resultVariable());

            m_nativeCrossSectionGenerator->textureCoordinates(m_nativeCrossSectionFacesTextureCoords.p(), resultAccessor.p() ,mapper);


            RivScalarMapperUtils::applyTextureResultsToPart(m_nativeCrossSectionFaces.p(), 
                                                            m_nativeCrossSectionFacesTextureCoords.p(), 
                                                            mapper, 
                                                            1.0, 
                                                            caf::FC_NONE, 
                                                            eclipseView->isLightingDisabled());
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

