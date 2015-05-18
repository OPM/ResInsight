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

#include "RivTextureCoordsCreator.h"

#include "RimResultSlot.h"
#include "RigCaseData.h"
#include "RimReservoirView.h"
#include "RimEclipseCase.h"
#include "RigCaseCellResultsData.h"
#include "RigResultAccessorFactory.h"
#include "RigPipeInCellEvaluator.h"
#include "RivResultToTextureMapper.h"
#include "RimWellCollection.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTextureCoordsCreator::RivTextureCoordsCreator(RimResultSlot* cellResultSlot, size_t timeStepIndex, size_t gridIndex, const cvf::StructGridQuadToCellFaceMapper* quadMapper)
{
    RigCaseData* eclipseCase = cellResultSlot->reservoirView()->eclipseCase()->reservoirData();

    m_quadMapper = quadMapper;
    CVF_ASSERT(quadMapper && eclipseCase );

    size_t resTimeStepIdx = timeStepIndex;

    if (cellResultSlot->hasStaticResult()) resTimeStepIdx = 0;

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());

    m_resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, resTimeStepIdx, cellResultSlot->resultVariable());

    cvf::ref<RigPipeInCellEvaluator> pipeInCellEval = new RigPipeInCellEvaluator(cellResultSlot->reservoirView()->wellCollection()->isWellPipesVisible(timeStepIndex),
        eclipseCase->gridCellToWellIndex(gridIndex));

    const cvf::ScalarMapper* mapper = cellResultSlot->legendConfig()->scalarMapper();

    m_texMapper = new RivResultToTextureMapper(mapper, pipeInCellEval.p());
    CVF_ASSERT(m_texMapper.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivTextureCoordsCreator::isValid()
{
	if (m_quadMapper.isNull() || m_resultAccessor.isNull() || m_texMapper.isNull())
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextureCoordsCreator::createTextureCoords(cvf::Vec2fArray* quadTextureCoords)
{
	createTextureCoords(quadTextureCoords, m_quadMapper.p(), m_resultAccessor.p(), m_texMapper.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextureCoordsCreator::createTextureCoords(
	cvf::Vec2fArray* quadTextureCoords,
	const cvf::StructGridQuadToCellFaceMapper* quadMapper,
	const RigResultAccessor* resultAccessor,
	const RivResultToTextureMapper* texMapper)
{
    CVF_ASSERT(quadTextureCoords && quadMapper && resultAccessor && texMapper);

    size_t numVertices = quadMapper->quadCount()*4;
    quadTextureCoords->resize(numVertices);
    cvf::Vec2f* rawPtr = quadTextureCoords->ptr();

    double resultValue;
    cvf::Vec2f texCoord;

#pragma omp parallel for private(texCoord, resultValue)
    for (int i = 0; i < static_cast<int>(quadMapper->quadCount()); i++)
    {
        cvf::StructGridInterface::FaceType faceId = quadMapper->cellFace(i);
        size_t cellIdx = quadMapper->cellIndex(i);

        resultValue = resultAccessor->cellFaceScalar(cellIdx, faceId);
        texCoord = texMapper->getTexCoord(resultValue, cellIdx);

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[i*4 + j] = texCoord;
        }
    }
}

