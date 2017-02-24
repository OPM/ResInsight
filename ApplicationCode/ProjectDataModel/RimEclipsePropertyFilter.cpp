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
#include "RigEclipseCaseData.h"
#include "RigFormationNames.h"

#include "RimEclipseCase.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimViewController.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfAssert.h"
#include "cvfMath.h"
#include "RigFlowDiagResults.h"
#include "RimFlowDiagSolution.h"


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
    resultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    m_lowerBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_upperBound, "UpperBound", 0.0, "Max", "", "", "");
    m_upperBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_useCategorySelection, "CategorySelection", false, "Category Selection", "", "", "");
    m_upperBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

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
void RimEclipsePropertyFilter::rangeValues(double* lower, double* upper) const
{
    *lower = this->m_lowerBound;
    *upper = this->m_upperBound;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilter::isCategorySelectionActive() const
{
    if (resultDefinition->hasCategoryResult() && m_useCategorySelection)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

    if (   &m_lowerBound == changedField 
        || &m_upperBound == changedField
        || &obsoleteField_evaluationRegion == changedField
        || &isActive == changedField
        || &filterMode == changedField
        || &m_selectedCategoryValues == changedField
        || &m_useCategorySelection == changedField)
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

    m_lowerBound = m_minimumResultValue;
    m_upperBound = m_maximumResultValue;

    m_selectedCategoryValues = m_categoryValues;
    m_useCategorySelection = true;

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
    resultDefinition->defineUiOrdering(uiConfigName, *group1);
    
    // Fields declared in RimCellFilter
    uiOrdering.add(&filterMode);

    if (resultDefinition->hasCategoryResult())
    {
        uiOrdering.add(&m_useCategorySelection);
    }

    if ( resultDefinition->hasCategoryResult() && m_useCategorySelection() )
    {
        uiOrdering.add(&m_selectedCategoryValues);
    }
    else
    {
        uiOrdering.add(&m_lowerBound);
        uiOrdering.add(&m_upperBound);
    }

    uiOrdering.setForgetRemainingFields(true);

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
    firstAncestorOrThisOfType(rimView);
    CVF_ASSERT(rimView);

    bool isPropertyFilterControlled = false;

    if (rimView)
    {
        RimViewController* vc = rimView->viewController();
        if (vc && vc->isPropertyFilterOveridden())
        {
            isPropertyFilterControlled = true;
        }
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

    if (field == &m_lowerBound || field == &m_upperBound)
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

    clearCategories();

    if (resultDefinition->resultType() == RimDefines::FLOW_DIAGNOSTICS)
    {
        RimView* view;
        this->firstAncestorOrThisOfType(view);

        int timeStep = 0;
        if (view) timeStep = view->currentTimeStep();
        RigFlowDiagResultAddress resAddr = resultDefinition->flowDiagResAddress();
        if ( resultDefinition->flowDiagSolution() )
        {
            RigFlowDiagResults* results = resultDefinition->flowDiagSolution()->flowDiagResults();
            results->minMaxScalarValues(resAddr, timeStep, &max, &max);

            if ( resultDefinition->hasCategoryResult() )
            {
                setCategoryNames(resultDefinition->flowDiagSolution()->tracerNames());
            }
        }
    }
    else
    {
        size_t scalarIndex = resultDefinition->scalarResultIndex();
        if ( scalarIndex != cvf::UNDEFINED_SIZE_T )
        {
            RimReservoirCellResultsStorage* results = resultDefinition->currentGridCellResults();
            if ( results )
            {
                results->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);

                if ( resultDefinition->hasCategoryResult() )
                {
                    if ( resultDefinition->resultType() != RimDefines::FORMATION_NAMES )
                    {
                        setCategoryValues(results->cellResults()->uniqueCellScalarValues(scalarIndex));
                    }
                    else
                    {
                        CVF_ASSERT(parentContainer()->reservoirView()->eclipseCase()->reservoirData());
                        CVF_ASSERT(parentContainer()->reservoirView()->eclipseCase()->reservoirData()->activeFormationNames());

                        const std::vector<QString>& fnVector = parentContainer()->reservoirView()->eclipseCase()->reservoirData()->activeFormationNames()->formationNames();
                        setCategoryNames(fnVector);
                    }
                }
            }
        }
    }
    m_maximumResultValue = max;
    m_minimumResultValue = min;

    m_lowerBound.uiCapability()->setUiName(QString("Min (%1)").arg(min));
    m_upperBound.uiCapability()->setUiName(QString("Max (%1)").arg(max));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateFilterName()
{
    QString newFiltername = resultDefinition->resultVariableUiShortName();

    if (isCategorySelectionActive())
    {
        if (m_categoryNames.size() == 0)
        {
            newFiltername += " (";

            if ( m_selectedCategoryValues().size() && m_selectedCategoryValues().size() == m_categoryValues.size() )
            {
                newFiltername += QString::number(m_selectedCategoryValues()[0]);
                newFiltername += "..";
                newFiltername += QString::number(m_selectedCategoryValues()[m_selectedCategoryValues().size() - 1]);
            }
            else
            {
                for (size_t i = 0; i < m_selectedCategoryValues().size(); i++)
                {
                    int val = m_selectedCategoryValues()[i];
                    newFiltername += QString::number(val);

                    if (i < m_selectedCategoryValues().size() - 1)
                    {
                        newFiltername += ", ";
                    }
                }
            }
            
            newFiltername += ")";
        }

    }
    else
    {
        newFiltername += " (" + QString::number(m_lowerBound) + " .. " + QString::number(m_upperBound) + ")";
    }

    this->name = newFiltername;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::initAfterRead()
{
    resultDefinition->initAfterRead();

    resultDefinition->setEclipseCase(parentContainer()->reservoirView()->eclipseCase());
    updateIconState();
}

