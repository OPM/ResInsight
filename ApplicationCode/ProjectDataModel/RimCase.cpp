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

#include "RifReaderEclipseOutput.h"
#include "RifReaderMockModel.h"

#include "RimCase.h"
#include "RimReservoirView.h"

#include "RigCaseData.h"
#include "RigMainGrid.h"
#include "RigCaseCellResultsData.h"

#include "cvfAssert.h"

#include "cafPdmUiPushButtonEditor.h"

#include <QString>
#include "RimProject.h"
#include "RimReservoirCellResultsCacher.h"

CAF_PDM_SOURCE_INIT(RimCase, "RimReservoir");

//------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::RimCase()
{
    CAF_PDM_InitField(&caseName, "CaseName",  QString(), "Case name", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&reservoirViews, "ReservoirViews", "",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_matrixModelResults, "MatrixModelResults", "",  "", "", "");
    m_matrixModelResults.setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_fractureModelResults, "FractureModelResults", "",  "", "", "");
    m_fractureModelResults.setUiHidden(true);

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults = new RimReservoirCellResultsStorage;

    this->setReservoirData( NULL );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::~RimCase()
{
    reservoirViews.deleteAllChildObjects();

    delete m_matrixModelResults();
    delete m_fractureModelResults();

    if (this->reservoirData())
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT(this->reservoirData()->refCount() == 1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseData* RimCase::reservoirData()
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseData* RimCase::reservoirData() const
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::initAfterRead()
{
    size_t j;
    for (j = 0; j < reservoirViews().size(); j++)
    {
        RimReservoirView* riv = reservoirViews()[j];
        CVF_ASSERT(riv);

        riv->setEclipseCase(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimCase::createAndAddReservoirView()
{
    // If parent is collection, and number of views is zero, make sure rig is set to NULL to initiate normal case loading
    if (parentCaseCollection() != NULL && reservoirViews().size() == 0)
    {
        if (this->reservoirData())
        {
            CVF_ASSERT(this->reservoirData()->refCount() == 1);
        }

        this->setReservoirData( NULL );
    }

    RimReservoirView* riv = new RimReservoirView();
    riv->setEclipseCase(this);

    size_t i = reservoirViews().size();
    riv->name = QString("View %1").arg(i + 1);

    reservoirViews().push_back(riv);

    return riv;
}

//--------------------------------------------------------------------------------------------------
/// TODO: Move this functionality to PdmPointersField
//--------------------------------------------------------------------------------------------------
void RimCase::removeReservoirView(RimReservoirView* reservoirView)
{
    std::vector<size_t> indices;

    size_t i;
    for (i = 0; i < reservoirViews().size(); i++)
    {
        if (reservoirViews()[i] == reservoirView)
        {
            indices.push_back(i);
        }
    }

    // NB! Make sure the ordering goes from large to low index
    while (!indices.empty())
    {
        reservoirViews().erase(indices.back());
        indices.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::removeResult(const QString& resultName)
{
    size_t i;
    for (i = 0; i < reservoirViews().size(); i++)
    {
        RimReservoirView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        RimResultSlot* result = reservoirView->cellResult;
        CVF_ASSERT(result);

        bool rebuildDisplayModel = false;

        // Set cell result variable to none if displaying 
        if (result->resultVariable() == resultName)
        {
            result->resultVariable.v() = RimDefines::undefinedResultName();
            result->loadResult();

            rebuildDisplayModel = true;
        }

        std::list< caf::PdmPointer< RimCellPropertyFilter > >::iterator it;
        RimCellPropertyFilterCollection* propFilterCollection = reservoirView->propertyFilterCollection();
        for (it = propFilterCollection->propertyFilters.v().begin(); it != propFilterCollection->propertyFilters.v().end(); ++it)
        {
            RimCellPropertyFilter* propertyFilter = *it;
            if (propertyFilter->resultDefinition->resultVariable.v() == resultName)
            {
                propertyFilter->resultDefinition->resultVariable.v() = RimDefines::undefinedResultName();
                propertyFilter->resultDefinition->loadResult();
                propertyFilter->setDefaultValues();

                rebuildDisplayModel = true;
            }
        }

        if (rebuildDisplayModel)
        {
            reservoirViews()[i]->createDisplayModelAndRedraw();
        }


        // TODO
        // CellEdgeResults are not considered, as they do not support display of input properties yet
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &releaseResultMemory)
    {
        if (this->reservoirData())
        {
            for (size_t i = 0; i < reservoirViews().size(); i++)
            {
                RimReservoirView* reservoirView = reservoirViews()[i];
                CVF_ASSERT(reservoirView);

                RimResultSlot* result = reservoirView->cellResult;
                CVF_ASSERT(result);

                result->resultVariable.v() = RimDefines::undefinedResultName();
                result->loadResult();

                RimCellEdgeResultSlot* cellEdgeResult = reservoirView->cellEdgeResult;
                CVF_ASSERT(cellEdgeResult);

                cellEdgeResult->resultVariable.v() = RimDefines::undefinedResultName();
                cellEdgeResult->loadResult();

                reservoirView->createDisplayModelAndRedraw();
            }

            RigCaseCellResultsData* matrixModelResults = reservoirData()->results(RifReaderInterface::MATRIX_RESULTS);
            if (matrixModelResults)
            {
                matrixModelResults->clearAllResults();
            }

            RigCaseCellResultsData* fractureModelResults = reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS);
            if (fractureModelResults)
            {
                fractureModelResults->clearAllResults();
            }
        }

        releaseResultMemory = oldValue.toBool();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::computeCachedData()
{
    RigCaseData* rigEclipseCase = reservoirData();
    if (rigEclipseCase)
    {
        rigEclipseCase->computeActiveCellBoundingBoxes();

        rigEclipseCase->mainGrid()->computeCachedData();

        std::vector<RigGridBase*> grids;
        rigEclipseCase->allGrids(&grids);

        size_t i;
        for (i = 0; i < grids.size(); i++)
        {
            grids[i]->computeFaults();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimCase::parentCaseCollection()
{
    std::vector<RimCaseCollection*> parentObjects;
    this->parentObjectsOfType(parentObjects);

    if (parentObjects.size() > 0)
    {
        return parentObjects[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::setReservoirData(RigCaseData* eclipseCase)
{
    m_rigEclipseCase  = eclipseCase;
    if (this->reservoirData())
    {
        m_fractureModelResults()->setCellResults(reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS));
        m_matrixModelResults()->setCellResults(reservoirData()->results(RifReaderInterface::MATRIX_RESULTS));
        m_fractureModelResults()->setMainGrid(this->reservoirData()->mainGrid());
        m_matrixModelResults()->setMainGrid(this->reservoirData()->mainGrid());
    }
    else
    {
        m_fractureModelResults()->setCellResults(NULL);
        m_matrixModelResults()->setCellResults(NULL);
        m_fractureModelResults()->setMainGrid(NULL);
        m_matrixModelResults()->setMainGrid(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimCase::results(RifReaderInterface::PorosityModelResultType porosityModel)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

