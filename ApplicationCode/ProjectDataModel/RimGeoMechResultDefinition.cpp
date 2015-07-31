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

#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimDefines.h"
#include "RimGeoMechCase.h"
#include "RifGeoMechReaderInterface.h"
#include "cafPdmUiListEditor.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartResultsCollection.h"
#include "RiuMainWindow.h"
#include "RimGeoMechPropertyFilter.h"
#include "RigFemResultAddress.h"

namespace caf {

template<>
void caf::AppEnum< RigFemResultPosEnum >::setUp()
{
    addItem(RIG_NODAL,            "NODAL",            "Nodal");
    addItem(RIG_ELEMENT_NODAL,    "ELEMENT_NODAL",    "Element Nodal");
    addItem(RIG_INTEGRATION_POINT,"INTEGRATION_POINT","Integration Point");
    setDefault(RIG_NODAL);
}
}


CAF_PDM_SOURCE_INIT(RimGeoMechResultDefinition, "GeoMechResultDefinition");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition::RimGeoMechResultDefinition(void)
{  
    CAF_PDM_InitObject("Color Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultPositionType, "ResultPositionType" , "Result Position", "", "", "");
    m_resultPositionType.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    CAF_PDM_InitField(&m_resultFieldName, "ResultFieldName", QString(""), "Field Name", "", "", "");
    m_resultFieldName.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
    CAF_PDM_InitField(&m_resultComponentName, "ResultComponentName", QString(""), "Component", "", "", "");
    m_resultComponentName.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position", "", "", "");
    m_resultPositionTypeUiField.capability<caf::PdmXmlFieldHandle>()->setIOWritable(false);
    m_resultPositionTypeUiField.capability<caf::PdmXmlFieldHandle>()->setIOReadable(false);

    CAF_PDM_InitField(&m_resultVariableUiField, "ResultVariableUI", QString(""), "Value", "", "", "");
    m_resultVariableUiField.capability<caf::PdmXmlFieldHandle>()->setIOWritable(false);
    m_resultVariableUiField.capability<caf::PdmXmlFieldHandle>()->setIOReadable(false);

    m_resultVariableUiField.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultVariableUiField.capability<caf::PdmUiFieldHandle>()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition::~RimGeoMechResultDefinition(void)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechResultDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (m_reservoirView)
    {

       

        if (&m_resultVariableUiField == fieldNeedingOptions)
        {
            std::map<std::string, std::vector<std::string> >  fieldCompNames = getResultMetaDataForUIFieldSetting();
            QStringList uiVarNames;
            QStringList varNames;
            getUiAndResultVariableStringList(&uiVarNames, &varNames, fieldCompNames);

            for (int oIdx = 0; oIdx < uiVarNames.size(); ++oIdx)
            {
                options.push_back(caf::PdmOptionItemInfo(uiVarNames[oIdx], varNames[oIdx]));
            }

#if 0

            for (auto fieldIt = fieldCompNames.begin(); fieldIt != fieldCompNames.end(); ++fieldIt)
            {
                options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(fieldIt->first), QString::fromStdString(fieldIt->first)));

                for (auto compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
                {
                    options.push_back(caf::PdmOptionItemInfo(QString::fromStdString("   " +  *compIt), QString::fromStdString(fieldIt->first + " " + *compIt)));
                }
            }
#endif
        }
    }

    return options;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setReservoirView(RimGeoMechView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechResultDefinition::reservoirView()
{
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if(&m_resultPositionTypeUiField == changedField)
    {
        std::map<std::string, std::vector<std::string> >  fieldCompNames = getResultMetaDataForUIFieldSetting();
        QStringList uiVarNames;
        QStringList varNames;
        getUiAndResultVariableStringList(&uiVarNames, &varNames, fieldCompNames);

        if (m_resultPositionTypeUiField() == m_resultPositionType()
            && varNames.contains(composeUiVarString(m_resultFieldName(), m_resultComponentName())))
        {
            m_resultVariableUiField = composeUiVarString(m_resultFieldName(), m_resultComponentName());
        }
        else
        {
            m_resultVariableUiField = "";
        }

    }

    if (&m_resultVariableUiField == changedField)
    {
        QStringList fieldComponentNames = m_resultVariableUiField().split(QRegExp("\\s+"));
        if (fieldComponentNames.size() > 0)
        {
            m_resultPositionType = m_resultPositionTypeUiField;
            m_resultFieldName = fieldComponentNames[0];
            if (fieldComponentNames.size() > 1)
            {
                m_resultComponentName = fieldComponentNames[1];
            }
            else
            {
                m_resultComponentName = "";
            }

            if (m_reservoirView->geoMechCase()->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress()))
            {
                m_reservoirView->hasUserRequestedAnimation = true;
            }
            
            if (m_propertyFilter)
            {
                m_propertyFilter->setToDefaultValues();

                ((RimView*)reservoirView())->scheduleGeometryRegen(PROPERTY_FILTERED);
            }

            reservoirView()->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (m_propertyFilter)
    {
        m_propertyFilter->updateConnectedEditors();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RimGeoMechResultDefinition::getResultMetaDataForUIFieldSetting()
{
    RimGeoMechCase* gmCase = m_reservoirView->geoMechCase();
    if (gmCase && gmCase->geoMechData())
    {
        return gmCase->geoMechData()->femPartResults()->scalarFieldAndComponentNames(m_resultPositionTypeUiField());
    }
    else
    {
        return std::map<std::string, std::vector<std::string> >() ;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::getUiAndResultVariableStringList(QStringList* uiNames, QStringList* variableNames, 
    const std::map<std::string, std::vector<std::string> >&  fieldCompNames)
{
    CVF_ASSERT(uiNames && variableNames);

    std::map<std::string, std::vector<std::string> >::const_iterator fieldIt;
    for (fieldIt = fieldCompNames.begin(); fieldIt != fieldCompNames.end(); ++fieldIt)
    {
        QString resultFieldName = QString::fromStdString(fieldIt->first);

        if (resultFieldName == "E" || resultFieldName == "S") continue; // We will not show the native Stress and Strain

        QString resultFieldUiName = convertToUiResultFieldName(resultFieldName);

        uiNames->push_back(resultFieldUiName);
        variableNames->push_back(resultFieldName);

        std::vector<std::string>::const_iterator compIt;
        for (compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
        {
            QString resultCompName = QString::fromStdString(*compIt);
            uiNames->push_back("   " + resultCompName);
            variableNames->push_back(composeUiVarString(resultFieldName, resultCompName));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::composeUiVarString(const QString& resultFieldName, const QString& resultComponentName)
{
    return resultFieldName + " " + resultComponentName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::initAfterRead()
{
    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField = composeUiVarString(m_resultFieldName(), m_resultComponentName());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::loadResult()
{
    if (m_reservoirView->geoMechCase())
    {
        m_reservoirView->geoMechCase()->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechResultDefinition::ownerCaseData()
{
    return m_reservoirView->geoMechCase()->geoMechData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechResultDefinition::hasResult()
{
    return ownerCaseData()->femPartResults()->assertResultsLoaded(this->resultAddress());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultFieldUiName()
{
    return convertToUiResultFieldName(m_resultFieldName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultComponentUiName()
{
    return m_resultComponentName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUiResultFieldName(QString resultFieldName)
{
    if (resultFieldName == "E") return "NativeAbaqus Strain";
    if (resultFieldName == "S") return "NativeAbaqus Stress";
    if (resultFieldName == "NE") return "E"; // Make NE and NS appear as E and SE

    return resultFieldName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setOwnerPropertyFilter( RimGeoMechPropertyFilter* propertyFilter )
{
    m_propertyFilter = propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setResultAddress( const RigFemResultAddress& resultAddress )
{
    m_resultPositionType    = resultAddress.resultPosType;
    m_resultFieldName       = QString::fromStdString(resultAddress.fieldName);
    m_resultComponentName   = QString::fromStdString(resultAddress.componentName);

    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField = composeUiVarString(m_resultFieldName(), m_resultComponentName());
}
