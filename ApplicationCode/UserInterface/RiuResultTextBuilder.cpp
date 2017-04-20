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

#include "RiuResultTextBuilder.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSingleWellResultsData.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimFormationNames.h"
#include "RimLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafDisplayCoordTransform.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultTextBuilder::RiuResultTextBuilder(RimEclipseView* reservoirView, size_t gridIndex, size_t cellIndex, size_t timeStepIndex)
{
    CVF_ASSERT(reservoirView);
    
    m_reservoirView = reservoirView;
    m_gridIndex = gridIndex;
    m_cellIndex = cellIndex;
    m_timeStepIndex = timeStepIndex;

    m_nncIndex = cvf::UNDEFINED_SIZE_T;
    m_intersectionPoint = cvf::Vec3d::UNDEFINED;
    m_face = cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setNncIndex(size_t nncIndex)
{
    m_nncIndex = nncIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setIntersectionPoint(cvf::Vec3d intersectionPoint)
{
    m_intersectionPoint = intersectionPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setFace(cvf::StructGridInterface::FaceType face)
{
    m_face = face;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::mainResultText()
{
    QString text;

    // Produce result text for all variants
    // Priority defined as follows :  NNC, Fault, Grid
    {
        QString nncText = nncResultText();
        if (!nncText.isEmpty())
        {
            text = "NNC : " + nncText;
        }
        else if (m_cellIndex != cvf::UNDEFINED_SIZE_T)
        {
            QString faultText = faultResultText();

            if (!faultResultText().isEmpty())
            {
                text = "Fault : " + faultText;
            }
            else
            {
                text = "Grid cell : " + gridResultText();
            }
        }
        
        text += "\n";
    }

    QString topoText = this->geometrySelectionText("\n");
    text += topoText;
    appendDetails(text, formationDetails());
    text += "\n";

    appendDetails(text, nncDetails());
    
    if (m_cellIndex != cvf::UNDEFINED_SIZE_T)
    {
        appendDetails(text, faultResultDetails());
        appendDetails(text, cellEdgeResultDetails());
        appendDetails(text, gridResultDetails());
        appendDetails(text, wellResultText());
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::geometrySelectionText(QString itemSeparator)
{
    QString text;

    if (m_reservoirView && m_reservoirView->eclipseCase())
    {
        const RigEclipseCaseData* eclipseCase = m_reservoirView->eclipseCase()->eclipseCaseData();
        if (eclipseCase)
        {
            if (m_cellIndex != cvf::UNDEFINED_SIZE_T)
            {
                size_t i = 0;
                size_t j = 0;
                size_t k = 0;
                if (eclipseCase->grid(m_gridIndex)->ijkFromCellIndex(m_cellIndex, &i, &j, &k))
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    cvf::StructGridInterface::FaceEnum faceEnum(m_face);

                    QString faceText = faceEnum.text();

                    text += QString("Face : %1").arg(faceText) + itemSeparator;
                    text += QString("Hit grid %1").arg(m_gridIndex) + itemSeparator;
                    text += QString("Cell : [%1, %2, %3]").arg(i).arg(j).arg(k) + itemSeparator;
                }
            }

            
            cvf::ref<caf::DisplayCoordTransform> transForm = m_reservoirView->displayCoordTransform();
            cvf::Vec3d domainCoord = transForm->translateToDomainCoord(m_intersectionPoint);

            QString formattedText;
            formattedText.sprintf("Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]", domainCoord.x(), domainCoord.y(), -domainCoord.z());

            text += formattedText;
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::gridResultDetails()
{
    QString text;

    if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->eclipseCaseData())
    {
        RigEclipseCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->eclipseCaseData();
        RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);

        this->appendTextFromResultColors(eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_reservoirView->cellResult(), &text);

        if (!text.isEmpty())
        {
            text.prepend("-- Grid cell result details --\n");
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::faultResultDetails()
{
    QString text;

    if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->eclipseCaseData())
    {
        RigEclipseCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->eclipseCaseData();
        RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);
        RigMainGrid* mainGrid = grid->mainGrid();

        const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(m_cellIndex, m_face);
        if (fault)
        {
            text += "-- Fault result details --\n";
            
            text += QString("Fault Name: %1\n").arg(fault->name());

            cvf::StructGridInterface::FaceEnum faceHelper(m_face);
            text += "Fault Face : " + faceHelper.text() + "\n";

            if (m_reservoirView->faultResultSettings()->hasValidCustomResult())
            {
                text += "Fault result data:\n";
                this->appendTextFromResultColors(eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_reservoirView->currentFaultResultColors(), &text);
            }
        }
    }

    return text;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::formationDetails()
{
    QString text;
    RimCase* rimCase = m_reservoirView->eclipseCase();
    if(rimCase)
    {
        if(rimCase->activeFormationNames() && rimCase->activeFormationNames()->formationNamesData())
        {
            RigFormationNames* formNames = rimCase->activeFormationNames()->formationNamesData();

            size_t k =  cvf::UNDEFINED_SIZE_T;
            {
                const RigEclipseCaseData* eclipseData = m_reservoirView->eclipseCase()->eclipseCaseData();
                if(eclipseData)
                {
                    if(m_cellIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        size_t i = cvf::UNDEFINED_SIZE_T;
                        size_t j = cvf::UNDEFINED_SIZE_T;
     
                        eclipseData->grid(m_gridIndex)->ijkFromCellIndex(m_cellIndex, &i, &j, &k);
                    }
                }
            }

            if (k != cvf::UNDEFINED_SIZE_T)
            {
                QString formName = formNames->formationNameFromKLayerIdx(k);
                if(!formName.isEmpty())
                {
                    //text += "-- Formation details --\n";

                    text += QString("Formation Name: %1\n").arg(formName);
                }
            }
        }
    }

    return text;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::gridResultText()
{
    QString text = cellResultText(m_reservoirView->cellResult());

    return text;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::faultResultText()
{
    QString text;

    if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->eclipseCaseData())
    {
        RigEclipseCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->eclipseCaseData();
        RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);
        RigMainGrid* mainGrid = grid->mainGrid();

        const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(m_cellIndex, m_face);
        if (fault)
        {
            cvf::StructGridInterface::FaceEnum faceHelper(m_face);
            if (m_reservoirView->faultResultSettings()->hasValidCustomResult())
            {
                text = cellResultText(m_reservoirView->currentFaultResultColors());
            }
        }
    }

    return text;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::nncResultText()
{
    QString text;

    if (m_nncIndex != cvf::UNDEFINED_SIZE_T)
    {
        if (m_reservoirView.notNull() && m_reservoirView->eclipseCase())
        {
            RigEclipseCaseData* eclipseCase = m_reservoirView->eclipseCase()->eclipseCaseData();

            RigMainGrid* grid = eclipseCase->mainGrid();
            CVF_ASSERT(grid);

            RigNNCData* nncData = grid->nncData();
            CVF_ASSERT(nncData);

            if (nncData)
            {
                const RigConnection& conn = nncData->connections()[m_nncIndex];
                cvf::StructGridInterface::FaceEnum face(conn.m_c1Face);

                if (m_reservoirView->currentFaultResultColors())
                {
                    size_t scalarResultIdx = m_reservoirView->currentFaultResultColors()->scalarResultIndex();
                    const std::vector<double>* nncValues = nncData->connectionScalarResult(scalarResultIdx);
                    if (nncValues)
                    {
                        QString resultVar = m_reservoirView->currentFaultResultColors()->resultVariableUiName();
                        double scalarValue = (*nncValues)[m_nncIndex];

                        text = QString("%1 : %2").arg(resultVar).arg(scalarValue);
                    }
                }
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::appendTextFromResultColors(RigEclipseCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimEclipseCellColors* resultColors, QString* resultInfoText)
{
    if (!resultColors)
    {
        return;
    }

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultColors->porosityModel());
    if (resultColors->isTernarySaturationSelected())
    {
        RimReservoirCellResultsStorage* gridCellResults = resultColors->currentGridCellResults();
        if (gridCellResults)
        {
            size_t soilScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
            size_t sgasScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
            size_t swatScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");

            cvf::ref<RigResultAccessor> dataAccessObjectX = RigResultAccessorFactory::createFromResultIdx(eclipseCase, gridIndex, porosityModel, timeStepIndex, soilScalarSetIndex);
            cvf::ref<RigResultAccessor> dataAccessObjectY = RigResultAccessorFactory::createFromResultIdx(eclipseCase, gridIndex, porosityModel, timeStepIndex, sgasScalarSetIndex);
            cvf::ref<RigResultAccessor> dataAccessObjectZ = RigResultAccessorFactory::createFromResultIdx(eclipseCase, gridIndex, porosityModel, timeStepIndex, swatScalarSetIndex);

            double scalarValue = 0.0;

            if (dataAccessObjectX.notNull()) scalarValue = dataAccessObjectX->cellScalar(cellIndex);
            else scalarValue = 0.0;
            resultInfoText->append(QString("SOIL : %1\n").arg(scalarValue));

            if (dataAccessObjectY.notNull()) scalarValue = dataAccessObjectY->cellScalar(cellIndex);
            else scalarValue = 0.0;
            resultInfoText->append(QString("SGAS : %1\n").arg(scalarValue));

            if (dataAccessObjectZ.notNull()) scalarValue = dataAccessObjectZ->cellScalar(cellIndex);
            else scalarValue = 0.0;
            resultInfoText->append(QString("SWAT : %1\n").arg(scalarValue));
        }

        return;
    }
    else if (resultColors->hasResult())
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultColors->porosityModel());

        if (resultColors->hasStaticResult())
        {
            if (resultColors->resultVariable().compare(RimDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
            {
                cvf::ref<RigResultAccessor> transResultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedTransmissibilityResultName());
                {
                    double scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
                    resultInfoText->append(QString("Tran X : %1\n").arg(scalarValue));

                    scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
                    resultInfoText->append(QString("Tran Y : %1\n").arg(scalarValue));

                    scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
                    resultInfoText->append(QString("Tran Z : %1\n").arg(scalarValue));
                }

                return;
            }
            else if (resultColors->resultVariable().compare(RimDefines::combinedMultResultName(), Qt::CaseInsensitive) == 0)
            {
                cvf::ref<RigResultAccessor> multResultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedMultResultName());
                {
                    double scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
                    resultInfoText->append(QString("MULTX : %1\n").arg(scalarValue));
                    scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::NEG_I);
                    resultInfoText->append(QString("MULTX- : %1\n").arg(scalarValue));

                    scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
                    resultInfoText->append(QString("MULTY : %1\n").arg(scalarValue));
                    scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::NEG_J);
                    resultInfoText->append(QString("MULTY- : %1\n").arg(scalarValue));

                    scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
                    resultInfoText->append(QString("MULTZ : %1\n").arg(scalarValue));
                    scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::NEG_K);
                    resultInfoText->append(QString("MULTZ- : %1\n").arg(scalarValue));
                }

                return;
            }
            else if (resultColors->resultVariable().compare(RimDefines::combinedRiTranResultName(), Qt::CaseInsensitive) == 0)
            {
                cvf::ref<RigResultAccessor> transResultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiTranResultName());
                {
                    double scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
                    resultInfoText->append(QString("riTran X : %1\n").arg(scalarValue));

                    scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
                    resultInfoText->append(QString("riTran Y : %1\n").arg(scalarValue));

                    scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
                    resultInfoText->append(QString("riTran Z : %1\n").arg(scalarValue));
                }
            
                return;
            }
            else if (resultColors->resultVariable().compare(RimDefines::combinedRiMultResultName(), Qt::CaseInsensitive) == 0)
            {
                cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiMultResultName());
                {
                    double scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
                    resultInfoText->append(QString("riMult X : %1\n").arg(scalarValue));

                    scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
                    resultInfoText->append(QString("riMult Y : %1\n").arg(scalarValue));

                    scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
                    resultInfoText->append(QString("riMult Z : %1\n").arg(scalarValue));
                }

                return;
            }
            else if (resultColors->resultVariable().compare(RimDefines::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive) == 0)
            {
                cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromUiResultName(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiAreaNormTranResultName());
                {
                    double scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
                    resultInfoText->append(QString("riTransByArea X : %1\n").arg(scalarValue));

                    scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
                    resultInfoText->append(QString("riTransByArea Y : %1\n").arg(scalarValue));

                    scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
                    resultInfoText->append(QString("riTransByArea Z : %1\n").arg(scalarValue));
                }

                return;
            }
        }
    }

    resultInfoText->append(cellResultText(resultColors));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::cellEdgeResultDetails()
{
    QString text;

    if (m_reservoirView->cellEdgeResult()->hasResult())
    {
        text += "-- Cell edge result data --\n";

        if (m_reservoirView->cellEdgeResult()->isUsingSingleVariable())
        {
            text += cellResultText(m_reservoirView->cellEdgeResult()->singleVarEdgeResultColors());
            text +=  "\n";
        }
        else
        {
            std::vector<RimCellEdgeMetaData> metaData;
            m_reservoirView->cellEdgeResult()->cellEdgeMetaData(&metaData);

            std::set<size_t> uniqueResultIndices;

            for (int idx = 0; idx < 6; idx++)
            {
                size_t resultIndex = metaData[idx].m_resultIndex;
                if (resultIndex == cvf::UNDEFINED_SIZE_T) continue;
            
                if (uniqueResultIndices.find(resultIndex) != uniqueResultIndices.end()) continue;

                size_t adjustedTimeStep = m_timeStepIndex;
                if (metaData[idx].m_isStatic)
                {
                    adjustedTimeStep = 0;
                }

                RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_reservoirView->cellResult()->porosityModel());
                cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromResultIdx(m_reservoirView->eclipseCase()->eclipseCaseData(), m_gridIndex, porosityModel, adjustedTimeStep, resultIndex);
                if (resultAccessor.notNull())
                {
                    double scalarValue = resultAccessor->cellScalar(m_cellIndex);
                    text.append(QString("%1 : %2\n").arg(metaData[idx].m_resultVariable).arg(scalarValue));

                    uniqueResultIndices.insert(resultIndex);
                }
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::nncDetails()
{
    QString text;

    if (m_nncIndex != cvf::UNDEFINED_SIZE_T)
    {
        if (m_reservoirView.notNull() && m_reservoirView->eclipseCase())
        {
            RigEclipseCaseData* eclipseCase = m_reservoirView->eclipseCase()->eclipseCaseData();

            RigMainGrid* grid = eclipseCase->mainGrid();
            CVF_ASSERT(grid);

            RigNNCData* nncData = grid->nncData();
            CVF_ASSERT(nncData);

            if (nncData)
            {
                text += "-- NNC details --\n";
                {
                    const RigConnection& conn = nncData->connections()[m_nncIndex];
                    cvf::StructGridInterface::FaceEnum face(conn.m_c1Face);

                    // First cell of NNC
                    {
                        CVF_ASSERT(conn.m_c1GlobIdx < grid->globalCellArray().size());
                        const RigCell& cell = grid->globalCellArray()[conn.m_c1GlobIdx];

                        RigGridBase* hostGrid = cell.hostGrid();
                        size_t gridLocalCellIndex = cell.gridLocalCellIndex();

                        size_t i, j, k;
                        if (hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k))
                        {
                            // Adjust to 1-based Eclipse indexing
                            i++;
                            j++;
                            k++;

                            QString gridName = QString::fromStdString(hostGrid->gridName());
                            text.append(QString("NNC 1 : cell [%1, %2, %3] face %4 (%5)\n").arg(i).arg(j).arg(k).arg(face.text()).arg(gridName));
                        }
                    }

                    // Second cell of NNC
                    {
                        CVF_ASSERT(conn.m_c2GlobIdx < grid->globalCellArray().size());
                        const RigCell& cell = grid->globalCellArray()[conn.m_c2GlobIdx];

                        RigGridBase* hostGrid = cell.hostGrid();
                        size_t gridLocalCellIndex = cell.gridLocalCellIndex();

                        size_t i, j, k;
                        if (hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k))
                        {
                            // Adjust to 1-based Eclipse indexing
                            i++;
                            j++;
                            k++;

                            QString gridName = QString::fromStdString(hostGrid->gridName());
                            cvf::StructGridInterface::FaceEnum oppositeFaceEnum(cvf::StructGridInterface::oppositeFace(face));
                            QString faceText = oppositeFaceEnum.text();

                            text.append(QString("NNC 2 : cell [%1, %2, %3] face %4 (%5)\n").arg(i).arg(j).arg(k).arg(faceText).arg(gridName));
                        }
                    }
                }
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::appendDetails(QString& text, const QString& details)
{
    if (!details.isEmpty())
    {
        text += "\n";
        text += details;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::cellResultText(RimEclipseCellColors* resultColors)
{
    QString text;

    if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->eclipseCaseData())
    {
        RigEclipseCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->eclipseCaseData();
        RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);

        if (resultColors->isTernarySaturationSelected())
        {
            RimReservoirCellResultsStorage* gridCellResults = m_reservoirView->cellResult()->currentGridCellResults();
            if (gridCellResults)
            {
                size_t soilScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
                size_t sgasScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
                size_t swatScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");

                RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultColors->porosityModel());

                cvf::ref<RigResultAccessor> dataAccessObjectX = RigResultAccessorFactory::createFromResultIdx(eclipseCaseData, m_gridIndex, porosityModel, m_timeStepIndex, soilScalarSetIndex);
                cvf::ref<RigResultAccessor> dataAccessObjectY = RigResultAccessorFactory::createFromResultIdx(eclipseCaseData, m_gridIndex, porosityModel, m_timeStepIndex, sgasScalarSetIndex);
                cvf::ref<RigResultAccessor> dataAccessObjectZ = RigResultAccessorFactory::createFromResultIdx(eclipseCaseData, m_gridIndex, porosityModel, m_timeStepIndex, swatScalarSetIndex);

                double scalarValue = 0.0;

                if (dataAccessObjectX.notNull()) scalarValue = dataAccessObjectX->cellScalar(m_cellIndex);
                else scalarValue = 0.0;
                text += QString("SOIL : %1 ").arg(scalarValue);

                if (dataAccessObjectY.notNull()) scalarValue = dataAccessObjectY->cellScalar(m_cellIndex);
                else scalarValue = 0.0;
                text += QString("SGAS : %1 ").arg(scalarValue);

                if (dataAccessObjectZ.notNull()) scalarValue = dataAccessObjectZ->cellScalar(m_cellIndex);
                else scalarValue = 0.0;
                text += QString("SWAT : %1 ").arg(scalarValue);
            }
        }
        else
        {
            size_t adjustedTimeStep = m_timeStepIndex;
            if (resultColors->hasStaticResult())
            {
                adjustedTimeStep = 0;
            }
            
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createFromResultDefinition(eclipseCaseData, m_gridIndex, adjustedTimeStep, resultColors);
            if (resultAccessor.notNull())
            {
                double scalarValue = resultAccessor->cellFaceScalar(m_cellIndex, m_face);
                QString resultVar = resultColors->resultVariableUiName();

                QString resultValueText;
                if (resultColors->hasCategoryResult())
                {
                    RimLegendConfig* legendConfig = resultColors->legendConfig();

                    resultValueText += legendConfig->categoryNameFromCategoryValue(scalarValue);
                }
                else
                {
                    resultValueText = QString("%1").arg(scalarValue);
                }

                text = QString("%1 : %2").arg(resultVar).arg(resultValueText);
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::wellResultText()
{
    QString text;

    if (m_reservoirView->eclipseCase() &&
        m_reservoirView->eclipseCase()->eclipseCaseData() )
    {
        cvf::Collection<RigSingleWellResultsData> wellResults = m_reservoirView->eclipseCase()->eclipseCaseData()->wellResults();
        for (size_t i = 0; i < wellResults.size(); i++)
        {
            RigSingleWellResultsData* singleWellResultData = wellResults.at(i);

            if (!singleWellResultData->hasWellResult(m_timeStepIndex))
            {
                continue;
            }

            const RigWellResultFrame& wellResultFrame = singleWellResultData->wellResultFrame(m_timeStepIndex);
            const RigWellResultPoint* wellResultCell = wellResultFrame.findResultCell(m_gridIndex, m_cellIndex);
            if (wellResultCell)
            {
                text += QString("-- Well-cell connection info --\n Well Name: %1\n Branch Id: %2\n Segment Id: %3\n").arg(singleWellResultData->m_wellName).arg(wellResultCell->m_ertBranchId).arg(wellResultCell->m_ertSegmentId);
            }
        }
    }

    return text;
}

