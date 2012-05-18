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

#include "RimReservoirView.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"
#include "RigGridBase.h"
#include "RigReservoirCellResults.h"




namespace caf
{
    template<>
    void caf::AppEnum< RimCellPropertyFilter::EvaluationRegionType>::setUp()
    {
        addItem(RimCellPropertyFilter::RANGE_FILTER_REGION, "RANGE_FILTER_REGION",  "Range filter cells");
        addItem(RimCellPropertyFilter::GLOBAL_REGION,       "GLOBAL_REGION",        "All cells");
        setDefault(RimCellPropertyFilter::RANGE_FILTER_REGION);
    }
}


CAF_PDM_SOURCE_INIT(RimCellPropertyFilter, "CellPropertyFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilter::RimCellPropertyFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Cell Property Filter", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&evaluationRegion, "EvaluationRegion", "Evaluation region", "", "", "");

    CAF_PDM_InitFieldNoDefault(&resultDefinition, "ResultDefinition", "Result definition", "", "", "");
    resultDefinition = new RimResultDefinition();
    resultDefinition.setHidden(true);

    CAF_PDM_InitField(&lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    CAF_PDM_InitField(&upperBound, "UpperBound", 0.0, "Max", "", "", "");

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilter::~RimCellPropertyFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &name)
    {
    }
    else if (&resultDefinition == changedField)
    {
        setDefaultValues();
        m_parentContainer->fieldChangedByUi(changedField, oldValue,  newValue);
    }
    else
    {
        m_parentContainer->fieldChangedByUi(changedField, oldValue, newValue);
        this->updateIconState();
    }

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellPropertyFilter::isCellRejected(const RigGridBase* grid, size_t timeStepIndex, size_t cellIndex) const
{
    CVF_ASSERT(grid);

    if (resultDefinition->gridScalarIndex() == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    size_t resultIndex = 0;
    if (resultDefinition->hasDynamicResult())
    {
        resultIndex = timeStepIndex;
    }

    double scalarValue = grid->cellScalar(resultIndex, resultDefinition->gridScalarIndex(), cellIndex);
    bool rejected = false;
    if (scalarValue < lowerBound || scalarValue > upperBound)
    {
        rejected = true;
    }

    if (filterMode == INCLUDE)
    {
        return rejected;
    }
    else
    {
        return !rejected;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::CellRangeFilter::CellStateType RimCellPropertyFilter::cellFilterState(const RigGridBase* grid, size_t timeStepIndex, size_t cellIndex) const
{
    CVF_TIGHT_ASSERT(grid);
    CVF_TIGHT_ASSERT(resultDefinition->gridScalarIndex() != cvf::UNDEFINED_SIZE_T);

    size_t tStepIdx = 0;
    if (resultDefinition->hasDynamicResult())
    {
        tStepIdx = timeStepIndex;
    }

    double scalarValue = grid->cellScalar(tStepIdx, resultDefinition->gridScalarIndex(), cellIndex);

    if ( lowerBound <= scalarValue  && scalarValue <= upperBound)
    {
        if (filterMode == INCLUDE) return cvf::CellRangeFilter::INCLUDED;
        else                       return cvf::CellRangeFilter::EXCLUDED;
    }
    else
    {
        return cvf::CellRangeFilter::NOT_INCLUDED;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCellPropertyFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    return resultDefinition->calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilter::setParentContainer(RimCellPropertyFilterCollection* parentContainer)
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilterCollection* RimCellPropertyFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilter::setDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    double min = 0.0;
    double max = 0.0;

    size_t scalarIndex = resultDefinition->gridScalarIndex();
    if (scalarIndex != cvf::UNDEFINED_SIZE_T)
    {
        RigReservoirCellResults* results = m_parentContainer->reservoirView()->gridCellResults();
        if (results)
        {
            results->minMaxCellScalarValues(scalarIndex, min, max);
        }
    }

    lowerBound = min;
    lowerBound.setUiName(QString("Min (%1)").arg(min));

    upperBound = max;
    upperBound.setUiName(QString("Max (%1)").arg(max));
}

