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


#include "RivCellEdgeGeometryUtils.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigCellEdgeResultAccessor.h"
#include "RigGridBase.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RivTernaryTextureCoordsCreator.h"

#include "cvfDrawableGeo.h"
#include "cvfScalarMapper.h"
#include "cvfVertexAttribute.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCellEdgeGeometryUtils::addCellEdgeResultsToDrawableGeo(
    size_t timeStepIndex,
    RimEclipseCellColors* cellResultColors,
    RimCellEdgeColors* cellEdgeResultColors,
    const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
    cvf::DrawableGeo* geo,
    size_t gridIndex,
    bool useDefaultValueForHugeVals,
    float opacityLevel)
{
    RigEclipseCaseData* eclipseCase = cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData();
    CVF_ASSERT(eclipseCase != NULL);

    // Create result access objects

    cvf::ref<RigResultAccessor> cellCenterDataAccessObject = createCellCenterResultAccessor(cellResultColors, timeStepIndex, eclipseCase, eclipseCase->grid(gridIndex));
    cvf::ref<RigResultAccessor> cellEdgeResultAccessor = createCellEdgeResultAccessor(cellResultColors, cellEdgeResultColors, timeStepIndex, eclipseCase, eclipseCase->grid(gridIndex));

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

    cvf::ScalarMapper* cellResultScalarMapper = cellResultColors->legendConfig()->scalarMapper();
    cvf::ScalarMapper* edgeResultScalarMapper = cellEdgeResultColors->legendConfig()->scalarMapper();

    double ignoredScalarValue = cellEdgeResultColors->ignoredScalarValue();

    const std::vector<cvf::ubyte>* isWellPipeVisible = NULL;
    cvf::cref<cvf::UIntArray>      gridCellToWellindexMap;

    if (opacityLevel < 1.0f)
    {
        isWellPipeVisible = &(cellResultColors->reservoirView()->wellCollection()->resultWellGeometryVisibilities(timeStepIndex));
        gridCellToWellindexMap = eclipseCase->gridCellToResultWellIndex(gridIndex);
    }

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
        {
            cvf::StructGridInterface::FaceType cellFace = quadToCellFaceMapper->cellFace(quadIdx);
            double scalarValue = cellCenterDataAccessObject->cellFaceScalar(cellIndex, cellFace);

            {
                float cellColorTextureCoord = 0.5f; // If no results exists, the texture will have a special color
                if (useDefaultValueForHugeVals || scalarValue != HUGE_VAL)
                {
                    if (scalarValue != HUGE_VAL)
                    {
                        cellColorTextureCoord = cellResultScalarMapper->mapToTextureCoord(scalarValue)[0];
                    }

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


        for (size_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            float edgeColor = -1.0f; // Undefined texture coord. Shader handles this.

            double scalarValue = cellEdgeResultAccessor->cellFaceScalar(cellIndex, static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx));

            if (!hideScalarValue(scalarValue, ignoredScalarValue, 1e-2))
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

    cvf::ref<cvf::IntVertexAttributeDirect> faceIntAttribute = new cvf::IntVertexAttributeDirect("a_face", faceIndexArray.p());
    geo->setVertexAttribute(faceIntAttribute.p());

    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosI", cellEdgeColorTextureCoordsArrays.at(0)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegI", cellEdgeColorTextureCoordsArrays.at(1)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosJ", cellEdgeColorTextureCoordsArrays.at(2)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegJ", cellEdgeColorTextureCoordsArrays.at(3)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorPosK", cellEdgeColorTextureCoordsArrays.at(4)));
    geo->setVertexAttribute(new cvf::FloatVertexAttribute("a_colorNegK", cellEdgeColorTextureCoordsArrays.at(5)));
}

bool RivCellEdgeGeometryUtils::hideScalarValue(double scalarValue, double scalarValueToHide, double tolerance)
{
    return (scalarValue == HUGE_VAL || cvf::Math::abs(scalarValue - scalarValueToHide) <= scalarValueToHide*tolerance);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivCellEdgeGeometryUtils::addTernaryCellEdgeResultsToDrawableGeo(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, RimCellEdgeColors* cellEdgeResultColors,
    const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
    cvf::DrawableGeo* geo, size_t gridIndex, float opacityLevel)
{
    RigEclipseCaseData* eclipseCase = cellResultColors->reservoirView()->eclipseCase()->eclipseCaseData();
    CVF_ASSERT(eclipseCase != NULL);

    cvf::ref<RigResultAccessor> cellEdgeResultAccessor = createCellEdgeResultAccessor(cellResultColors, cellEdgeResultColors, timeStepIndex, eclipseCase, eclipseCase->grid(gridIndex));

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

    cvf::ScalarMapper* edgeResultScalarMapper = cellEdgeResultColors->legendConfig()->scalarMapper();

    double ignoredScalarValue = cellEdgeResultColors->ignoredScalarValue();

    RivTernaryTextureCoordsCreator texturer(cellResultColors, cellResultColors->ternaryLegendConfig(),
        timeStepIndex,
        gridIndex,
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

        for (size_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            float edgeColor = -1.0f; // Undefined texture coord. Shader handles this.

            double scalarValue = cellEdgeResultAccessor->cellFaceScalar(cellIndex, static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx));

            if (!hideScalarValue(scalarValue, ignoredScalarValue, 1e-2))
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
cvf::ref<RigResultAccessor> RivCellEdgeGeometryUtils::createCellEdgeResultAccessor(
    RimEclipseCellColors* cellResultColors,
    RimCellEdgeColors* cellEdgeResultColors,
    size_t timeStepIndex,
    RigEclipseCaseData* eclipseCase,
    const RigGridBase* grid)
{
    cvf::ref<RigCellEdgeResultAccessor> cellEdgeResultAccessor = new RigCellEdgeResultAccessor();
    
    if (cellEdgeResultColors->propertyType() == RimCellEdgeColors::ANY_SINGLE_PROPERTY)
    {
        cvf::ref<RigResultAccessor> daObj = RivCellEdgeGeometryUtils::createCellCenterResultAccessor(cellEdgeResultColors->singleVarEdgeResultColors(), timeStepIndex, eclipseCase, grid);

        for (size_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            cellEdgeResultAccessor->setDataAccessObjectForFace(static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx), daObj.p());
        }
    }
    else
    {
        size_t resultIndices[6];
        cellEdgeResultColors->gridScalarIndices(resultIndices);

        std::vector<RimCellEdgeMetaData> metaData;
        cellEdgeResultColors->cellEdgeMetaData(&metaData);

        size_t cubeFaceIdx;
        for (cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
        {
            size_t adjustedTimeStep = timeStepIndex;
            if (metaData[cubeFaceIdx].m_isStatic)
            {
                adjustedTimeStep = 0;
            }

            RiaDefines::PorosityModelType porosityModel = cellResultColors->porosityModel();
            cvf::ref<RigResultAccessor> daObj = RigResultAccessorFactory::createFromResultIdx(eclipseCase, grid->gridIndex(), porosityModel, adjustedTimeStep, resultIndices[cubeFaceIdx]);
            cellEdgeResultAccessor->setDataAccessObjectForFace(static_cast<cvf::StructGridInterface::FaceType>(cubeFaceIdx), daObj.p());
        }
    }

    return cellEdgeResultAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RivCellEdgeGeometryUtils::createCellCenterResultAccessor(RimEclipseCellColors* cellResultColors, size_t timeStepIndex, RigEclipseCaseData* eclipseCase, const RigGridBase* grid)
{
    cvf::ref<RigResultAccessor> resultAccessor = NULL;

    if (cellResultColors->hasResult())
    {
        resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCase, grid->gridIndex(), timeStepIndex, cellResultColors);
    }

    if (resultAccessor.isNull())
    {
        resultAccessor = new RigHugeValResultAccessor;
    }

    return resultAccessor;
}

