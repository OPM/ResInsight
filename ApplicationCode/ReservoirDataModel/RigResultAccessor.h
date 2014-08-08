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

#pragma once

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"


//==================================================================================================
/// 
//==================================================================================================
class RigResultAccessor : public cvf::Object
{
public:
    virtual double cellScalar(size_t gridLocalCellIndex) const = 0;
    virtual double cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const = 0;
};

#if 0

//==================================================================================================
/// 
//==================================================================================================
class RigResultAccessor2d : public cvf::Object
{
public:
    virtual cvf::Vec2d cellScalar(size_t gridLocalCellIndex) const = 0;
    virtual cvf::Vec2d cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const = 0;

    virtual QString resultName() const = 0;
    
};

//==================================================================================================
/// 
//==================================================================================================
class RigTernaryResultAccessor : public Rig2DResultAccessor
{
public:
    /// Requires two of the arguments to be present
    void setTernaryResultAccessors(RigResultAccessObject* soil, RigResultAccessObject* sgas, RigResultAccessObject* swat);

    /// Returns [SOil, SWat] regardless of which one of the three is missing. if Soil or SWat is missing, it is calculated 
    /// based on the two others
    virtual cvf::Vec2d cellScalar(size_t gridLocalCellIndex) { };
    virtual cvf::Vec2d cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) { return cellScalar(size_t gridLocalCellIndex); };

    virtual QString resultName() const = 0;

};

class RivTernaryScalarMapper : public cvf::Object
{
public:
    RivTernaryScalarMapper(const cvf::Color3f& undefScalarColor, float opacityLevel) : m_undefScalarColor(undefScalarColor), m_opacityLevel(opacityLevel)
    {

    }

    /// Calculate texture coords into an image produced by updateTexture, from the scalarValue
     Vec2f               mapToTextureCoord(double soil, double swat, bool isTransparent) {}

    /// Update the supplied TextureImage to be addressable by the texture coords delivered by mapToTextureCoord
     bool                updateTexture(TextureImage* image){}

private:
    cvf::Color3f m_undefScalarColor; 
    float m_opacityLevel;
};

class RigPipeInCellEvaluator: public cvf::Object
{
public: 
    RigPipeInCellEvaluator(const std::vector<cvf::ubyte>& isWellPipeVisibleForWellIndex, 
                           const cvf::UIntArray* gridCellToWellIndexMap) 
                           : m_isWellPipeVisibleForWellIndex(isWellPipeVisibleForWellIndex), 
                             m_gridCellToWellIndexMap(gridCellToWellIndexMap)
    {
    }

    bool isWellPipeInCell( size_t cellIndex)
    {
        cvf::uint wellIndex = m_gridCellToWellIndexMap->get(cellIndex);

        if (wellIndex == cvf::UNDEFINED_UINT)
        {  
            return false;
        }

        return m_isWellPipeVisibleForWellIndex[wellIndex];
    }

private:

    const std::vector<cvf::ubyte>& m_isWellPipeVisibleForWellIndex;
    const cvf::UIntArray* m_gridCellToWellIndexMap;
};

class RivResultToTextureMapper : public cvf::Object
{

    RivResultToTextureMapper(const cvf::ScalarMapper* scalarMapper, 
        const RigPipeInCellEvaluator* pipeInCellEvaluator) 
        : m_scalarMapper(scalarMapper), m_pipeInCellEvaluator(pipeInCellEvaluator)
    {}
    
    Vec2f getTexCoord(double resultValue, size_t cellIndex)
    {
        Vec2f texCoord(0,0);

       if (resultValue == HUGE_VAL || resultValue != resultValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
            return texCoord;
        }

        texCoord = m_scalarMapper->mapToTexCoord(resultValue);
  
        if (!m_pipeInCellEvaluator->isWellPipeInCell(cellIndex))
        {
            texCoord[1] = 0; // Set the Y texture coordinate to the opaque line in the texture
        }

        return texCoord;
    }
  
private:
    cvf::cref<cvf::ScalarMapper> m_scalarMapper;
    cvf::cref<RigPipeInCellEvaluator> m_pipeInCellEvaluator;
};




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
        else if (cellResultSlot->resultVariable().compare(RimDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
        {
            cvf::Vec2fArray* textureCoords = m_surfaceFacesTextureCoords.p();
            RivTransmissibilityColorMapper::updateCombinedTransmissibilityTextureCoordinates(cellResultSlot, m_grid.p(), textureCoords, m_surfaceGenerator.quadToCellFaceMapper());
        }
        else
        {
            // If the result is static, only read that.
            size_t resTimeStepIdx = timeStepIndex;
            if (cellResultSlot->hasStaticResult()) resTimeStepIdx = 0;

            RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(cellResultSlot->porosityModel());
            cvf::ref<RigResultAccessObject> resultAccessor = RigResultAccessObjectFactory::createResultAccessObject(eclipseCase, m_grid->gridIdx(), porosityModel, resTimeStepIdx, cellResultSlot->resultVariable());

            if (resultAccessor.isNull()) return;

            RivResultToTextureMapper texMapper(mapper, 
                RigPipeInCellEvaluator(cellResultSlot->reservoirView()->wellCollection()->isWellPipesVisible(timeStepIndex),
                eclipseCase->gridCellToWellIndex(m_grid->gridIndex())));

            const StructGridQuadToCellFaceMapper* quadMapper = m_surfaceGenerator.quadToCellFaceMapper();

            size_t numVertices = quadMapper->quadCount()*4;
            m_surfaceFacesTextureCoords->resize(numVertices);
            cvf::Vec2f* rawPtr = m_surfaceFacesTextureCoords->ptr();

            double cellScalarValue;
            cvf::Vec2f texCoord;

#pragma omp parallel for private(texCoord, cellScalarValue)
            for (int i = 0; i < static_cast<int>(m_quadMapper->quadCount()); i++)
            {
                StructGridInterface::FaceType faceId = m_quadMapper->cellFace(i);

                cellScalarValue = resultAccessor->cellFaceScalar(m_quadMapper->cellIndex(i), faceId);

                texCoord = texMapper->mapToTextureCoord(cellScalarValue);

                size_t j;
                for (j = 0; j < 4; j++)
                {   
                    rawPtr[i*4 + j] = texCoord;
                }
            }
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

#endif
