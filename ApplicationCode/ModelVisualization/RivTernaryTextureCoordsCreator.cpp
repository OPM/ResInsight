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

#include "RivTernaryTextureCoordsCreator.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigPipeInCellEvaluator.h"
#include "RigResultAccessorFactory.h"
#include "RigTernaryResultAccessor2d.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTernaryLegendConfig.h"

#include "RivTernaryResultToTextureMapper.h"
#include "RivTernaryScalarMapper.h"

#include "cvfStructGridGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernaryTextureCoordsCreator::RivTernaryTextureCoordsCreator(
    RimEclipseCellColors* cellResultColors, 
    RimTernaryLegendConfig* ternaryLegendConfig,
    size_t timeStepIndex, 
    size_t gridIndex, 
    const cvf::StructGridQuadToCellFaceMapper* quadMapper)
{
    CVF_ASSERT(quadMapper);
    m_quadMapper = quadMapper;

    RigEclipseCaseData* eclipseCase = cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData();

    size_t resTimeStepIdx = timeStepIndex;

    if (cellResultColors->hasStaticResult()) resTimeStepIdx = 0;

    RiaDefines::PorosityModelType porosityModel = cellResultColors->porosityModel();

    cvf::ref<RigResultAccessor> soil = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SOIL");
    cvf::ref<RigResultAccessor> sgas = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SGAS");
    cvf::ref<RigResultAccessor> swat = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SWAT");

    m_resultAccessor = new RigTernaryResultAccessor();
    m_resultAccessor->setTernaryResultAccessors(soil.p(), sgas.p(), swat.p());

    cvf::ref<RigPipeInCellEvaluator> pipeInCellEval = 
        new RigPipeInCellEvaluator(cellResultColors->reservoirView()->wellCollection()->resultWellGeometryVisibilities(timeStepIndex),
                                   eclipseCase->gridCellToResultWellIndex(gridIndex));

    const RivTernaryScalarMapper* mapper = ternaryLegendConfig->scalarMapper();

    m_texMapper = new RivTernaryResultToTextureMapper(mapper, pipeInCellEval.p());
    CVF_ASSERT(m_texMapper.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernaryTextureCoordsCreator::RivTernaryTextureCoordsCreator(
    RimEclipseCellColors* cellResultColors, 
    RimTernaryLegendConfig* ternaryLegendConfig, 
    size_t timeStepIndex)
    : m_quadMapper(NULL)
{
    RigEclipseCaseData* eclipseCase = cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData();

    size_t resTimeStepIdx = timeStepIndex;

    if (cellResultColors->hasStaticResult()) resTimeStepIdx = 0;

    RiaDefines::PorosityModelType porosityModel = cellResultColors->porosityModel();

    size_t gridIndex = 0;
    cvf::ref<RigResultAccessor> soil = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SOIL");
    cvf::ref<RigResultAccessor> sgas = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SGAS");
    cvf::ref<RigResultAccessor> swat = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, "SWAT");

    m_resultAccessor = new RigTernaryResultAccessor();
    m_resultAccessor->setTernaryResultAccessors(soil.p(), sgas.p(), swat.p());

    const RivTernaryScalarMapper* mapper = ternaryLegendConfig->scalarMapper();

    // Create a texture mapper without detecting transparency using RigPipeInCellEvaluator
    m_texMapper = new RivTernaryResultToTextureMapper(mapper, NULL);
    CVF_ASSERT(m_texMapper.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryTextureCoordsCreator::createTextureCoords(cvf::Vec2fArray* quadTextureCoords)
{
    CVF_ASSERT(m_quadMapper.notNull());
    createTextureCoords(quadTextureCoords, m_quadMapper.p(), m_resultAccessor.p(), m_texMapper.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryTextureCoordsCreator::createTextureCoords(cvf::Vec2fArray* triTextureCoords, const std::vector<size_t>& triangleToCellIdx)
{
    CVF_ASSERT(m_quadMapper.isNull());

    createTextureCoords(triTextureCoords, triangleToCellIdx, m_resultAccessor.p(), m_texMapper.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryTextureCoordsCreator::createTextureCoords(
    cvf::Vec2fArray* quadTextureCoords,
    const cvf::StructGridQuadToCellFaceMapper* quadMapper,
    const RigTernaryResultAccessor* resultAccessor,
    const RivTernaryResultToTextureMapper* texMapper)
{
    CVF_ASSERT(quadTextureCoords && quadMapper && resultAccessor && texMapper);

    size_t numVertices = quadMapper->quadCount()*4;
    quadTextureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = quadTextureCoords->ptr();

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(quadMapper->quadCount()); i++)
    {
        cvf::StructGridInterface::FaceType faceId = quadMapper->cellFace(i);
        size_t cellIdx = quadMapper->cellIndex(i);

        cvf::Vec2d resultValue = resultAccessor->cellFaceScalar(cellIdx, faceId);
        cvf::Vec2f texCoord = texMapper->getTexCoord(resultValue.x(), resultValue.y(), cellIdx);

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[i*4 + j] = texCoord;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryTextureCoordsCreator::createTextureCoords(cvf::Vec2fArray* textureCoords, const std::vector<size_t>& triangleToCellIdx, const RigTernaryResultAccessor* resultAccessor, const RivTernaryResultToTextureMapper* texMapper)
{
    CVF_ASSERT(textureCoords && resultAccessor && texMapper);

    size_t numVertices = triangleToCellIdx.size() * 3;
    textureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = textureCoords->ptr();

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(triangleToCellIdx.size()); i++)
    {
        size_t cellIdx = triangleToCellIdx[i];

        cvf::Vec2d resultValue = resultAccessor->cellScalarGlobIdx(cellIdx);
        cvf::Vec2f texCoord = texMapper->getTexCoord(resultValue.x(), resultValue.y(), cellIdx);

        size_t j;
        for (j = 0; j < 3; j++)
        {
            rawPtr[i * 3 + j] = texCoord;
        }
    }
}
