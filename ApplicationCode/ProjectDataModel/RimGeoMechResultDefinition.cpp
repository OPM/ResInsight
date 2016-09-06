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

#include "RifGeoMechReaderInterface.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"

#include "RimDefines.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechView.h"
#include "RimViewLinker.h"
#include "RimWellLogCurve.h"

#include "cafPdmUiListEditor.h"

namespace caf {

template<>
void caf::AppEnum< RigFemResultPosEnum >::setUp()
{
    addItem(RIG_NODAL,            "NODAL",            "Nodal");
    addItem(RIG_ELEMENT_NODAL,    "ELEMENT_NODAL",    "Element Nodal");
    addItem(RIG_INTEGRATION_POINT,"INTEGRATION_POINT","Integration Point");
    addItem(RIG_FORMATION_NAMES, "FORMATION_NAMES", "Formation Names");
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
    m_resultPositionType.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_resultFieldName, "ResultFieldName", QString(""), "Field Name", "", "", "");
    m_resultFieldName.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&m_resultComponentName, "ResultComponentName", QString(""), "Component", "", "", "");
    m_resultComponentName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position", "", "", "");
    m_resultPositionTypeUiField.xmlCapability()->setIOWritable(false);
    m_resultPositionTypeUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_resultVariableUiField, "ResultVariableUI", QString(""), "Value", "", "", "");
    m_resultVariableUiField.xmlCapability()->setIOWritable(false);
    m_resultVariableUiField.xmlCapability()->setIOReadable(false);

    m_resultVariableUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultVariableUiField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
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

    if (m_geomCase)
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
        }
    }

    return options;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setGeoMechCase(RimGeoMechCase* geomCase)
{
    m_geomCase = geomCase;
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
            && varNames.contains(composeFieldCompString(m_resultFieldName(), m_resultComponentName())))
        {
            m_resultVariableUiField = composeFieldCompString(m_resultFieldName(), m_resultComponentName());
        }
        else
        {
            m_resultVariableUiField = "";
        }
    }

    // Get the possible property filter owner
    RimGeoMechPropertyFilter* propFilter = dynamic_cast<RimGeoMechPropertyFilter*>(this->parentField()->ownerObject());
    RimView* view = NULL;
    this->firstAnchestorOrThisOfType(view);
    RimWellLogCurve* curve = NULL;
    this->firstAnchestorOrThisOfType(curve);


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


            if (m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress()))
            {
                if (view) view->hasUserRequestedAnimation = true;
            }
            
            if (propFilter)
            {
                propFilter->setToDefaultValues();

                if (view) view->scheduleGeometryRegen(PROPERTY_FILTERED);
            }

            if (view) view->scheduleCreateDisplayModelAndRedraw();

            if (dynamic_cast<RimGeoMechCellColors*>(this))
            {
                if (view)
                {
                    RimViewLinker* viewLinker = view->assosiatedViewLinker();
                    if (viewLinker)
                    {
                        viewLinker->updateCellResult();
                    }
                }
            }

            if (curve)
            {
                curve->loadDataAndUpdate();
            }
        }
    }
      
    if (propFilter)
    {
        propFilter->updateConnectedEditors();
    }

    if (curve)
    {
        curve->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RimGeoMechResultDefinition::getResultMetaDataForUIFieldSetting()
{
    RimGeoMechCase* gmCase = m_geomCase;
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

        if (resultFieldName == "E" || resultFieldName == "S" || resultFieldName == "POR") continue; // We will not show the native POR, Stress and Strain

        QString resultFieldUiName = convertToUiResultFieldName(resultFieldName);

        uiNames->push_back(resultFieldUiName);
        variableNames->push_back(resultFieldName);

        std::vector<std::string>::const_iterator compIt;
        for (compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
        {
            QString resultCompName = QString::fromStdString(*compIt);
            uiNames->push_back("   " + convertToUIComponentName(resultCompName));
            variableNames->push_back(composeFieldCompString(resultFieldName, resultCompName));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::composeFieldCompString(const QString& resultFieldName, const QString& resultComponentName)
{
    if (resultComponentName.isEmpty())
        return resultFieldName;
    else
        return resultFieldName + " " + resultComponentName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::initAfterRead()
{
    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField = composeFieldCompString(m_resultFieldName(), m_resultComponentName());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::loadResult()
{
    if (m_geomCase)
    {
        m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechResultDefinition::ownerCaseData()
{
    return m_geomCase->geoMechData();
}

//--------------------------------------------------------------------------------------------------
/// Is the result probably valid and possible to load
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
    return convertToUIComponentName(m_resultComponentName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUiResultFieldName(QString resultFieldName)
{
    if (resultFieldName == "E") return "NativeAbaqus Strain";
    if (resultFieldName == "S") return "NativeAbaqus Stress";
    if (resultFieldName == "NE") return "E"; // Make NE and NS appear as E and SE
    if (resultFieldName == "POR-Bar") return "POR"; // POR-Bar appear as POR

    return resultFieldName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUIComponentName(QString resultComponentName)
{
    return resultComponentName;
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
    m_resultVariableUiField = composeFieldCompString(m_resultFieldName(), m_resultComponentName());
}
