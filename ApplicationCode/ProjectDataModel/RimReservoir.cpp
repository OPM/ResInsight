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

#include "RIStdInclude.h"

#include "RifReaderEclipseOutput.h"
#include "RifReaderMockModel.h"

#include "RimReservoir.h"
#include "RimReservoirView.h"

#include "RigReservoir.h"
#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"

#include "cvfAssert.h"

#include "cafPdmUiPushButtonEditor.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoir::RimReservoir()
{
    m_rigReservoir = NULL;

    CAF_PDM_InitField(&caseName, "CaseName",  QString(), "Case name", "", "" ,"");
//     CAF_PDM_InitField(&releaseResultMemory, "ReleaseResultMemory", true, "Release result memory", "", "" ,"");
//     releaseResultMemory.setIOReadable(false);
//     releaseResultMemory.setIOWritable(false);
//     releaseResultMemory.setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&reservoirViews, "ReservoirViews", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoir* RimReservoir::reservoirData()
{
    return m_rigReservoir.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigReservoir* RimReservoir::reservoirData() const
{
    return m_rigReservoir.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoir::initAfterRead()
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
RimReservoir::~RimReservoir()
{
    reservoirViews.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimReservoir::createAndAddReservoirView()
{
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
void RimReservoir::removeReservoirView(RimReservoirView* reservoirView)
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
void RimReservoir::removeResult(const QString& resultName)
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
void RimReservoir::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &releaseResultMemory)
    {
        if (m_rigReservoir.notNull())
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

            RigReservoirCellResults* matrixModelResults = m_rigReservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS);
            if (matrixModelResults)
            {
                matrixModelResults->clearAllResults();
            }

            RigReservoirCellResults* fractureModelResults = m_rigReservoir->mainGrid()->results(RifReaderInterface::FRACTURE_RESULTS);
            if (fractureModelResults)
            {
                fractureModelResults->clearAllResults();
            }
        }

        releaseResultMemory = oldValue.toBool();
    }
}

