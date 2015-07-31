/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipsePropertyFilter.h"

#include "RigCaseCellResultsData.h"

#include "RimEclipsePropertyFilterCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimEclipseResultDefinition.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cvfAssert.h"
#include "cvfMath.h"


namespace caf
{ // Obsolete stuff
    template<>
    void caf::AppEnum< RimEclipsePropertyFilter::EvaluationRegionType>::setUp()
    {
        addItem(RimEclipsePropertyFilter::RANGE_FILTER_REGION, "RANGE_FILTER_REGION",  "Range filter cells");
        addItem(RimEclipsePropertyFilter::GLOBAL_REGION,       "GLOBAL_REGION",        "All cells");
        setDefault(RimEclipsePropertyFilter::RANGE_FILTER_REGION);
    }
}


CAF_PDM_SOURCE_INIT(RimEclipsePropertyFilter, "CellPropertyFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter::RimEclipsePropertyFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Cell Property Filter", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&obsoleteField_evaluationRegion, "EvaluationRegion", "Evaluation region", "", "", "");
    obsoleteField_evaluationRegion.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    obsoleteField_evaluationRegion.capability<caf::PdmXmlFieldHandle>()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&resultDefinition, "ResultDefinition", "Result definition", "", "", "");
    resultDefinition = new RimEclipseResultDefinition();
    
    // MODOTODO
    // How to handle this???
/*
    // Take ownership of the fields in RimResultDefinition to be able to trap fieldChangedByUi in this class
    resultDefinition->m_resultTypeUiField.setparOwnerObject(this);
    resultDefinition->m_resultTypeUiField.capability<caf::PdmUiFieldHandle>()->setUiName("");
    resultDefinition->m_porosityModelUiField.setOwnerObject(this);
    resultDefinition->m_porosityModelUiField.capability<caf::PdmUiFieldHandle>()->setUiName("");
    resultDefinition->m_resultVariableUiField.setOwnerObject(this);
    resultDefinition->m_resultVariableUiField.capability<caf::PdmUiFieldHandle>()->setUiName("");
*/

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    resultDefinition.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);

    CAF_PDM_InitField(&lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    lowerBound.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&upperBound, "UpperBound", 0.0, "Max", "", "", "");
    upperBound.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    updateIconState();

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter::~RimEclipsePropertyFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
        setToDefaultValues();
        m_parentContainer->fieldChangedByUi(changedField, oldValue,  newValue);
        updateFilterName();
    }

    if (   &lowerBound == changedField 
        || &upperBound == changedField
        || &obsoleteField_evaluationRegion == changedField
        || &isActive == changedField
        || &filterMode == changedField)
    {
        m_parentContainer->fieldChangedByUi(changedField, oldValue, newValue);
        updateFilterName();
        this->updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipsePropertyFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionItems = resultDefinition->calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    std::vector<int> indicesToRemove;
    for (int i = 0; i < optionItems.size(); i++)
    {
        QString text = optionItems[i].optionUiText;

        if (RimDefines::isPerCellFaceResult(text))
        {
            indicesToRemove.push_back(i);
        }
    }

    std::sort(indicesToRemove.begin(), indicesToRemove.end());
    
    std::vector<int>::reverse_iterator rit;
    for (rit = indicesToRemove.rbegin(); rit != indicesToRemove.rend(); ++rit)
    {
        optionItems.takeAt(*rit);
    }

    return optionItems;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setParentContainer(RimEclipsePropertyFilterCollection* parentContainer)
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RimEclipsePropertyFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setToDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    computeResultValueRange();

    lowerBound = m_minimumResultValue;
    upperBound = m_maximumResultValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    // Fields declared in RimCellFilter
    uiOrdering.add(&name);

    // Fields declared in RimResultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    group1->add(&(resultDefinition->m_resultTypeUiField));
    group1->add(&(resultDefinition->m_porosityModelUiField));
    group1->add(&(resultDefinition->m_resultVariableUiField));
    
    // Fields declared in RimCellFilter
    uiOrdering.add(&isActive);
    uiOrdering.add(&filterMode);

    // Fields declared in this class (RimCellPropertyFilter)
    uiOrdering.add(&lowerBound);
    uiOrdering.add(&upperBound);
    uiOrdering.add(&filterMode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::computeResultValueRange()
{
    CVF_ASSERT(m_parentContainer);

    double min = 0.0;
    double max = 0.0;

    size_t scalarIndex = resultDefinition->scalarResultIndex();
    if (scalarIndex != cvf::UNDEFINED_SIZE_T)
    {
        RimReservoirCellResultsStorage* results = resultDefinition->currentGridCellResults();
        if (results)
        {
            results->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
        }
    }

    m_maximumResultValue = max;
    m_minimumResultValue = min;

    lowerBound.capability<caf::PdmUiFieldHandle>()->setUiName(QString("Min (%1)").arg(min));
    upperBound.capability<caf::PdmUiFieldHandle>()->setUiName(QString("Max (%1)").arg(max));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateFilterName()
{
    QString newFiltername;
    newFiltername = resultDefinition->resultVariable() + " ("
     + QString::number(lowerBound()) + " .. " + QString::number(upperBound) + ")";
    this->name = newFiltername;
    RiuMainWindow::instance()->forceProjectTreeRepaint();
}

