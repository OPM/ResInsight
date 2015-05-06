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

#include "RimGeoMechResultSlot.h"
#include "RimGeoMechView.h"
#include "RimLegendConfig.h"
#include "RimDefines.h"
#include "RimGeoMechCase.h"
#include "RifGeoMechReaderInterface.h"
#include "cafPdmUiListEditor.h"
#include "RigGeoMechCaseData.h"

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


CAF_PDM_SOURCE_INIT(RimGeoMechResultSlot, "GeoMechResultSlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::RimGeoMechResultSlot(void)
{
  
    CAF_PDM_InitObject("Color Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_resultPositionType, "ResultPositionType" , "Result Position", "", "", "");
    m_resultPositionType.setUiHidden(true);
    CAF_PDM_InitField(&m_resultFieldName, "ResultFieldName", QString(""), "Field Name", "", "", "");
    m_resultFieldName.setUiHidden(true);
    CAF_PDM_InitField(&m_resultComponentName, "ResultComponentName", QString(""), "Component", "", "", "");
    m_resultComponentName.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position", "", "", "");
    m_resultPositionTypeUiField.setIOWritable(false);
    m_resultPositionTypeUiField.setIOReadable(false);

    CAF_PDM_InitField(&m_resultVariableUiField, "ResultVariableUI", QString(""), "Value", "", "", "");
    m_resultVariableUiField.setIOWritable(false);
    m_resultVariableUiField.setIOReadable(false);

    m_resultVariableUiField.setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultVariableUiField.setUiLabelPosition(caf::PdmUiItemInfo::TOP);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::~RimGeoMechResultSlot(void)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechResultSlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
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
void RimGeoMechResultSlot::setReservoirView(RimGeoMechView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
        if (fieldComponentNames.size() == 2)
        {
            m_resultPositionType = m_resultPositionTypeUiField;
            m_resultFieldName = fieldComponentNames[0];
            m_resultComponentName = fieldComponentNames[1];
        }

        // Todo: Read results into cache ?

        if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
       
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RimGeoMechResultSlot::getResultMetaDataForUIFieldSetting()
{
    RimGeoMechCase* gmCase = m_reservoirView->geoMechCase();
    if (gmCase && gmCase->geoMechData())
    {
        return gmCase->geoMechData()->scalarFieldAndComponentNames(m_resultPositionTypeUiField());
    }
    else
    {
        return std::map<std::string, std::vector<std::string> >() ;
    }
}

void RimGeoMechResultSlot::getUiAndResultVariableStringList(QStringList* uiNames, QStringList* variableNames, 
                                                            const std::map<std::string, std::vector<std::string> >&  fieldCompNames)
{
    CVF_ASSERT(uiNames && variableNames);

    for (auto fieldIt = fieldCompNames.begin(); fieldIt != fieldCompNames.end(); ++fieldIt)
    {
        uiNames->push_back(QString::fromStdString(fieldIt->first));
        variableNames->push_back(QString::fromStdString(fieldIt->first));

        for (auto compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
        {
            uiNames->push_back(QString::fromStdString("   " +  *compIt));
            variableNames->push_back(composeUiVarString(QString::fromStdString(fieldIt->first), QString::fromStdString(*compIt)));
        }
    }
}

QString RimGeoMechResultSlot::composeUiVarString(const QString& resultFieldName, const QString& resultComponentName)
{
    return resultFieldName + " " + resultComponentName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultSlot::initAfterRead()
{
    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultVariableUiField = composeUiVarString(m_resultFieldName(), m_resultComponentName());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultSlot::loadResult()
{
   
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechResultSlot::ownerCaseData()
{
    return m_reservoirView->geoMechCase()->geoMechData();
}
