/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechPropertyFilter.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigFormationNames.h"

#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimViewController.h"

#include "RiuMainWindow.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfAssert.h"
#include "cvfMath.h"

CAF_PDM_SOURCE_INIT(RimGeoMechPropertyFilter, "GeoMechPropertyFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter::RimGeoMechPropertyFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("Property Filter", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&resultDefinition, "ResultDefinition", "Result Definition", "", "", "");
    resultDefinition = new RimGeoMechResultDefinition();

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    resultDefinition.uiCapability()->setUiHidden(true);
    resultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

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
RimGeoMechPropertyFilter::~RimGeoMechPropertyFilter()
{
    delete resultDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (   &lowerBound == changedField
        || &upperBound == changedField
        || &isActive == changedField
        || &filterMode == changedField
        || &m_selectedCategoryValues == changedField)
    {
        this->updateIconState();
        this->updateFilterName();
        this->uiCapability()->updateConnectedEditors();

        parentContainer()->updateDisplayModelNotifyManagedViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setParentContainer(RimGeoMechPropertyFilterCollection* parentContainer)
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection* RimGeoMechPropertyFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setToDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    computeResultValueRange();

    lowerBound = m_minimumResultValue;
    upperBound = m_maximumResultValue;

    m_selectedCategoryValues = m_categoryValues;

    this->updateFilterName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    resultDefinition->uiOrdering(uiConfigName, *group1);

    caf::PdmUiGroup& group2 = *(uiOrdering.addNewGroup("Filter Settings"));

    group2.add(&filterMode);

    if ( resultDefinition->hasCategoryResult() )
    {
        group2.add(&m_selectedCategoryValues);
    }
    else
    {
        group2.add(&lowerBound);
        group2.add(&upperBound);
    }

    updateReadOnlyStateOfAllFields();

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    PdmObject::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);

    updateActiveState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::updateReadOnlyStateOfAllFields()
{
    bool readOnlyState = isPropertyFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields;
    this->fields(objFields);

    // Include fields declared in RimResultDefinition
    objFields.push_back(&(resultDefinition->m_resultPositionTypeUiField));
    objFields.push_back(&(resultDefinition->m_resultVariableUiField));
    objFields.push_back(&(resultDefinition->m_isTimeLapseResultUiField));
    objFields.push_back(&(resultDefinition->m_timeLapseBaseTimestepUiField));

    for (size_t i = 0; i < objFields.size(); i++)
    {
        objFields[i]->uiCapability()->setUiReadOnly(readOnlyState);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilter::isPropertyFilterControlled()
{
    bool isPropertyFilterControlled = false;

    Rim3dView* rimView = NULL;
    firstAncestorOrThisOfType(rimView);
    CVF_ASSERT(rimView);
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
void RimGeoMechPropertyFilter::updateActiveState()
{
    isActive.uiCapability()->setUiReadOnly(isPropertyFilterControlled());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilter::isActiveAndHasResult()
{
    if (this->isActive() && this->resultDefinition->hasResult())
    {
        return true;
    }
    
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
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
void RimGeoMechPropertyFilter::computeResultValueRange()
{
    CVF_ASSERT(m_parentContainer);

    double min = 0.0;
    double max = 0.0;

    clearCategories();

    RigFemResultAddress resultAddress = resultDefinition->resultAddress();
    if (resultAddress.isValid() && resultDefinition->ownerCaseData())
    {
        if (resultDefinition->hasCategoryResult())
        {
            std::vector<QString> fnVector;
            if (resultDefinition->ownerCaseData()->femPartResults()->activeFormationNames())
            {
                fnVector = resultDefinition->ownerCaseData()->femPartResults()->activeFormationNames()->formationNames();
            }
            setCategoryNames(fnVector);
        }
        else
        {
            resultDefinition->ownerCaseData()->femPartResults()->minMaxScalarValues(resultAddress, &min, &max);
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
void RimGeoMechPropertyFilter::updateFilterName()
{
    RigFemResultAddress resultAddress = resultDefinition->resultAddress();
    QString newFiltername;

    if (resultAddress.resultPosType == RIG_FORMATION_NAMES)
    {
        newFiltername = resultDefinition->resultFieldName();
    }
    else
    {
        QString posName;

        switch (resultAddress.resultPosType)
        {
            case RIG_NODAL: posName = "N"; break;
            case RIG_ELEMENT_NODAL: posName = "EN"; break;
            case RIG_INTEGRATION_POINT: posName = "IP"; break;
        }

        QString fieldUiName = resultDefinition->resultFieldUiName();
        QString compoUiName = resultDefinition->resultComponentUiName();

        newFiltername =  posName + ", " + fieldUiName + ", " + compoUiName + " ("
         + QString::number(lowerBound()) + " .. " + QString::number(upperBound) + ")";
    }

    this->name = newFiltername;

    uiCapability()->updateConnectedEditors();
}

