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

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigPipeInCellEvaluator.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInViewCollection.h"

#include "RivResultToTextureMapper.h"
#include "RivDefaultResultToTextureMapper.h"

#include "cvfStructGridGeometryGenerator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTextureCoordsCreator::RivTextureCoordsCreator(RimEclipseCellColors* cellResultColors, size_t timeStepIndex, size_t gridIndex, const cvf::StructGridQuadToCellFaceMapper* quadMapper)
{
    RigEclipseCaseData* eclipseCase = cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData();

    m_quadMapper = quadMapper;
    CVF_ASSERT(quadMapper && eclipseCase );

    m_resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase, gridIndex, timeStepIndex, cellResultColors);

    cvf::ref<RigPipeInCellEvaluator> pipeInCellEval = createPipeInCellEvaluator(cellResultColors, timeStepIndex, gridIndex);

    const cvf::ScalarMapper* mapper = cellResultColors->legendConfig()->scalarMapper();

    m_texMapper = new RivDefaultResultToTextureMapper(mapper, pipeInCellEval.p());
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
void RivTextureCoordsCreator::setResultToTextureMapper(RivResultToTextureMapper* textureMapper)
{
    m_texMapper = textureMapper;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigPipeInCellEvaluator * RivTextureCoordsCreator::createPipeInCellEvaluator(RimEclipseCellColors* cellColors, size_t timeStep, size_t gridIndex)
{
    return new RigPipeInCellEvaluator(cellColors->reservoirView()->wellCollection()->resultWellGeometryVisibilities(timeStep),
                                      cellColors->reservoirView()->eclipseCase()->eclipseCaseData()->gridCellToResultWellIndex(gridIndex));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTextureCoordsCreator::createTextureCoords(cvf::Vec2fArray* quadTextureCoords,
                                                  const cvf::StructGridQuadToCellFaceMapper* quadMapper,
                                                  const RigResultAccessor* resultAccessor,
                                                  const RivResultToTextureMapper* texMapper)
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

        double resultValue = resultAccessor->cellFaceScalar(cellIdx, faceId);
        cvf::Vec2f texCoord = texMapper->getTexCoord(resultValue, cellIdx);

        size_t j;
        for (j = 0; j < 4; j++)
        {   
            rawPtr[i*4 + j] = texCoord;
        }
    }
}

