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
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"

#include "RiaDefines.h"

#include "Rim3dWellLogCurve.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimPlotCurve.h"
#include "RimViewLinker.h"

#include "cafPdmUiListEditor.h"

namespace caf {

template<>
void caf::AppEnum< RigFemResultPosEnum >::setUp()
{
    addItem(RIG_NODAL,            "NODAL",            "Nodal");
    addItem(RIG_ELEMENT_NODAL,    "ELEMENT_NODAL",    "Element Nodal");
    addItem(RIG_INTEGRATION_POINT,"INTEGRATION_POINT","Integration Point");
    addItem(RIG_ELEMENT_NODAL_FACE, "ELEMENT_NODAL_FACE", "Element Nodal On Face");
    addItem(RIG_FORMATION_NAMES, "FORMATION_NAMES", "Formation Names");
    addItem(RIG_ELEMENT, "ELEMENT", "Element");
    addItem(RIG_WELLPATH_DERIVED, "WELLPATH_DERIVED", "Well Path Derived");
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

    CAF_PDM_InitField(&m_isTimeLapseResult, "IsTimeLapseResult", false, "TimeLapseResult", "", "", "");
    m_isTimeLapseResult.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_timeLapseBaseTimestep, "TimeLapseBaseTimeStep", 0, "Base Time Step", "", "", "");
    m_timeLapseBaseTimestep.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_compactionRefLayer, "CompactionRefLayer", 0, "Compaction Ref Layer", "", "", "");
    m_compactionRefLayer.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position", "", "", "");
    m_resultPositionTypeUiField.xmlCapability()->setIOWritable(false);
    m_resultPositionTypeUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_resultVariableUiField, "ResultVariableUI", QString(""), "Value", "", "", "");
    m_resultVariableUiField.xmlCapability()->setIOWritable(false);
    m_resultVariableUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_isTimeLapseResultUiField, "IsTimeLapseResultUI", false, "Enable Relative Result", "", "Use the difference with respect to a specific time step as the result variable to plot", "");
    m_isTimeLapseResultUiField.xmlCapability()->setIOWritable(false);
    m_isTimeLapseResultUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_timeLapseBaseTimestepUiField, "TimeLapseBaseTimeStepUI", 0, "Base Time Step", "", "", "");
    m_timeLapseBaseTimestepUiField.xmlCapability()->setIOWritable(false);
    m_timeLapseBaseTimestepUiField.xmlCapability()->setIOReadable(false);

    m_resultVariableUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultVariableUiField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitField(&m_compactionRefLayerUiField, "CompactionRefLayerUi", (int)RigFemResultAddress::NO_COMPACTION, "Compaction Ref Layer", "", "The compaction is calculated with reference to this layer. Default layer is the topmost layer with POR", "");
    m_compactionRefLayerUiField.xmlCapability()->setIOWritable(false);
    m_compactionRefLayerUiField.xmlCapability()->setIOReadable(false);

    m_isChangedByField = false;
    m_addWellPathDerivedResults = false;
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
void RimGeoMechResultDefinition::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultPositionTypeUiField);
    uiOrdering.add(&m_resultVariableUiField);

    if ( m_resultPositionTypeUiField() != RIG_FORMATION_NAMES )
    {
        caf::PdmUiGroup * timeLapseGr = uiOrdering.addNewGroup("Relative Result Options");
        timeLapseGr->add(&m_isTimeLapseResultUiField);
        if ( m_isTimeLapseResultUiField() )
            timeLapseGr->add(&m_timeLapseBaseTimestepUiField);
    }

    if (m_resultPositionTypeUiField() == RIG_NODAL)
    {
        caf::PdmUiGroup * compactionGroup = uiOrdering.addNewGroup("Compaction Options");
        compactionGroup->add(&m_compactionRefLayerUiField);

        if (m_compactionRefLayerUiField == (int)RigFemResultAddress::NO_COMPACTION)
        {
            if (m_geomCase &&  m_geomCase->geoMechData() )
            {
                m_compactionRefLayerUiField = (int)m_geomCase->geoMechData()->femParts()->part(0)->structGrid()->reservoirIJKBoundingBox().first.z();
            }
        }
    }

    if (!m_isChangedByField)
    {
        m_resultPositionTypeUiField = m_resultPositionType;
        m_resultVariableUiField = composeFieldCompString(m_resultFieldName(), m_resultComponentName());
    }

    m_isChangedByField = false;

    uiOrdering.skipRemainingFields(true);
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
        if (&m_resultPositionTypeUiField == fieldNeedingOptions)
        {
            std::vector<RigFemResultPosEnum> optionItems = { RIG_NODAL, RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT, RIG_ELEMENT_NODAL_FACE, RIG_FORMATION_NAMES, RIG_ELEMENT };
            if (m_addWellPathDerivedResults)
            {
                optionItems.push_back(RIG_WELLPATH_DERIVED);
            }
            for (RigFemResultPosEnum value : optionItems)
            {
                options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RigFemResultPosEnum>::uiText(value), QVariant(value)));
            }
        }
        else if (&m_resultVariableUiField == fieldNeedingOptions)
        {
            std::map<std::string, std::vector<std::string> >  fieldCompNames = getResultMetaDataForUIFieldSetting();

            QStringList uiVarNames;
            QStringList varNames;
            bool isNeedingTimeLapseStrings =  m_isTimeLapseResultUiField() && (m_resultPositionTypeUiField() != RIG_FORMATION_NAMES);

            getUiAndResultVariableStringList(&uiVarNames, &varNames, fieldCompNames, isNeedingTimeLapseStrings, m_timeLapseBaseTimestepUiField);

            for (int oIdx = 0; oIdx < uiVarNames.size(); ++oIdx)
            {
                options.push_back(caf::PdmOptionItemInfo(uiVarNames[oIdx], varNames[oIdx]));
            }
        }
        else if (&m_isTimeLapseResultUiField == fieldNeedingOptions)
        {
            //options.push_back(caf::PdmOptionItemInfo("Absolute", false));
            //options.push_back(caf::PdmOptionItemInfo("Time Lapse", true));
        }
        else if (&m_timeLapseBaseTimestepUiField == fieldNeedingOptions)
        {
            std::vector<std::string> stepNames;
            if(m_geomCase->geoMechData())
            {
                 stepNames = m_geomCase->geoMechData()->femPartResults()->filteredStepNames();
            }

            for (size_t stepIdx = 0; stepIdx < stepNames.size(); ++stepIdx)
            {
                options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(stepNames[stepIdx]), static_cast<int>(stepIdx)));
            }
        }
        else if (&m_compactionRefLayerUiField == fieldNeedingOptions)
        {
            if (m_geomCase->geoMechData())
            {
                size_t kCount = m_geomCase->geoMechData()->femParts()->part(0)->structGrid()->gridPointCountK() - 1;
                for ( size_t layerIdx = 0; layerIdx < kCount; ++layerIdx )
                {
                    options.push_back(caf::PdmOptionItemInfo(QString::number(layerIdx + 1), (int)layerIdx));
                }
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
    m_isChangedByField = true;

    if (&m_resultPositionTypeUiField == changedField)
    {
        if (m_resultPositionTypeUiField() == RIG_WELLPATH_DERIVED)
        {
            m_isTimeLapseResultUiField = false;
            m_isTimeLapseResultUiField.uiCapability()->setUiReadOnly(true);
            m_timeLapseBaseTimestepUiField.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_isTimeLapseResultUiField.uiCapability()->setUiReadOnly(false);
            m_timeLapseBaseTimestepUiField.uiCapability()->setUiReadOnly(false);
        }
    }

    if (&m_isTimeLapseResultUiField == changedField)
    {
        m_isTimeLapseResult = m_isTimeLapseResultUiField;
        if (m_isTimeLapseResult())
        {
            if (m_timeLapseBaseTimestep() == RigFemResultAddress::NO_TIME_LAPSE)
            {
                m_timeLapseBaseTimestep = 0;
                m_timeLapseBaseTimestepUiField = 0;
            }
        }
    }

    if(   &m_resultPositionTypeUiField == changedField 
       || &m_isTimeLapseResultUiField == changedField
       || &m_timeLapseBaseTimestepUiField == changedField)
    {
        std::map<std::string, std::vector<std::string> >  fieldCompNames = getResultMetaDataForUIFieldSetting();
        QStringList uiVarNames;
        QStringList varNames;
        bool isNeedingTimeLapseStrings =  m_isTimeLapseResultUiField() && (m_resultPositionTypeUiField() != RIG_FORMATION_NAMES);
        getUiAndResultVariableStringList(&uiVarNames, &varNames, fieldCompNames, isNeedingTimeLapseStrings, m_timeLapseBaseTimestepUiField);

        if (m_resultPositionTypeUiField() == m_resultPositionType()
            && m_isTimeLapseResultUiField() == m_isTimeLapseResult()
            && m_timeLapseBaseTimestepUiField() == m_timeLapseBaseTimestep()
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
    RimGridView* view = nullptr;
    this->firstAncestorOrThisOfType(view);
    RimPlotCurve* curve = nullptr;
    this->firstAncestorOrThisOfType(curve);
    Rim3dWellLogCurve* rim3dWellLogCurve = nullptr;
    this->firstAncestorOrThisOfType(rim3dWellLogCurve);


    if (&m_resultVariableUiField == changedField || &m_compactionRefLayerUiField == changedField)
    {
        QStringList fieldComponentNames = m_resultVariableUiField().split(QRegExp("\\s+"));
        if (fieldComponentNames.size() > 0)
        {
            m_resultPositionType = m_resultPositionTypeUiField;
            if (m_resultPositionType() == RIG_FORMATION_NAMES)
            {
                // Complete string of selected formation is stored in m_resultFieldName
                m_resultFieldName = m_resultVariableUiField();
                m_resultComponentName = "";
                m_isTimeLapseResult = false;
                m_timeLapseBaseTimestep = 0;
            }
            else
            {
                m_resultFieldName = fieldComponentNames[0];
                if (fieldComponentNames.size() > 1)
                {
                    m_resultComponentName = fieldComponentNames[1];
                }
                else
                {
                    m_resultComponentName = "";
                }

                m_isTimeLapseResult     = m_isTimeLapseResultUiField();
                m_timeLapseBaseTimestep = m_timeLapseBaseTimestepUiField();
                m_compactionRefLayer    = m_compactionRefLayerUiField();
            }

            if (m_geomCase->geoMechData() && m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress()))
            {
                if (view) view->hasUserRequestedAnimation = true;
            }
            
            if (propFilter)
            {
                propFilter->setToDefaultValues();

                if (view) view->scheduleGeometryRegen(PROPERTY_FILTERED);
            }

            if ( view )
            {
                view->scheduleCreateDisplayModelAndRedraw();
                view->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }

            if (dynamic_cast<RimGeoMechCellColors*>(this))
            {
                this->updateLegendCategorySettings();

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
                curve->loadDataAndUpdate(true);
            }

            if (rim3dWellLogCurve)
            {
                rim3dWellLogCurve->updateCurveIn3dView();
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

    if (rim3dWellLogCurve)
    {        
        rim3dWellLogCurve->resetMinMaxValuesAndUpdateUI();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RimGeoMechResultDefinition::getResultMetaDataForUIFieldSetting()
{
    RimGeoMechCase* gmCase = m_geomCase;
    std::map<std::string, std::vector<std::string> > fieldWithComponentNames;
    if (gmCase && gmCase->geoMechData())
    {
        fieldWithComponentNames = gmCase->geoMechData()->femPartResults()->scalarFieldAndComponentNames(m_resultPositionTypeUiField());
    }

    return fieldWithComponentNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::getUiAndResultVariableStringList(QStringList* uiNames, 
                                                                  QStringList* variableNames, 
                                                                  const std::map<std::string, std::vector<std::string> >&  fieldCompNames,
                                                                  bool isTimeLapseResultList,
                                                                  int baseFrameIdx)
{
    CVF_ASSERT(uiNames && variableNames);

    std::map<std::string, std::vector<std::string> >::const_iterator fieldIt;
    for (fieldIt = fieldCompNames.begin(); fieldIt != fieldCompNames.end(); ++fieldIt)
    {
        QString resultFieldName = QString::fromStdString(fieldIt->first);

        if (resultFieldName == "E" || resultFieldName == "S" || resultFieldName == "POR") continue; // We will not show the native POR, Stress and Strain

        QString resultFieldUiName = convertToUiResultFieldName(resultFieldName, isTimeLapseResultList, baseFrameIdx);

        uiNames->push_back(resultFieldUiName);
        variableNames->push_back(resultFieldName);

        std::vector<std::string>::const_iterator compIt;
        for (compIt = fieldIt->second.begin(); compIt != fieldIt->second.end(); ++compIt)
        {
            QString resultCompName = QString::fromStdString(*compIt);
            uiNames->push_back("   " + convertToUIComponentName(resultCompName, isTimeLapseResultList, baseFrameIdx));
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
    m_isTimeLapseResultUiField = m_isTimeLapseResult;
    m_timeLapseBaseTimestepUiField = m_timeLapseBaseTimestep;
    m_isTimeLapseResultUiField.uiCapability()->setUiReadOnly(resultPositionType() == RIG_WELLPATH_DERIVED);    
    m_timeLapseBaseTimestepUiField.uiCapability()->setUiReadOnly(resultPositionType() == RIG_WELLPATH_DERIVED);
    m_compactionRefLayerUiField = m_compactionRefLayer;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::loadResult()
{
    if (m_geomCase && m_geomCase->geoMechData())
    {
        if (this->resultAddress().fieldName == "FractureGradient" ||
            this->resultAddress().fieldName == "ShearFailureGradient")
        {
            RigFemResultAddress stressResAddr(RIG_ELEMENT_NODAL, std::string("ST"), "");
            RigFemResultAddress porBarResAddr(RIG_ELEMENT_NODAL, std::string("POR-Bar"), "");
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(stressResAddr);
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(porBarResAddr);
        }
        else
        {
            m_geomCase->geoMechData()->femPartResults()->assertResultsLoaded(this->resultAddress());
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultDefinition::setAddWellPathDerivedResults(bool addWellPathDerivedResults)
{
    m_addWellPathDerivedResults = addWellPathDerivedResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimGeoMechResultDefinition::resultAddress() const
{
    return RigFemResultAddress(resultPositionType(),
                               resultFieldName().toStdString(), 
                               resultComponentName().toStdString(),
                               m_isTimeLapseResult() ? m_timeLapseBaseTimestep() : RigFemResultAddress::NO_TIME_LAPSE,
                               resultFieldName().toStdString() == RigFemPartResultsCollection::FIELD_NAME_COMPACTION ? m_compactionRefLayer() : RigFemResultAddress::NO_COMPACTION);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RimGeoMechResultDefinition::observedResults() const
{
    return std::vector<RigFemResultAddress>(1, resultAddress());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultPosEnum RimGeoMechResultDefinition::resultPositionType() const
{
    return m_resultPositionType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultFieldName() const
{
    return m_resultFieldName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultComponentName() const
{
    return m_resultComponentName();
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
    RigGeoMechCaseData* caseData = ownerCaseData();
    if (caseData)
    {
        RigFemPartResultsCollection* results = caseData->femPartResults();
        if (results)
        {
            return results->assertResultsLoaded(this->resultAddress());
        }
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultFieldUiName()
{
    return convertToUiResultFieldName(m_resultFieldName(), m_isTimeLapseResult, m_timeLapseBaseTimestep);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::resultComponentUiName()
{
    return convertToUIComponentName(m_resultComponentName(), m_isTimeLapseResult, m_timeLapseBaseTimestep);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUiResultFieldName(QString resultFieldName,
                                                               bool isTimeLapseResultList,
                                                               int baseFrameIdx)
{
    QString newName (resultFieldName);

    if (resultFieldName == "E") newName = "NativeAbaqus Strain";
    if (resultFieldName == "S") newName =  "NativeAbaqus Stress";
    if (resultFieldName == "NE") newName =  "E"; // Make NE and NS appear as E and SE
    if (resultFieldName == "POR-Bar") newName =  "POR"; // POR-Bar appear as POR
    if (resultFieldName == "MODULUS") newName = "Young's Modulus";
    if (resultFieldName == "RATIO") newName = "Poisson's Ratio";

    if (isTimeLapseResultList) newName += "_D" + QString::number(baseFrameIdx);

    return newName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGeoMechResultDefinition::convertToUIComponentName(QString resultComponentName,
                                                             bool isTimeLapseResultList,
                                                             int baseFrameIdx)
{
    if(isTimeLapseResultList) resultComponentName += "_D" + QString::number(baseFrameIdx);

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

    m_isTimeLapseResult     = resultAddress.isTimeLapse();
    m_isTimeLapseResultUiField = m_isTimeLapseResult;

    if (m_isTimeLapseResult)
    {
        m_timeLapseBaseTimestep = resultAddress.timeLapseBaseFrameIdx;
    }
    m_timeLapseBaseTimestepUiField = m_timeLapseBaseTimestep;

    m_compactionRefLayer = resultAddress.refKLayerIndex;
    m_compactionRefLayerUiField = m_compactionRefLayer;
}
