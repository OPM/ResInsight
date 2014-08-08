/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RivGridPartMgr.h"


#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafProgressInfo.h"
#include "cvfDrawableGeo.h"
#include "cvfMath.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderState_FF.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderSourceRepository.h"
#include "cvfStructGrid.h"
#include "cvfUniform.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimTernaryLegendConfig.h"
#include "RimWellCollection.h"
#include "RivCellEdgeEffectGenerator.h"
#include "RivSourceInfo.h"
#include "cvfStructGridGeometryGenerator.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RivResultToTextureMapper.h"
#include "RivTextureCoordsCreator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridPartMgr::RivGridPartMgr(const RigGridBase* grid, size_t gridIdx)
:   m_surfaceGenerator(grid), 
    m_gridIdx(gridIdx),
    m_grid(grid),
    m_surfaceFaceFilter(grid), 
    m_opacityLevel(1.0f),
    m_defaultColor(cvf::Color3::WHITE)
{
    CVF_ASSERT(grid);
    m_cellVisibility = new cvf::UByteArray;
    m_surfaceFacesTextureCoords = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::setCellVisibility(cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(m_scaleTransform.notNull());
    CVF_ASSERT(cellVisibilities);

    m_cellVisibility = cellVisibilities;

    m_surfaceGenerator.setCellVisibility(cellVisibilities);
    m_surfaceGenerator.addFaceVisibilityFilter(&m_surfaceFaceFilter);

    generatePartGeometry(m_surfaceGenerator);
}

void RivGridPartMgr::generatePartGeometry(cvf::StructGridGeometryGenerator& geoBuilder)
{
    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = geoBuilder.generateSurface();
        if (geo.notNull())
        {
            geo->computeNormals();

            if (useBufferObjects)
            {
                geo->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid " + cvf::String(static_cast<int>(m_gridIdx)));
            part->setId(m_gridIdx);       // !! For now, use grid index as part ID (needed for pick info)
            part->setDrawable(geo.p());
            part->setTransform(m_scaleTransform.p());

            // Set mapping from triangle face index to cell index
            cvf::ref<RivSourceInfo> si = new RivSourceInfo;
            si->m_cellFaceFromTriangleMapper = geoBuilder.triangleToCellFaceMapper();

            part->setSourceInfo(si.p());

            part->updateBoundingBox();
            
            // Set default effect
            caf::SurfaceEffectGenerator geometryEffgen(cvf::Color4f(cvf::Color3f::WHITE), caf::PO_1);
            cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();
            part->setEffect(geometryOnlyEffect.p());
            part->setEnableMask(surfaceBit);
            m_surfaceFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = geoBuilder.createMeshDrawable();
        if (geoMesh.notNull())
        {
            if (useBufferObjects)
            {
                geoMesh->setRenderMode(cvf::DrawableGeo::BUFFER_OBJECT);
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid mesh " + cvf::String(static_cast<int>(m_gridIdx)));
            part->setDrawable(geoMesh.p());

            part->setTransform(m_scaleTransform.p());
            part->updateBoundingBox();

            RiaPreferences* prefs = RiaApplication::instance()->preferences();

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
            eff = effGen.generateEffect();

            // Set priority to make sure fault lines are rendered first
            part->setPriority(10);

            part->setEnableMask(meshSurfaceBit);
            part->setEffect(eff.p());
            m_surfaceGridLines = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    CVF_ASSERT(model != NULL);

    if(m_surfaceFaces.notNull()    ) model->addPart(m_surfaceFaces.p()    );
    if(m_surfaceGridLines.notNull()) model->addPart(m_surfaceGridLines.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellColor(cvf::Color4f color)
{
    if (m_surfaceFaces.isNull()) return;

    // Set default effect
    caf::SurfaceEffectGenerator geometryEffgen(color, caf::PO_1);
    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateEffect();

    if (m_surfaceFaces.notNull()) m_surfaceFaces->setEffect(geometryOnlyEffect.p());

    if (color.a() < 1.0f)
    {
        // Set priority to make sure this transparent geometry are rendered last
        if (m_surfaceFaces.notNull()) m_surfaceFaces->setPriority(100);
    }

    m_opacityLevel = color.a();
    m_defaultColor = color.toColor3f();

    // Update mesh colors as well, in case of change
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect> eff;
    if (m_surfaceFaces.notNull())
    {
        caf::MeshEffectGenerator effGen(prefs->defaultGridLineColors());
        eff = effGen.generateEffect();
        m_surfaceGridLines->setEffect(eff.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    CVF_ASSERT(cellResultSlot);

    const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();
    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();

    cvf::ref<cvf::Color3ubArray> surfaceFacesColorArray;

    // Outer surface
    if (m_surfaceFaces.notNull())
    {
        if (cellResultSlot->isTernarySaturationSelected())
        {
            surfaceFacesColorArray = new cvf::Color3ubArray;

            RivTransmissibilityColorMapper::updateTernarySaturationColorArray(timeStepIndex, cellResultSlot, m_grid.p(), surfaceFacesColorArray.p(), m_surfaceGenerator.quadToCellFaceMapper());
        }
        else
        {
            RivTextureCoordsCreator texturer(cellResultSlot, 
                                             timeStepIndex, 
                                             m_grid->gridIndex(),  
                                             m_surfaceGenerator.quadToCellFaceMapper());

            texturer.createTextureCoords(m_surfaceFacesTextureCoords.p());
        }


        if (surfaceFacesColorArray.notNull()) // Ternary result
        {
            cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(m_surfaceFaces->drawable());
            if (dg)
            {
                dg->setColorArray(surfaceFacesColorArray.p());
            }

            cvf::ref<cvf::Effect> perVertexColorEffect = RivGridPartMgr::createPerVertexColoringEffect(m_opacityLevel);
            m_surfaceFaces->setEffect(perVertexColorEffect.p());

            m_surfaceFaces->setPriority(100);
        }
        else
        {
            applyTextureResultsToPart(m_surfaceFaces.p(), m_surfaceFacesTextureCoords.p(), mapper );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

cvf::ref<cvf::Effect> RivGridPartMgr::createScalarMapperEffect(const cvf::ScalarMapper* mapper)
{
    caf::PolygonOffset polygonOffset = caf::PO_1;
    caf::ScalarMapperEffectGenerator scalarEffgen(mapper, polygonOffset);
    scalarEffgen.setOpacityLevel(m_opacityLevel);
    cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();
    return scalarEffect;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

void RivGridPartMgr::applyTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const cvf::ScalarMapper* mapper)
{
    cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(part->drawable());
    if (dg) dg->setTextureCoordArray(textureCoords);

    cvf::ref<cvf::Effect> scalarEffect = createScalarMapperEffect(mapper);
    part->setEffect(scalarEffect.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot)
{
    updateCellEdgeResultColorOnPart(
        m_surfaceFaces.p(),
        &m_surfaceGenerator,
        timeStepIndex, cellResultSlot, cellEdgeResultSlot);

}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridPartMgr::updateCellEdgeResultColorOnPart(   cvf::Part* facePart, 
                                                        cvf::StructGridGeometryGenerator* surfaceGenerator, 
                                                        size_t timeStepIndex, 
                                                        RimResultSlot* cellResultSlot, 
                                                        RimCellEdgeResultSlot* cellEdgeResultSlot)
{
    if (facePart)
    {
        cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(facePart->drawable());
        if (dg) 
        {
            RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultSlot, cellEdgeResultSlot, 
                surfaceGenerator, dg, m_grid->gridIndex(), m_opacityLevel );

            cvf::ScalarMapper* cellScalarMapper = NULL;
            if (cellResultSlot->hasResult()) cellScalarMapper = cellResultSlot->legendConfig()->scalarMapper();

            CellEdgeEffectGenerator cellFaceEffectGen(cellEdgeResultSlot->legendConfig()->scalarMapper(), cellScalarMapper);
            cellFaceEffectGen.setOpacityLevel(m_opacityLevel);
            cellFaceEffectGen.setDefaultCellColor(m_defaultColor);

            cvf::ref<cvf::Effect> eff = cellFaceEffectGen.generateEffect();

            facePart->setEffect(eff.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridPartMgr::~RivGridPartMgr()
{
#if 0
    if (m_faultFaces.notNull()) m_faultFaces->deleteOrReleaseOpenGLResources();
    if (m_faultGridLines.notNull()) m_faultGridLines->deleteOrReleaseOpenGLResources();
    if (m_surfaceGridLines.notNull()) m_surfaceGridLines->deleteOrReleaseOpenGLResources();
    if (m_surfaceFaces.notNull()) m_surfaceFaces->deleteOrReleaseOpenGLResources();
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivGridPartMgr::createPerVertexColoringEffect(float opacity)
{
    cvf::ref<cvf::Effect> colorArrayEffect = new cvf::Effect;

    if (RiaApplication::instance()->useShaders())
    {
        cvf::ShaderProgramGenerator gen("PerVertexColor", cvf::ShaderSourceProvider::instance());
        gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(cvf::ShaderSourceRepository::src_VaryingColorGlobalAlpha);
        gen.addFragmentCode(caf::CommonShaderSources::light_AmbientDiffuse());
        gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

        cvf::ref<cvf::ShaderProgram> m_shaderProg = gen.generate();
        m_shaderProg->setDefaultUniform(new cvf::UniformFloat("u_alpha", opacity));

        colorArrayEffect->setShaderProgram(m_shaderProg.p());
    }
    else
    {
        cvf::ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF(cvf::Color3::BLUE);
        mat->setAlpha(opacity);
        mat->enableColorMaterial(true);
        colorArrayEffect->setRenderState(mat.p());

        cvf::ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
        lighting->enableTwoSided(true);
        colorArrayEffect->setRenderState(lighting.p());
    }
    
    // Simple transparency
    if (opacity < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        colorArrayEffect->setRenderState(blender.p());
    }
    
    caf::PolygonOffset polygonOffset = caf::PO_1;
    cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = caf::EffectGenerator::createAndConfigurePolygonOffsetRenderState(polygonOffset);
    colorArrayEffect->setRenderState(polyOffset.p());

    return colorArrayEffect;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTransmissibilityColorMapper::updateCombinedTransmissibilityTextureCoordinates(RimResultSlot* cellResultSlot,
    const RigGridBase* grid,
    cvf::Vec2fArray* textureCoords, 
    const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper)
{
    const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();
    if (!mapper) return;

    const RimReservoirCellResultsStorage* gridCellResults = cellResultSlot->currentGridCellResults();
    if (!gridCellResults) return;

    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
    if (!eclipseCase) return;

    size_t tranPosXScalarSetIndex, tranPosYScalarSetIndex, tranPosZScalarSetIndex;
    if (!gridCellResults->cellResults()->findTransmissibilityResults(tranPosXScalarSetIndex, tranPosYScalarSetIndex, tranPosZScalarSetIndex)) return;

    // If the result is static, only read that.
    size_t resTimeStepIdx = 0;

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectTranX = eclipseCase->resultAccessor(grid, porosityModel, resTimeStepIdx, tranPosXScalarSetIndex);
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectTranY = eclipseCase->resultAccessor(grid, porosityModel, resTimeStepIdx, tranPosYScalarSetIndex);
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectTranZ = eclipseCase->resultAccessor(grid, porosityModel, resTimeStepIdx, tranPosZScalarSetIndex);


    int quadCount = static_cast<int>(quadToCellFaceMapper->quadCount());
    size_t numVertices = quadCount*4;

    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    double cellScalarValue;
    cvf::Vec2f texCoord;

#pragma omp parallel for private(texCoord, cellScalarValue)
    for (int quadIdx = 0; quadIdx < quadCount; quadIdx++)
    {
        cellScalarValue = HUGE_VAL;

        size_t cellIndex = quadToCellFaceMapper->cellIndex(quadIdx);
        cvf::StructGridInterface::FaceType cellFace = quadToCellFaceMapper->cellFace(quadIdx);

        switch (cellFace)
        {
        case cvf::StructGridInterface::POS_I:
            {
                cellScalarValue = dataAccessObjectTranX->cellScalar(cellIndex);
            }
            break;
        case cvf::StructGridInterface::NEG_I:
            {
                size_t i, j, k, neighborGridCellIdx;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

                if(grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_I, &neighborGridCellIdx))
                {
                    cellScalarValue = dataAccessObjectTranX->cellScalar(neighborGridCellIdx);
                }
            }
            break;
        case cvf::StructGridInterface::POS_J:
            {
                cellScalarValue = dataAccessObjectTranY->cellScalar(cellIndex);
            }
            break;
        case cvf::StructGridInterface::NEG_J:
            {
                size_t i, j, k, neighborGridCellIdx;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

                if(grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_J, &neighborGridCellIdx))
                {
                    cellScalarValue = dataAccessObjectTranY->cellScalar(neighborGridCellIdx);
                }
            }
            break;
        case cvf::StructGridInterface::POS_K:
            {
                cellScalarValue = dataAccessObjectTranZ->cellScalar(cellIndex);
            }
            break;
        case cvf::StructGridInterface::NEG_K:
            {
                size_t i, j, k, neighborGridCellIdx;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

                if(grid->cellIJKNeighbor(i, j, k, cvf::StructGridInterface::NEG_K, &neighborGridCellIdx))
                {
                    cellScalarValue = dataAccessObjectTranZ->cellScalar(neighborGridCellIdx);
                }
            }
            break;
        default:
            CVF_ASSERT(false);
        }


        texCoord = mapper->mapToTextureCoord(cellScalarValue);
        if (cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[quadIdx*4 + j] = texCoord;
        }
    }

}


//--------------------------------------------------------------------------------------------------
/// Helper class used to provide zero for all cells
/// This way we can avoid to test if a StructGridScalarDataAccess object is valid before reading out the value.
//--------------------------------------------------------------------------------------------------
class ScalarDataAccessZeroForAllCells : public cvf::StructGridScalarDataAccess
{
public:
    virtual double cellScalar(size_t cellIndex) const
    {
        return 0.0;
    }
    virtual void setCellScalar(size_t cellIndex, double value)
    {
    }
};


//--------------------------------------------------------------------------------------------------
/// Creates and assigns a ternary saturation color for all four vertices of a quad representing a cell face
///
/// Loads ternary saturation results SOIL, SWAT and SGAS
/// If any of these are not present, the values for a missing component is set to 0.0
//--------------------------------------------------------------------------------------------------
void RivTransmissibilityColorMapper::updateTernarySaturationColorArray(size_t timeStepIndex, RimResultSlot* cellResultSlot, 
            const RigGridBase* grid, cvf::Color3ubArray* colorArray, 
            const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper)
{
    RimReservoirCellResultsStorage* gridCellResults = cellResultSlot->currentGridCellResults();
    if (!gridCellResults) return;

    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
    if (!eclipseCase) return;

    size_t soilScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
    size_t sgasScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
    size_t swatScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

    double soilMin = 0.0;
    double soilMax = 1.0;
    double sgasMin = 0.0;
    double sgasMax = 1.0;
    double swatMin = 0.0;
    double swatMax = 1.0;

    cellResultSlot->ternaryLegendConfig()->ternaryRanges(soilMin, soilMax, sgasMin, sgasMax, swatMin, swatMax);

    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectSoil = eclipseCase->resultAccessor(grid, porosityModel, timeStepIndex, soilScalarSetIndex);
    if (dataAccessObjectSoil.isNull())
    {
        dataAccessObjectSoil = new ScalarDataAccessZeroForAllCells;
    }

    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectSgas = eclipseCase->resultAccessor(grid, porosityModel, timeStepIndex, sgasScalarSetIndex);
    if (dataAccessObjectSgas.isNull())
    {
        dataAccessObjectSgas = new ScalarDataAccessZeroForAllCells;
    }
    
    cvf::ref<cvf::StructGridScalarDataAccess> dataAccessObjectSwat = eclipseCase->resultAccessor(grid, porosityModel, timeStepIndex, swatScalarSetIndex);
    if (dataAccessObjectSwat.isNull())
    {
        dataAccessObjectSwat = new ScalarDataAccessZeroForAllCells;
    }

    double soilRange = soilMax - soilMin;
    double soilFactor = 255.0 / soilRange;

    double sgasRange = sgasMax - sgasMin;
    double sgasFactor = 255.0 / sgasRange;

    double swatRange = swatMax - swatMin;
    double swatFactor = 255.0 / swatRange;

    size_t numVertices = quadToCellFaceMapper->quadCount()*4;

    colorArray->resize(numVertices);

    cvf::Color3ub ternaryColorByte;
    double v, vNormalized;

#pragma omp parallel for private(ternaryColorByte, v, vNormalized)
    for (int quadIdx = 0; quadIdx < static_cast<int>(quadToCellFaceMapper->quadCount()); quadIdx++)
    {
        size_t gridCellIndex = quadToCellFaceMapper->cellIndex(quadIdx);

        {
            v = dataAccessObjectSgas->cellScalar(gridCellIndex);
            vNormalized = (v - sgasMin) * sgasFactor;

            vNormalized = cvf::Math::clamp(vNormalized, 0.0, 255.0);
            ternaryColorByte.r() = vNormalized;
        }

        {
            v = dataAccessObjectSoil->cellScalar(gridCellIndex);
            vNormalized = (v - soilMin) * soilFactor;

            vNormalized = cvf::Math::clamp(vNormalized, 0.0, 255.0);
            ternaryColorByte.g() = vNormalized;
        }

        {
            v = dataAccessObjectSwat->cellScalar(gridCellIndex);
            vNormalized = (v - swatMin) * swatFactor;

            vNormalized = cvf::Math::clamp(vNormalized, 0.0, 255.0);
            ternaryColorByte.b() = vNormalized;
        }

        size_t j;
        for (j = 0; j < 4; j++)
        {
            colorArray->set(quadIdx*4 + j, ternaryColorByte);
        }
    }
}
