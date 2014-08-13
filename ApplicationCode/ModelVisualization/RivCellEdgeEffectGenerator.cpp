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

#include "RiaStdInclude.h"

#include "RivCellEdgeEffectGenerator.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigCellEdgeResultAccessor.h"
#include "RigGridBase.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimWellCollection.h"

#include "RivTernaryScalarMapper.h"
#include "RimTernaryLegendConfig.h"

#include "cvfStructGridGeometryGenerator.h"
#include "cvfqtUtils.h"
#include "RivTernaryTextureCoordsCreator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCellEdgeGeometryGenerator::addCellEdgeResultsToDrawableGeo(
    size_t timeStepIndex, 
    RimResultSlot* cellResultSlot, 
    RimCellEdgeResultSlot* cellEdgeResultSlot, 
    cvf::StructGridGeometryGenerator* generator, 
    cvf::DrawableGeo* geo, 
    size_t gridIndex, 
    float opacityLevel)
{
	RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
	CVF_ASSERT(eclipseCase != NULL);

	const RigGridBase* grid = dynamic_cast<const RigGridBase*>(generator->activeGrid());
	CVF_ASSERT(grid != NULL);
	
	// Create result access objects
	
	cvf::ref<RigResultAccessor> cellCenterDataAccessObject = createCellCenterResultAccessor(cellResultSlot, timeStepIndex, eclipseCase, grid);
	cvf::ref<RigResultAccessor> cellEdgeResultAccessor = createCellEdgeCenterResultAccessor(cellResultSlot, cellEdgeResultSlot, timeStepIndex, eclipseCase, grid);
	
	
	const cvf::StructGridQuadToCellFaceMapper* quadToCellFace = generator->quadToCellFaceMapper();

    size_t vertexCount = geo->vertexArray()->size();
    size_t quadCount = vertexCount / 4;

    cvf::ref<cvf::Vec2fArray> localCoords = new cvf::Vec2fArray;
    localCoords->resize(vertexCount);

    cvf::ref<cvf::IntArray> faceIndexArray = new cvf::IntArray;
    faceIndexArray->resize(vertexCount);

    cvf::ref<cvf::FloatArray> cellColorTextureCoordArray = new cvf::FloatArray;
    cellColorTextureCoordArray->resize(vertexCount);

    // Build six cell face color arrays
    cvf::Collection<cvf::FloatArray> cellEdgeColorTextureCoordsArrays;
    size_t idx;
    for (idx = 0; idx < 6; idx++)
    {
        cvf::ref<cvf::FloatArray> colorArray = new cvf::FloatArray;
        colorArray->resize(vertexCount);
        cellEdgeColorTextureCoordsArrays.push_back(colorArray.p());
    }

	cvf::ScalarMapper* cellResultScalarMapper = cellResultSlot->legendConfig()->scalarMapper();
    cvf::ScalarMapper* edgeResultScalarMapper = cellEdgeResultSlot->legendConfig()->scalarMapper();
	
    double ignoredScalarValue = cellEdgeResultSlot->ignoredScalarValue();

    const std::vector<cvf::ubyte>* isWellPipeVisible = NULL;     
    cvf::ref<cvf::UIntArray>       gridCellToWellindexMap;

    if (opacityLevel < 1.0f)
    {
        isWellPipeVisible = &(cellResultSlot->reservoirView()->wellCollection()->isWellPipesVisible(timeStepIndex));
        gridCellToWellindexMap = eclipseCase->gridCellToWellIndex( gridIndex );
    }

#pragma omp parallel for
    for (int quadIdx = 0; quadIdx < static_cast<int>(quadCount); quadIdx++)
    {
        localCoords->set(quadIdx * 4 + 0, cvf::Vec2f(0, 0));
        localCoords->set(quadIdx * 4 + 1, cvf::Vec2f(1, 0));
        localCoords->set(quadIdx * 4 + 2, cvf::Vec2f(1, 1));
        localCoords->set(quadIdx * 4 + 3, cvf::Vec2f(0, 1));

        faceIndexArray->set(quadIdx * 4 + 0, quadToCellFace->cellFace(quadIdx) );
        faceIndexArray->set(quadIdx * 4 + 1, quadToCellFace->cellFace(quadIdx) );
        faceIndexArray->set(quadIdx * 4 + 2, quadToCellFace->cellFace(quadIdx) );
        faceIndexArray->set(quadIdx * 4 + 3, quadToCellFace->cellFace(quadIdx) );

		size_t cellIndex = quadToCellFace->cellIndex(quadIdx);
		{
			cvf::StructGridInterface::FaceType cellFace = quadToCellFace->cellFace(quadIdx);
			double scalarValue = cellCenterDataAccessObject->cellFaceScalar(cellIndex, cellFace);

			{
				float cellColorTextureCoord = 0.5f; // If no results exists, the texture will have a special color
				if (scalarValue != HUGE_VAL)
				{
					cellColorTextureCoord = cellResultScalarMapper->mapToTextureCoord(scalarValue)[0];
					// If we are dealing with wellcells, the default is transparent.
					// we need to make cells opaque if there are no wellpipe through them.
					if (opacityLevel < 1.0f)
					{
						cvf::uint wellIndex = gridCellToWellindexMap->get(cellIndex);
						if (wellIndex != cvf::UNDEFINED_UINT)
						{
							if (!(*isWellPipeVisible)[wellIndex])
							{
								cellColorTextureCoord += 2.0f; // The shader must interpret values in the range 2-3 as "opaque"
							}
						}
					}
				}
				else
				{
					cellColorTextureCoord = -1.0f; // Undefined texture coord. Shader handles this.
				}

				cellColorTextureCoordArray->set(quadIdx * 4 + 0, cellColorTextureCoord);
				cellColorTextureCoordArray->set(quadIdx * 4 + 1, cellColorTextureCoord);
				cellColorTextureCoordArray->set(quadIdx * 4 + 2, cellColorTextureCoord);
				cellColorTextureCoordArray->set(quadIdx * 4 + 3, cellColorTextureCoord);
			}
		}


        float edgeColor;
		for (size_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            edgeColor = -1.0f; // Undefined texture coord. Shader handles this.

			double scalarValue = cellEdgeResultAccessor->cellFaceScalar(cellIndex, static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx));

            if (scalarValue != HUGE_VAL && scalarValue != ignoredScalarValue)
            {
                edgeColor = edgeResultScalarMapper->mapToTextureCoord(scalarValue)[0];
            }

            cvf::FloatArray* colArr = cellEdgeColorTextureCoordsArrays.at(cubeFaceIdx);

            colArr->set(quadIdx * 4 + 0, edgeColor);
            colArr->set(quadIdx * 4 + 1, edgeColor);
            colArr->set(quadIdx * 4 + 2, edgeColor);
            colArr->set(quadIdx * 4 + 3, edgeColor);
        }
    }

    geo->setVertexAttribute(new cvf::Vec2fVertexAttribute("a_localCoord", localCoords.p()));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorCell", cellColorTextureCoordArray.p()));

    cvf::ref<cvf::IntVertexAttributeDirect> faceIntAttribute =  new cvf::IntVertexAttributeDirect("a_face", faceIndexArray.p());
    geo->setVertexAttribute(faceIntAttribute.p());

    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosI", cellEdgeColorTextureCoordsArrays.at(0)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegI", cellEdgeColorTextureCoordsArrays.at(1)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosJ", cellEdgeColorTextureCoordsArrays.at(2)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegJ", cellEdgeColorTextureCoordsArrays.at(3)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosK", cellEdgeColorTextureCoordsArrays.at(4)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegK", cellEdgeColorTextureCoordsArrays.at(5)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RivCellEdgeGeometryGenerator::createCellEdgeCenterResultAccessor(
	RimResultSlot* cellResultSlot,
	RimCellEdgeResultSlot* cellEdgeResultSlot,
	size_t timeStepIndex,
	RigCaseData* eclipseCase,
	const RigGridBase* grid)
{
	cvf::ref<RigCellEdgeResultAccessor> cellEdgeResultAccessor = new RigCellEdgeResultAccessor();
	{
		size_t resultIndices[6];
		cellEdgeResultSlot->gridScalarIndices(resultIndices);
		RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

		size_t cubeFaceIdx;
		for (cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
		{
			// Assuming static values to be mapped onto cell edge, always using time step zero
			cvf::ref<RigResultAccessor> daObj = RigResultAccessorFactory::createResultAccessor(eclipseCase, grid->gridIndex(), porosityModel, 0, resultIndices[cubeFaceIdx]);
			cellEdgeResultAccessor->setDataAccessObjectForFace(static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx), daObj.p());
		}
	}

	return cellEdgeResultAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCellEdgeGeometryGenerator::addTernaryCellEdgeResultsToDrawableGeo(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot, cvf::StructGridGeometryGenerator* generator, cvf::DrawableGeo* geo, size_t gridIndex, float opacityLevel)
{
	RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();
	CVF_ASSERT(eclipseCase != NULL);

	const RigGridBase* grid = dynamic_cast<const RigGridBase*>(generator->activeGrid());
	CVF_ASSERT(grid != NULL);

	cvf::ref<RigResultAccessor> cellEdgeResultAccessor = createCellEdgeCenterResultAccessor(cellResultSlot, cellEdgeResultSlot, timeStepIndex, eclipseCase, grid);

	const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper = generator->quadToCellFaceMapper();

	size_t vertexCount = geo->vertexArray()->size();
	size_t quadCount = vertexCount / 4;

	cvf::ref<cvf::Vec2fArray> localCoords = new cvf::Vec2fArray;
	localCoords->resize(vertexCount);

	cvf::ref<cvf::IntArray> faceIndexArray = new cvf::IntArray;
	faceIndexArray->resize(vertexCount);

	cvf::ref<cvf::Vec2fArray> vCellColorTextureCoordArray = new cvf::Vec2fArray;
	vCellColorTextureCoordArray->resize(vertexCount);

	// Build six cell face color arrays
	cvf::Collection<cvf::FloatArray> cellEdgeColorTextureCoordsArrays;
	size_t idx;
	for (idx = 0; idx < 6; idx++)
	{
		cvf::ref<cvf::FloatArray> colorArray = new cvf::FloatArray;
		colorArray->resize(vertexCount);
		cellEdgeColorTextureCoordsArrays.push_back(colorArray.p());
	}

	RivTernaryScalarMapper* ternaryCellResultScalarMapper = cellResultSlot->ternaryLegendConfig()->scalarMapper();
	cvf::ScalarMapper* edgeResultScalarMapper = cellEdgeResultSlot->legendConfig()->scalarMapper();

	double ignoredScalarValue = cellEdgeResultSlot->ignoredScalarValue();

	RivTernaryTextureCoordsCreator texturer(cellResultSlot, cellResultSlot->ternaryLegendConfig(),
		timeStepIndex,
		grid->gridIndex(),
		quadToCellFaceMapper);

	texturer.createTextureCoords(vCellColorTextureCoordArray.p());

#pragma omp parallel for
	for (int quadIdx = 0; quadIdx < static_cast<int>(quadCount); quadIdx++)
	{
		localCoords->set(quadIdx * 4 + 0, cvf::Vec2f(0, 0));
		localCoords->set(quadIdx * 4 + 1, cvf::Vec2f(1, 0));
		localCoords->set(quadIdx * 4 + 2, cvf::Vec2f(1, 1));
		localCoords->set(quadIdx * 4 + 3, cvf::Vec2f(0, 1));

		faceIndexArray->set(quadIdx * 4 + 0, quadToCellFaceMapper->cellFace(quadIdx));
		faceIndexArray->set(quadIdx * 4 + 1, quadToCellFaceMapper->cellFace(quadIdx));
		faceIndexArray->set(quadIdx * 4 + 2, quadToCellFaceMapper->cellFace(quadIdx));
		faceIndexArray->set(quadIdx * 4 + 3, quadToCellFaceMapper->cellFace(quadIdx));

		size_t cellIndex = quadToCellFaceMapper->cellIndex(quadIdx);

		float edgeColor;
		for (size_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
		{
			edgeColor = -1.0f; // Undefined texture coord. Shader handles this.

			double scalarValue = cellEdgeResultAccessor->cellFaceScalar(cellIndex, static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx));

			if (scalarValue != HUGE_VAL && scalarValue != ignoredScalarValue)
			{
				edgeColor = edgeResultScalarMapper->mapToTextureCoord(scalarValue)[0];
			}

			cvf::FloatArray* colArr = cellEdgeColorTextureCoordsArrays.at(cubeFaceIdx);

			colArr->set(quadIdx * 4 + 0, edgeColor);
			colArr->set(quadIdx * 4 + 1, edgeColor);
			colArr->set(quadIdx * 4 + 2, edgeColor);
			colArr->set(quadIdx * 4 + 3, edgeColor);
		}
	}

	geo->setVertexAttribute(new cvf::Vec2fVertexAttribute("a_localCoord", localCoords.p()));
	geo->setVertexAttribute(new cvf::Vec2fVertexAttribute("a_cellTextureCoord", vCellColorTextureCoordArray.p()));

	cvf::ref<cvf::IntVertexAttributeDirect> faceIntAttribute = new cvf::IntVertexAttributeDirect("a_face", faceIndexArray.p());
	geo->setVertexAttribute(faceIntAttribute.p());

	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosI", cellEdgeColorTextureCoordsArrays.at(0)));
	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegI", cellEdgeColorTextureCoordsArrays.at(1)));
	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosJ", cellEdgeColorTextureCoordsArrays.at(2)));
	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegJ", cellEdgeColorTextureCoordsArrays.at(3)));
	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosK", cellEdgeColorTextureCoordsArrays.at(4)));
	geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegK", cellEdgeColorTextureCoordsArrays.at(5)));

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RivCellEdgeGeometryGenerator::createCellCenterResultAccessor(RimResultSlot* cellResultSlot, size_t timeStepIndex, RigCaseData* eclipseCase, const RigGridBase* grid)
{
	cvf::ref<RigResultAccessor> resultAccessor = NULL;

	if (cellResultSlot->hasResult())
	{
		if (!cellResultSlot->hasDynamicResult())
		{
			// Static result values are located at time step 0
			timeStepIndex = 0;
		}

		RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());
		resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, grid->gridIndex(), porosityModel, timeStepIndex, cellResultSlot->resultVariable());
	}

	if (resultAccessor.isNull())
	{
		resultAccessor = new RigHugeValResultAccessor;
	}

	return resultAccessor;
}







//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellEdgeEffectGenerator::CellEdgeEffectGenerator(const cvf::ScalarMapper* edgeScalarMapper)
{
    CVF_ASSERT(edgeScalarMapper != NULL);

    m_edgeScalarMapper = edgeScalarMapper;

    m_cullBackfaces = false;
    m_opacityLevel = 1.0f;
    m_defaultCellColor = cvf::Color3f(cvf::Color3::WHITE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::setScalarMapper(const cvf::ScalarMapper* cellScalarMapper)
{
	m_cellScalarMapper = cellScalarMapper;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::setTernaryScalarMapper(const RivTernaryScalarMapper* ternaryScalarMapper)
{
	m_ternaryCellScalarMapper = ternaryScalarMapper;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CellEdgeEffectGenerator::isEqual(const EffectGenerator* other) const
{
    const CellEdgeEffectGenerator* otherCellFaceEffectGenerator = dynamic_cast<const CellEdgeEffectGenerator*>(other);
    
    if (otherCellFaceEffectGenerator 
        && m_cellScalarMapper.p() == otherCellFaceEffectGenerator->m_cellScalarMapper.p() 
        && m_edgeScalarMapper.p() == otherCellFaceEffectGenerator->m_edgeScalarMapper.p()
		&& m_ternaryCellScalarMapper.p() == otherCellFaceEffectGenerator->m_ternaryCellScalarMapper.p()
		&& m_cullBackfaces == otherCellFaceEffectGenerator->m_cullBackfaces
        && m_opacityLevel         == otherCellFaceEffectGenerator->m_opacityLevel
        && m_undefinedColor       == otherCellFaceEffectGenerator->m_undefinedColor
        && m_defaultCellColor     == otherCellFaceEffectGenerator->m_defaultCellColor
        )
    {
        cvf::ref<cvf::TextureImage> texImg2 = new cvf::TextureImage;

        if (otherCellFaceEffectGenerator->m_edgeScalarMapper.notNull())
        {
            otherCellFaceEffectGenerator->m_edgeScalarMapper->updateTexture(texImg2.p());
            if (!caf::ScalarMapperEffectGenerator::isImagesEqual(m_edgeTextureImage.p(), texImg2.p())) return false;
        }

        if (otherCellFaceEffectGenerator->m_cellScalarMapper.notNull())
        {
            otherCellFaceEffectGenerator->m_cellScalarMapper->updateTexture(texImg2.p());
            if (!caf::ScalarMapperEffectGenerator::isImagesEqual(m_cellTextureImage.p(), texImg2.p())) return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::EffectGenerator* CellEdgeEffectGenerator::copy() const
{
    CellEdgeEffectGenerator * newEffect = new CellEdgeEffectGenerator(m_edgeScalarMapper.p());
	newEffect->setScalarMapper(m_cellScalarMapper.p());
	newEffect->setTernaryScalarMapper(m_ternaryCellScalarMapper.p());
    newEffect->m_edgeTextureImage = m_edgeTextureImage;
    newEffect->m_cellTextureImage = m_cellTextureImage;

    newEffect->setOpacityLevel(m_opacityLevel);
    newEffect->setCullBackfaces(m_cullBackfaces);
    newEffect->setUndefinedColor(m_undefinedColor);
    newEffect->setDefaultCellColor(m_defaultCellColor);

    return newEffect;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::updateForShaderBasedRendering(cvf::Effect* effect) const
{
	cvf::ref<cvf::Effect> eff = effect;

	// Set up shader program

	cvf::ShaderProgramGenerator shaderGen("CellEdgeFaceShaderProgramGenerator", cvf::ShaderSourceProvider::instance());
	{
		QFile data(":/Shader/fs_CellFace.glsl");
		if (data.open(QFile::ReadOnly))
		{
			QTextStream in(&data);

			QString data = in.readAll();
			cvf::String cvfString = cvfqt::Utils::toString(data);

			shaderGen.addFragmentCode(cvfString);
		}
	}

	if (m_ternaryCellScalarMapper.notNull())
	{
		{
			QFile data(":/Shader/vs_2dTextureCellFace.glsl");
			if (data.open(QFile::ReadOnly))
			{
				QTextStream in(&data);

				QString data = in.readAll();
				cvf::String cvfString = cvfqt::Utils::toString(data);

				shaderGen.addVertexCode(cvfString);
			}
		}
	}
	else
	{
		{
			QFile data(":/Shader/vs_CellFace.glsl");
			if (data.open(QFile::ReadOnly))
			{
				QTextStream in(&data);

				QString data = in.readAll();
				cvf::String cvfString = cvfqt::Utils::toString(data);

				shaderGen.addVertexCode(cvfString);
			}
		}
	}

    shaderGen.addFragmentCode(caf::CommonShaderSources::light_AmbientDiffuse());
    shaderGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Standard);

    cvf::ref<cvf::ShaderProgram> prog = shaderGen.generate();
    eff->setShaderProgram(prog.p());
 
    // Set up textures

    m_edgeTextureImage = new cvf::TextureImage;
    m_cellTextureImage = new cvf::TextureImage;

    cvf::ref<cvf::TextureImage> modifiedCellTextImage;
    m_edgeScalarMapper->updateTexture(m_edgeTextureImage.p());
	if (m_ternaryCellScalarMapper.notNull())
	{
		m_ternaryCellScalarMapper->updateTexture(m_cellTextureImage.p(), m_opacityLevel);
		modifiedCellTextImage = m_cellTextureImage;
	}
    else if (m_cellScalarMapper.notNull()) 
    {
        m_cellScalarMapper->updateTexture(m_cellTextureImage.p());
        modifiedCellTextImage = caf::ScalarMapperEffectGenerator::addAlphaAndUndefStripes(m_cellTextureImage.p(), m_undefinedColor, m_opacityLevel);
    }
    else 
    {
        modifiedCellTextImage = new cvf::TextureImage;
        modifiedCellTextImage->allocate(2,1);
        modifiedCellTextImage->fill(cvf::Color4ub(cvf::Color4f(m_defaultCellColor, m_opacityLevel)));
    }

    cvf::ref<cvf::Texture> edgeTexture = new cvf::Texture(m_edgeTextureImage.p());
    cvf::ref<cvf::Texture> cellTexture = new cvf::Texture(modifiedCellTextImage.p());

    cvf::ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    cvf::ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(edgeTexture.p(), sampler.p(), "u_edgeTexture2D");
    texBind->addBinding(cellTexture.p(), sampler.p(), "u_cellTexture2D");
    eff->setRenderState(texBind.p());

    // Polygon offset

    if (true)
    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        eff->setRenderState(polyOffset.p());
    }

    // Simple transparency
    if (m_opacityLevel < 1.0f)
    {
        cvf::ref<cvf::RenderStateBlending> blender = new cvf::RenderStateBlending;
        blender->configureTransparencyBlending();
        eff->setRenderState(blender.p());
    }

    // Backface culling

    if (m_cullBackfaces)
    {
        cvf::ref<cvf::RenderStateCullFace> faceCulling = new cvf::RenderStateCullFace;
        eff->setRenderState(faceCulling.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellEdgeEffectGenerator::updateForFixedFunctionRendering(cvf::Effect* effect) const
{
    caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(cvf::Color3f::CRIMSON), caf::PO_1);

    surfaceGen.updateEffect(effect);
}


