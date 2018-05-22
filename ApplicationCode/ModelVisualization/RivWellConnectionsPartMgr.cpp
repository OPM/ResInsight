/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RivWellConnectionsPartMgr.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include "RigSimWellData.h"
#include "RigFlowDiagResults.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigActiveCellInfo.h"

#include "cafEffectGenerator.h"
#include "cafDisplayCoordTransform.h"

#include "cvfDrawableGeo.h"
#include "cvfPart.h"
#include "cvfModelBasicList.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellConnectionsPartMgr::RivWellConnectionsPartMgr(RimEclipseView* reservoirView, RimSimWellInView* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell      = well;
    m_useCurvedArrows = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellConnectionsPartMgr::~RivWellConnectionsPartMgr()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellConnectionsPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (  m_rimReservoirView.isNull() ) return;
    if ( !m_rimReservoirView->eclipseCase() ) return;
    if ( !m_rimWell->showWell() ) return;
    if ( !m_rimWell->simWellData()->hasWellResult(frameIndex) ) return;
    if ( !m_rimWell->simWellData()->wellResultFrame(frameIndex).m_isOpen ) return;
    if (  m_rimWell->simWellData()->wellResultFrame(frameIndex).m_productionType == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE ) return;

    bool   isProducer =  (m_rimWell->simWellData()->wellResultFrame(frameIndex).m_productionType == RigWellResultFrame::PRODUCER);
    double pipeRadius = m_rimWell->pipeRadius();

    cvf::Vec3d                           wellHeadTop;
    cvf::Vec3d                           wellHeadBottom;
    double                               characteristicCellSize;
    double                               mainArrowZHeight;
    cvf::ref<caf::DisplayCoordTransform> displayCordXf;
    RigFlowDiagResults*                  flowResults;

    std::string                          injectorName;
    std::string                          producerName;
    std::string                          crossFlowInjectorName;
    std::string                          crossFlowProducerName;

    double                               fluxWidthScale = 0.0;

    {
        RimEclipseResultCase* eclResCase = dynamic_cast<RimEclipseResultCase*>(m_rimReservoirView->eclipseCase());
        if ( !eclResCase ) return;

        if ( !eclResCase->defaultFlowDiagSolution() ) return;

        flowResults                    = eclResCase->defaultFlowDiagSolution()->flowDiagResults();
        displayCordXf = m_rimReservoirView->displayCoordTransform();
        RigEclipseCaseData* rigReservoir                   = m_rimReservoirView->eclipseCase()->eclipseCaseData();

        characteristicCellSize                      = rigReservoir->mainGrid()->characteristicIJCellSize();

        m_rimWell->wellHeadTopBottomPosition(static_cast<int>(frameIndex), &wellHeadTop, &wellHeadBottom);
        wellHeadTop = displayCordXf->transformToDisplayCoord(wellHeadTop);
        wellHeadBottom = displayCordXf->transformToDisplayCoord(wellHeadBottom);
        wellHeadTop.z() += characteristicCellSize;

        cvf::Vec3d activeCellsBoundingBoxMax = displayCordXf->transformToDisplayCoord(m_rimReservoirView->currentActiveCellInfo()->geometryBoundingBox().max());
        mainArrowZHeight = activeCellsBoundingBoxMax.z() + 1.5*characteristicCellSize; // Above the bbox somewhat;

        if ( isProducer )
        { 
            producerName = m_rimWell->name().toStdString();
            crossFlowInjectorName = RimFlowDiagSolution::addCrossFlowEnding(m_rimWell->name()).toStdString();
        }
        else
        {
            injectorName = m_rimWell->name().toStdString();
            crossFlowProducerName = RimFlowDiagSolution::addCrossFlowEnding(m_rimWell->name()).toStdString();
        }

        double maxAbsFlux = flowResults->maxAbsPairFlux(static_cast<int>(frameIndex));
        if (maxAbsFlux != 0.0) fluxWidthScale = characteristicCellSize / maxAbsFlux;

    }

    bool enableLighting                = !m_rimReservoirView->isLightingDisabled();
    RimSimWellInViewCollection* wellColl = m_rimReservoirView->wellCollection();

    // Create potentially two the arrows to/from m_rimWell for each of the other wells in the model.
    // One arrow for the "official" state of the well, and one to account for cross flow contributions 

    for ( RimSimWellInView * otherWell: wellColl->wells )
    {
        if (  otherWell == m_rimWell ) continue;
        if ( !otherWell->simWellData()->hasWellResult(frameIndex) ) continue;
        if ( !otherWell->simWellData()->wellResultFrame(frameIndex).m_isOpen ) continue;
        if (  otherWell->simWellData()->wellResultFrame(frameIndex).m_productionType == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE ) continue;

        bool isOtherProducer = (otherWell->simWellData()->wellResultFrame(frameIndex).m_productionType == RigWellResultFrame::PRODUCER);

        {
            std::string otherWellName   =  otherWell->name().toStdString();
            std::string otherWellXfName = RimFlowDiagSolution::addCrossFlowEnding(otherWell->name()).toStdString();

            if ( isProducer != isOtherProducer )
            {
                if ( isOtherProducer )
                {
                    producerName = otherWellName;
                    crossFlowInjectorName = otherWellXfName;
                }
                else
                {
                    injectorName = otherWellName;
                    crossFlowProducerName = otherWellXfName;
                }
            }
            else
            {
                if ( isProducer )
                {
                    injectorName = otherWellXfName;
                    crossFlowProducerName = otherWellName;
                }
                else
                {
                    producerName = otherWellXfName;
                    crossFlowInjectorName = otherWellName;
                }
            }
        }

        std::pair<double, double> injProdFluxPair   = flowResults->injectorProducerPairFluxes(injectorName, producerName, static_cast<int>(frameIndex));
        std::pair<double, double> injProdFluxPairXF = flowResults->injectorProducerPairFluxes(crossFlowInjectorName, crossFlowProducerName, static_cast<int>(frameIndex));

        const double fluxThreshold = 0.0; // Todo : Needs threshold in Gui

        if (   fabs(injProdFluxPair.first)   <= fluxThreshold 
            && fabs(injProdFluxPair.second)  <= fluxThreshold 
            && fabs(injProdFluxPairXF.first) <= fluxThreshold 
            && fabs(injProdFluxPairXF.second)<= fluxThreshold ) continue; 

        float width   = fluxWidthScale * ( isProducer ? injProdFluxPair.second  :  injProdFluxPair.first);
        float widthXf = fluxWidthScale * (!isProducer ? injProdFluxPairXF.second:  injProdFluxPairXF.first);

        cvf::Vec3d otherWellHeadTop;
        cvf::Vec3d otherWellHeadBottom;
        {
            otherWell->wellHeadTopBottomPosition(static_cast<int>(frameIndex), &otherWellHeadTop, &otherWellHeadBottom);
            otherWellHeadTop = displayCordXf->transformToDisplayCoord(otherWellHeadTop);
            otherWellHeadBottom = displayCordXf->transformToDisplayCoord(otherWellHeadBottom);
            otherWellHeadTop.z() += characteristicCellSize;
        }

        {
            cvf::Vec3f startPoint = cvf::Vec3f(0.5*(wellHeadTop + otherWellHeadTop));
            if (m_useCurvedArrows) startPoint.z() = mainArrowZHeight;
            cvf::Vec3f endPoint = cvf::Vec3f(wellHeadTop + (3* pipeRadius * (otherWellHeadTop - wellHeadTop).getNormalized()));
            cvf::Color4f arrowColor(otherWell->wellPipeColor());

            if (   fabs(injProdFluxPair.first)  > fluxThreshold
                && fabs(injProdFluxPair.second) > fluxThreshold )
            {
                if ( isProducer == isOtherProducer )
                {
                    startPoint.z() -=  0.5*characteristicCellSize;
                    endPoint.z()   -= 0.5*characteristicCellSize;
                }
                cvf::ref<cvf::Part> arrowPart = createArrowPart(startPoint, endPoint, width, isProducer, arrowColor, enableLighting);
                model->addPart(arrowPart.p());
            }

            if (   fabs(injProdFluxPairXF.first)   > fluxThreshold
                && fabs(injProdFluxPairXF.second)  > fluxThreshold )
            {
                startPoint.z() -= 0.5*characteristicCellSize;
                endPoint.z()   -= 0.5*characteristicCellSize;
                cvf::ref<cvf::Part> arrowPart = createArrowPart(startPoint, endPoint, widthXf, !isProducer, arrowColor, enableLighting);
                model->addPart(arrowPart.p());
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>  RivWellConnectionsPartMgr::createArrowPart(const cvf::Vec3f& startPoint, 
                                                                const cvf::Vec3f& endPoint, 
                                                                float width, 
                                                                bool isProducer, 
                                                                const cvf::Color4f& arrowColor, 
                                                                bool enableLighting)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    cvf::ref<cvf::DrawableGeo> geo = createArrowGeometry(startPoint, endPoint, width, isProducer);

    part->setDrawable(geo.p());
    caf::SurfaceEffectGenerator surfaceGen(arrowColor, caf::PO_1);
    surfaceGen.enableLighting(enableLighting);

    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

    part->setEffect(eff.p());

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref< cvf::DrawableGeo> RivWellConnectionsPartMgr::createArrowGeometry(const cvf::Vec3f& startPoint,
                                                                   const cvf::Vec3f& endPoint, 
                                                                   double width, 
                                                                   bool useArrowEnd)
{
    // Vertex layout
    //              _ -  _ 
    // __________ -         - _         
    //                            -        Producer end:   Injector end
    // 0        2 4 6 8 10 20 12 14        16                 16
    // :  flat  : : : :  :  :  :  : end        18         18 
    // 1        3 5 7 9 11 19 13 15        17                 17

    static const cvf::uint producerArrowFaceList[8 * 5 + 4] ={ 4,   0, 1, 3, 2,
        4,   2, 3, 5, 4,
        4,   4, 5, 7, 6,
        4,   6, 7, 9, 8,
        4,   8, 9, 11, 10,
        4,   10, 11, 20, 19,
        4,   19, 20, 13, 12,
        4,   12, 13, 15, 14,
        3,   16, 17, 18 };

    static const cvf::uint injectorArrowFaceList[8 * 5 + 8] ={ 4,   0, 1, 3, 2,
        4,   2, 3, 5, 4,
        4,   4, 5, 7, 6,
        4,   6, 7, 9, 8,
        4,   8, 9, 11, 10,
        4,   10, 11, 20, 19,
        4,   19, 20, 13, 12,
        4,   12, 13, 15, 14,
        3,   14, 18, 16,
        3,   18, 15, 17 };


    cvf::Vec3f endPointInTopPlane = endPoint;
    if (m_useCurvedArrows) endPointInTopPlane.z() = startPoint.z();

    cvf::Vec3f heightDiff = cvf::Vec3f::ZERO;
    if (m_useCurvedArrows)  heightDiff.z() = (startPoint.z() - endPoint.z());

    cvf::Vec3f fromTo = endPointInTopPlane - startPoint;
    float length = fromTo.length();

    float halfWidth = width * 0.5;
    cvf::Vec3f widthVector = halfWidth *(fromTo.getNormalized() ^ -cvf::Vec3f::Z_AXIS);
    
    float heightScale = 0.3*length * 0.15;
    cvf::Vec3f heightScaleVec =  cvf::Vec3f::ZERO;

    if (m_useCurvedArrows) heightScaleVec.z() = heightScale;

    float endStart = 0.4f;
    float endStep  = (1.0f - endStart) / 7.5f;

    cvf::ref< cvf::Vec3fArray> arrowVertexArray = new cvf::Vec3fArray;
    arrowVertexArray->resize(18+3);

    (*arrowVertexArray)[0]  = 0.0f             * fromTo + startPoint + widthVector;
    (*arrowVertexArray)[1]  = 0.0f             * fromTo + startPoint - widthVector;
    (*arrowVertexArray)[2]  = endStart         * fromTo + startPoint + widthVector;
    (*arrowVertexArray)[3]  = endStart         * fromTo + startPoint - widthVector;

    (*arrowVertexArray)[4]  = (1*endStep + endStart) * fromTo + startPoint + widthVector + 0.250f * heightScaleVec;//0.0250f * heightDiff;
    (*arrowVertexArray)[5]  = (1*endStep + endStart) * fromTo + startPoint - widthVector + 0.250f * heightScaleVec;//0.0250f * heightDiff;
    (*arrowVertexArray)[6]  = (2*endStep + endStart) * fromTo + startPoint + widthVector + 0.750f * heightScaleVec;//0.0750f * heightDiff;
    (*arrowVertexArray)[7]  = (2*endStep + endStart) * fromTo + startPoint - widthVector + 0.750f * heightScaleVec;//0.0750f * heightDiff;
    (*arrowVertexArray)[8]  = (3*endStep + endStart) * fromTo + startPoint + widthVector + 1.000f * heightScaleVec;//0.1000f * heightDiff;
    (*arrowVertexArray)[9]  = (3*endStep + endStart) * fromTo + startPoint - widthVector + 1.000f * heightScaleVec;//0.1000f * heightDiff;
    (*arrowVertexArray)[10] = (4*endStep + endStart) * fromTo + startPoint + widthVector + 0.875f * heightScaleVec;//0.0875f * heightDiff;
    (*arrowVertexArray)[11] = (4*endStep + endStart) * fromTo + startPoint - widthVector + 0.875f * heightScaleVec;//0.0875f * heightDiff;
    (*arrowVertexArray)[19] = (4.7f*endStep + endStart) * fromTo + startPoint + widthVector + 0.400f * heightScaleVec;//0.0875f * heightDiff;
    (*arrowVertexArray)[20] = (4.7f*endStep + endStart) * fromTo + startPoint - widthVector + 0.400f * heightScaleVec;//0.0875f * heightDiff;

    (*arrowVertexArray)[12] = (5*endStep + endStart) * fromTo + startPoint + widthVector;
    (*arrowVertexArray)[13] = (5*endStep + endStart) * fromTo + startPoint - widthVector;

    (*arrowVertexArray)[14] = (6*endStep + endStart) * fromTo + startPoint + widthVector - 0.5f * heightDiff;
    (*arrowVertexArray)[15] = (6*endStep + endStart) * fromTo + startPoint - widthVector - 0.5f * heightDiff;

    if ( useArrowEnd )
    {
        (*arrowVertexArray)[16] = (6*endStep + endStart) * fromTo + startPoint + 1.6f * widthVector - 0.5f * heightDiff;
        (*arrowVertexArray)[17] = (6*endStep + endStart) * fromTo + startPoint - 1.6f * widthVector - 0.5f * heightDiff;
        (*arrowVertexArray)[18] = 1.0f                   * fromTo + startPoint                      - 1.0f * heightDiff;
    }
    else
    {
        (*arrowVertexArray)[16] = 1.0f                   * fromTo + startPoint + 0.5f * widthVector - 1.0f * heightDiff;
        (*arrowVertexArray)[17] = 1.0f                   * fromTo + startPoint - 0.5f * widthVector - 1.0f * heightDiff;
        (*arrowVertexArray)[18] = (6*endStep + endStart) * fromTo + startPoint                      - 0.5f * heightDiff;
    }

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(arrowVertexArray.p());

    if ( useArrowEnd )
        geo->setFromFaceList(cvf::UIntArray(producerArrowFaceList, 8 * 5 + 4));
    else
        geo->setFromFaceList(cvf::UIntArray(injectorArrowFaceList, 8 * 5 + 8));  

    geo->computeNormals();

    return geo;
}
