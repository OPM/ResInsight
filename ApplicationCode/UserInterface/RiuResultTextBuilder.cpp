/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
#include "RigCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimFaultResultSlot.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultTextBuilder::RiuResultTextBuilder(RimReservoirView* reservoirView, size_t gridIndex, size_t cellIndex, size_t timeStepIndex)
{
    CVF_ASSERT(reservoirView);
    
    m_reservoirView = reservoirView;
	m_gridIndex = gridIndex;
	m_cellIndex = cellIndex;
	m_timeStepIndex = timeStepIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::mainResultText()
{
	QString text;

	QString nncText = nncResultText();
	if (!nncText.isEmpty())
	{
		text = nncText;
	}
	else
	{
		text = gridResultText();
	}

	return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::gridResultDetails()
{
	QString text;

	if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->reservoirData())
	{
		RigCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->reservoirData();
		RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);

		this->appendTextFromResultSlot(eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_reservoirView->cellResult(), &text);
	}

	return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::faultResultDetails()
{
	QString text;

	if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->reservoirData())
	{
		RigCaseData* eclipseCaseData = m_reservoirView->eclipseCase()->reservoirData();
		RigGridBase* grid = eclipseCaseData->grid(m_gridIndex);
		RigMainGrid* mainGrid = grid->mainGrid();

		const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace(m_cellIndex, m_face);
		if (fault)
		{
			text.append(QString("Fault Name: %1\n").arg(fault->name()));

			cvf::StructGridInterface::FaceEnum faceHelper(m_face);
			text.append("Fault Face : " + faceHelper.text() + "\n");

			if (m_reservoirView->faultResultSettings()->hasValidCustomResult())
			{
				text.append("Fault result data:\n");
				this->appendTextFromResultSlot(eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_reservoirView->currentFaultResultSlot(), &text);
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
	QString text;

	return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::nncResultText()
{
	QString text;

	return text;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::appendTextFromResultSlot(RigCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimResultSlot* resultSlot, QString* resultInfoText)
{
	if (!resultSlot)
	{
		return;
	}

	RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultSlot->porosityModel());
	if (resultSlot->isTernarySaturationSelected())
	{
		RimReservoirCellResultsStorage* gridCellResults = resultSlot->currentGridCellResults();
		if (gridCellResults)
		{
			size_t soilScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SOIL");
			size_t sgasScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");
			size_t swatScalarSetIndex = gridCellResults->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");

			cvf::ref<RigResultAccessor> dataAccessObjectX = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, soilScalarSetIndex);
			cvf::ref<RigResultAccessor> dataAccessObjectY = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, sgasScalarSetIndex);
			cvf::ref<RigResultAccessor> dataAccessObjectZ = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, swatScalarSetIndex);

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
	}
	else if (resultSlot->hasResult())
	{
		RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultSlot->porosityModel());
		cvf::ref<RigResultAccessor> resultAccessor;

		if (resultSlot->hasStaticResult())
		{
			if (resultSlot->resultVariable().compare(RimDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
			{
				cvf::ref<RigResultAccessor> transResultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedTransmissibilityResultName());
				{
					double scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
					resultInfoText->append(QString("Tran X : %1\n").arg(scalarValue));

					scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
					resultInfoText->append(QString("Tran Y : %1\n").arg(scalarValue));

					scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
					resultInfoText->append(QString("Tran Z : %1\n").arg(scalarValue));
				}
			}
			else if (resultSlot->resultVariable().compare(RimDefines::combinedMultResultName(), Qt::CaseInsensitive) == 0)
			{
				cvf::ref<RigResultAccessor> multResultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedMultResultName());
				{
					double scalarValue = 0.0;

					scalarValue = multResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
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
			}
			else if (resultSlot->resultVariable().compare(RimDefines::combinedRiTransResultName(), Qt::CaseInsensitive) == 0)
			{
				cvf::ref<RigResultAccessor> transResultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiTransResultName());
				{
					double scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
					resultInfoText->append(QString("riTran X : %1\n").arg(scalarValue));

					scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
					resultInfoText->append(QString("riTran Y : %1\n").arg(scalarValue));

					scalarValue = transResultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
					resultInfoText->append(QString("riTran Z : %1\n").arg(scalarValue));
				}
			}
			else if (resultSlot->resultVariable().compare(RimDefines::combinedRiMultResultName(), Qt::CaseInsensitive) == 0)
			{
				cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiMultResultName());
				{
					double scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
					resultInfoText->append(QString("riMult X : %1\n").arg(scalarValue));

					scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
					resultInfoText->append(QString("riMult Y : %1\n").arg(scalarValue));

					scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
					resultInfoText->append(QString("riMult Z : %1\n").arg(scalarValue));
				}
			}
			else if (resultSlot->resultVariable().compare(RimDefines::combinedRiAreaNormTransResultName(), Qt::CaseInsensitive) == 0)
			{
				cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, RimDefines::combinedRiAreaNormTransResultName());
				{
					double scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_I);
					resultInfoText->append(QString("riTransByArea X : %1\n").arg(scalarValue));

					scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_J);
					resultInfoText->append(QString("riTransByArea Y : %1\n").arg(scalarValue));

					scalarValue = resultAccessor->cellFaceScalar(cellIndex, cvf::StructGridInterface::POS_K);
					resultInfoText->append(QString("riTransByArea Z : %1\n").arg(scalarValue));
				}
			}
			else
			{
				resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, 0, resultSlot->scalarResultIndex());
			}
		}
		else
		{
			resultAccessor = RigResultAccessorFactory::createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, resultSlot->scalarResultIndex());
		}

		if (resultAccessor.notNull())
		{
			double scalarValue = resultAccessor->cellScalar(cellIndex);
			resultInfoText->append(resultSlot->resultVariable());
			resultInfoText->append(QString(" : %1\n").arg(scalarValue));
		}
	}

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::cellEdgeResultDetails()
{
	QString text;

	//if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->reservoirData())
	if (m_reservoirView->cellEdgeResult()->hasResult())
	{
		size_t resultIndices[6];
		QStringList resultNames;
		m_reservoirView->cellEdgeResult()->gridScalarIndices(resultIndices);
		m_reservoirView->cellEdgeResult()->gridScalarResultNames(&resultNames);

		text.push_back("\nCell edge result data:\n");
		for (int idx = 0; idx < 6; idx++)
		{
			if (resultIndices[idx] == cvf::UNDEFINED_SIZE_T) continue;

			// Cell edge results are static, results are loaded for first time step only
			RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_reservoirView->cellResult()->porosityModel());
			cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(m_reservoirView->eclipseCase()->reservoirData(), m_gridIndex, porosityModel, 0, resultIndices[idx]);
			if (resultAccessor.notNull())
			{
				double scalarValue = resultAccessor->cellScalar(m_cellIndex);
				text.append(QString("%1 : %2\n").arg(resultNames[idx]).arg(scalarValue));
			}
		}
	}

	return text;
}
