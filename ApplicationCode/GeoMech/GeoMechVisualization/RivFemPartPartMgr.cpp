/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <stdlib.h>

#include "RivGeoMechPartMgr.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifGeoMechReaderInterface.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"

#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimLegendConfig.h"

#include "RivFemPickSourceInfo.h"
#include "RivPartPriority.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivSourceInfo.h"
#include "RivTextureCoordsCreator.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfMath.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderState_FF.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfStructGrid.h"
#include "cvfTransform.h"
#include "cvfUniform.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFemPartPartMgr::RivFemPartPartMgr(const RigFemPart* grid)
:   m_surfaceGenerator(grid), 
    m_grid(grid),
    m_opacityLevel(1.0f),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(grid);
    m_gridIdx = grid->elementPartId();
    m_cellVisibility = new cvf::UByteArray;
    m_surfaceFacesTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartPartMgr::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(m_scaleTransform.notNull());
    CVF_ASSERT(cellVisibilities);

    m_cellVisibility = cellVisibilities;

    m_surfaceGenerator.setElementVisibility(cellVisibilities);

    generatePartGeometry(m_surfaceGenerator);
}

void RivFemPartPartMgr::generatePartGeometry(RivFemPartGeometryGenerator& geoBuilder)
{
    bool useBufferObjects = true;
    // Surface geometry
    {   
        m_surfaceFaces = NULL; // To possibly free memory before adding the new stuff

        cvf::ref<cvf::DrawableGeo> geo = geoBuilder.generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("FemPart " + cvf::String(m_gridIdx));
            part->setId(m_gridIdx);       // Use grid index as part ID 
            part->setDrawable(geo.p());
            part->setTransform(m_scaleTransform.p());

            // Set mapping from triangle face index to element index
            cvf::ref<RivFemPickSourceInfo> si = new RivFemPickSourceInfo(m_gridIdx, geoBuilder.triangleToElementMapper());
            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            
            // Set default effect
            caf::SurfaceEffectGenerator geometryEffgen(cvf::Color4f(cvf::Color3f::WHITE), caf::PO_1);
            cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();
            part->setEffect(geometryOnlyEffect.p());
            part->setEnableMask(surfaceBit);
            m_surfaceFaces = part;
        }
    }

    // Mesh geometry
    {
        m_surfaceGridLines = NULL; // To possibly free memory before adding the new stuff

        cvf::ref<cvf::DrawableGeo> geoMesh = geoBuilder.createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh " + cvf::String(m_gridIdx));
            part->setDrawable(geoMesh.p());

            part->setTransform(m_scaleTransform.p());
            part->updateBoundingBox();

            RiaPreferences* prefs = RiaApplication::instance()->preferences();

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
            eff = effGen.generateCachedEffect();

            part->setPriority(RivPartPriority::PartType::MeshLines);

            part->setEnableMask(meshSurfaceBit);
            part->setEffect(eff.p());
            m_surfaceGridLines = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != NULL);

    if(m_surfaceFaces.notNull()    ) model->addPart(m_surfaceFaces.p()    );
    if(m_surfaceGridLines.notNull()) model->addPart(m_surfaceGridLines.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartPartMgr::updateCellColor(cvf::Color4f color)
{
    if (m_surfaceFaces.isNull()) return;

    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(color, caf::PO_1);
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if (m_surfaceFaces.notNull()) m_surfaceFaces->setEffect(geometryOnlyEffect.p());

    if (color.a() < 1.0f)
    {
        // Set priority to make sure this transparent geometry are rendered last
        if (m_surfaceFaces.notNull()) m_surfaceFaces->setPriority(RivPartPriority::PartType::Transparent);
    }

    m_opacityLevel = color.a();
    m_defaultColor = color.toColor3f();

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    if (m_surfaceFaces.notNull())
    {
        caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
        eff = effGen.generateCachedEffect();
        m_surfaceGridLines->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFemPartPartMgr::updateCellResultColor(size_t timeStepIndex, RimGeoMechCellColors* cellResultColors)
{
    CVF_ASSERT(cellResultColors);

    cvf::ref<cvf::Color3ubArray> surfaceFacesColorArray;

    // Outer surface
    if (m_surfaceFaces.notNull())
    {

        const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

        RigGeoMechCaseData* caseData = cellResultColors->ownerCaseData();
        
        if (!caseData) return;

        RigFemResultAddress resVarAddress = cellResultColors->resultAddress();

        // Do a "Hack" to show elm nodal and not nodal POR results
        if (resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar") resVarAddress.resultPosType = RIG_ELEMENT_NODAL;

        const std::vector<float>& resultValues = caseData->femPartResults()->resultValues(resVarAddress, m_gridIdx, (int)timeStepIndex);

        const std::vector<size_t>* vxToResultMapping = NULL;
        int vxCount = 0;

        if (resVarAddress.resultPosType == RIG_NODAL)
        {
            vxToResultMapping = &(m_surfaceGenerator.quadVerticesToNodeIdxMapping());
        }
        else if (   resVarAddress.resultPosType == RIG_ELEMENT_NODAL 
                 || resVarAddress.resultPosType == RIG_INTEGRATION_POINT
                 || resVarAddress.resultPosType == RIG_FORMATION_NAMES)
        {
            vxToResultMapping = &(m_surfaceGenerator.quadVerticesToGlobalElmNodeIdx());
        }
        else if(resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE)
        {
            vxToResultMapping = &(m_surfaceGenerator.quadVerticesToGlobalElmFaceNodeIdx());
        }

        vxCount = static_cast<int>(vxToResultMapping->size());
        m_surfaceFacesTextureCoords->resize(vxCount);

        if (resultValues.size() == 0)
        {
            m_surfaceFacesTextureCoords->setAll(cvf::Vec2f(0.0, 1.0f));
        }
        else
        {
            cvf::Vec2f* rawPtr = m_surfaceFacesTextureCoords->ptr();

            #pragma omp parallel for schedule(dynamic)
            for (int quadStartIdx = 0; quadStartIdx < vxCount; quadStartIdx += 4)
            {
                float resultValue1 = resultValues[(*vxToResultMapping)[quadStartIdx + 0]];
                float resultValue2 = resultValues[(*vxToResultMapping)[quadStartIdx + 1]];
                float resultValue3 = resultValues[(*vxToResultMapping)[quadStartIdx + 2]];
                float resultValue4 = resultValues[(*vxToResultMapping)[quadStartIdx + 3]];

                if (    resultValue1 == HUGE_VAL || resultValue1 != resultValue1    // a != a is true for NAN's
                    ||  resultValue2 == HUGE_VAL || resultValue2 != resultValue2 
                    ||  resultValue3 == HUGE_VAL || resultValue3 != resultValue3 
                    ||  resultValue4 == HUGE_VAL || resultValue4 != resultValue4)
                {
                    rawPtr[quadStartIdx][1]       = 1.0f;
                    rawPtr[quadStartIdx + 1][1]   = 1.0f;
                    rawPtr[quadStartIdx + 2][1]   = 1.0f;
                    rawPtr[quadStartIdx + 3][1]   = 1.0f;
                }
                else
                {
                    rawPtr[quadStartIdx]        = mapper->mapToTextureCoord(resultValue1);
                    rawPtr[quadStartIdx + 1]    = mapper->mapToTextureCoord(resultValue2);
                    rawPtr[quadStartIdx + 2]    = mapper->mapToTextureCoord(resultValue3);
                    rawPtr[quadStartIdx + 3]    = mapper->mapToTextureCoord(resultValue4);
                }
            }
        }

        Rim3dView* view = NULL;
        cellResultColors->firstAncestorOrThisOfType(view);
        CVF_ASSERT(view);

        RivScalarMapperUtils::applyTextureResultsToPart(m_surfaceFaces.p(), 
                                                        m_surfaceFacesTextureCoords.p(), 
                                                        mapper, 
                                                        m_opacityLevel, 
                                                        caf::FC_NONE, 
                                                        view->isLightingDisabled());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFemPartPartMgr::~RivFemPartPartMgr()
{

}


