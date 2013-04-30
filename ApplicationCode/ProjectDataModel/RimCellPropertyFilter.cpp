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

#include "RimReservoirView.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"
#include "RigGridBase.h"
#include "RigCaseCellResultsData.h"

#include "cafPdmUiDoubleSliderEditor.h"



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
    
    // Take ownership of the fields in RimResultDefinition to be able to trap fieldChangedByUi in this class
    resultDefinition->m_resultTypeUiField.setOwnerObject(this);
    resultDefinition->m_resultTypeUiField.setUiName("");
    resultDefinition->m_porosityModelUiField.setOwnerObject(this);
    resultDefinition->m_porosityModelUiField.setUiName("");
    resultDefinition->m_resultVariableUiField.setOwnerObject(this);
    resultDefinition->m_resultVariableUiField.setUiName("");

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    resultDefinition.setUiHidden(true);

    CAF_PDM_InitField(&lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    lowerBound.setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&upperBound, "UpperBound", 0.0, "Max", "", "", "");
    upperBound.setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    updateIconState();

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
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
    if (&name == changedField)
    {
    }

    if (   &(resultDefinition->m_resultTypeUiField) == changedField 
        || &(resultDefinition->m_porosityModelUiField) == changedField)
    {
        resultDefinition->fieldChangedByUi(changedField, oldValue, newValue);
    }

    if ( &(resultDefinition->m_resultVariableUiField) == changedField )
    {
        resultDefinition->fieldChangedByUi(changedField, oldValue, newValue);
        setDefaultValues();
        m_parentContainer->fieldChangedByUi(changedField, oldValue,  newValue);
    }

    if (   &lowerBound == changedField 
        || &upperBound == changedField
        || &evaluationRegion == changedField
        || &active == changedField
        || &filterMode == changedField)
    {
        m_parentContainer->fieldChangedByUi(changedField, oldValue, newValue);
        this->updateIconState();
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
        RimReservoirCellResultsStorage* results = resultDefinition->currentGridCellResults();
        if (results)
        {
            results->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
        }
    }

    lowerBound = min;
    lowerBound.setUiName(QString("Min (%1)").arg(min));

    upperBound = max;
    upperBound.setUiName(QString("Max (%1)").arg(max));

    m_maximumResultValue = max;
    m_minimumResultValue = min;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    // Fields declared in RimCellFilter
    uiOrdering.add(&name);

    // Fields declared in RimResultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    group1->add(&(resultDefinition->m_resultTypeUiField));
    group1->add(&(resultDefinition->m_porosityModelUiField));
    group1->add(&(resultDefinition->m_resultVariableUiField));
    
    // Fields declared in RimCellFilter
    uiOrdering.add(&active);
    uiOrdering.add(&filterMode);

    // Fields declared in this class (RimCellPropertyFilter)
    uiOrdering.add(&lowerBound);
    uiOrdering.add(&upperBound);
    uiOrdering.add(&filterMode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (m_minimumResultValue == cvf::UNDEFINED_DOUBLE || m_maximumResultValue == cvf::UNDEFINED_DOUBLE)
    {
        return;
    }

    if (field == &lowerBound || field == &upperBound)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (!myAttr)
        {
            return;
        }

        myAttr->m_minimum = m_minimumResultValue;
        myAttr->m_maximum = m_maximumResultValue;
    }
}

