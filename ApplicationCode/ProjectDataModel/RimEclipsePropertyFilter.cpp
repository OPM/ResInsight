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
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimViewController.h"

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
{
    CAF_PDM_InitObject("Cell Property Filter", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&obsoleteField_evaluationRegion, "EvaluationRegion", "Evaluation region", "", "", "");
    obsoleteField_evaluationRegion.uiCapability()->setUiHidden(true);
    obsoleteField_evaluationRegion.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&resultDefinition, "ResultDefinition", "Result definition", "", "", "");
    resultDefinition = new RimEclipseResultDefinition();

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    resultDefinition.uiCapability()->setUiHidden(true);
    resultDefinition.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitField(&lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    lowerBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&upperBound, "UpperBound", 0.0, "Max", "", "", "");
    upperBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    updateIconState();

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter::~RimEclipsePropertyFilter()
{
    delete resultDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&name == changedField)
    {
    }

    if (   &lowerBound == changedField 
        || &upperBound == changedField
        || &obsoleteField_evaluationRegion == changedField
        || &isActive == changedField
        || &filterMode == changedField)
    {
        updateFilterName();
        this->updateIconState();
        this->uiCapability()->updateConnectedEditors();

        parentContainer()->updateDisplayModelNotifyManagedViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RimEclipsePropertyFilter::parentContainer()
{
    return dynamic_cast<RimEclipsePropertyFilterCollection*>(this->parentField()->ownerObject());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setToDefaultValues()
{
    CVF_ASSERT(parentContainer());

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

    updateReadOnlyStateOfAllFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    PdmObject::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);

    updateActiveState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateReadOnlyStateOfAllFields()
{
    bool readOnlyState = isPropertyFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields;
    this->fields(objFields);

    // Include fields declared in RimResultDefinition
    objFields.push_back(&(resultDefinition->m_resultTypeUiField));
    objFields.push_back(&(resultDefinition->m_porosityModelUiField));
    objFields.push_back(&(resultDefinition->m_resultVariableUiField));

    for (size_t i = 0; i < objFields.size(); i++)
    {
        objFields[i]->uiCapability()->setUiReadOnly(readOnlyState);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilter::isPropertyFilterControlled()
{
    RimView* rimView = NULL;
    firstAnchestorOrThisOfType(rimView);
    CVF_ASSERT(rimView);

    bool isPropertyFilterControlled = false;
    RimViewController* vc = rimView->viewController();
    if (vc && vc->isPropertyFilterOveridden())
    {
        isPropertyFilterControlled = true;
    }

    return isPropertyFilterControlled;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateActiveState()
{
    isActive.uiCapability()->setUiReadOnly(isPropertyFilterControlled());
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
    CVF_ASSERT(parentContainer());

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

    lowerBound.uiCapability()->setUiName(QString("Min (%1)").arg(min));
    upperBound.uiCapability()->setUiName(QString("Max (%1)").arg(max));
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

    uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::initAfterRead()
{
    resultDefinition->initAfterRead();

    resultDefinition->setEclipseCase(parentContainer()->reservoirView()->eclipseCase());
    resultDefinition->loadResult();
    updateIconState();
    computeResultValueRange();
}

