/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RimTensorResults.h"

#include "RigFemResultAddress.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimLegendConfig.h"

#include "cafAppEnum.h"
#include "cafPdmUiListEditor.h"


CAF_PDM_SOURCE_INIT(RimTensorResults, "RimTensorResults");


namespace caf
{
    template<>
    void AppEnum< RimTensorResults::TensorColors >::setUp()
    {
        addItem(RimTensorResults::WHITE_GRAY_BLACK , "WHITE_GRAY_BLACK", "White, Gray, Black");
        addItem(RimTensorResults::MAGENTA_BROWN_BLACK, "MAGENTA_BROWN_BLACK", "Magenta, Brown, Black");
        addItem(RimTensorResults::RESULT_COLORS, "RESULT_COLORS", "Result Colors");

        setDefault(RimTensorResults::WHITE_GRAY_BLACK);
    }

    template<>
    void AppEnum< RimTensorResults::ScaleMethod >::setUp()
    {
        addItem(RimTensorResults::RESULT, "RESULT", "Result");
        addItem(RimTensorResults::CONSTANT, "CONSTANT", "Constant");

        setDefault(RimTensorResults::RESULT);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTensorResults::RimTensorResults()
{
    CAF_PDM_InitObject("Tensor Results", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();
    legendConfig.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionType, "ResultPositionType", "Result Position", "", "", "");
    m_resultPositionType.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultFieldName, "ResultVariable", "Result Variable", "", "", "");
    m_resultFieldName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultPositionTypeUiField, "ResultPositionTypeUi", "Result Position", "", "", "");
    m_resultPositionTypeUiField.xmlCapability()->setIOWritable(false);
    m_resultPositionTypeUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_resultFieldNameUiField, "ResultVariableUI", QString(""), "Value", "", "", "");
    m_resultFieldNameUiField.xmlCapability()->setIOWritable(false);
    m_resultFieldNameUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_showTensors, "ShowTensors", true, "", "", "", "");

    CAF_PDM_InitField(&m_principal1, "Principal1", true, "Principal 1", "", "", "");
    CAF_PDM_InitField(&m_principal2, "Principal2", true, "Principal 2", "", "", "");
    CAF_PDM_InitField(&m_principal3, "Principal3", true, "Principal 3", "", "", "");

    CAF_PDM_InitField(&m_threshold, "Threshold", 0.0f, "Threshold", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_vectorColor, "VectorColor", "Color", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_scaleMethod, "ScaleMethod", "Scale Method", "", "", "");
    CAF_PDM_InitField(&m_sizeScale, "SizeScale", 1.0f, "Size Scale", "", "", "");

    m_resultFieldNameUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultFieldNameUiField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    m_resultPositionType = RIG_ELEMENT_NODAL;
    m_resultPositionTypeUiField = m_resultPositionType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTensorResults::~RimTensorResults()
{
    delete legendConfig;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimTensorResults::selectedTensorResult() const
{
    return RigFemResultAddress(m_resultPositionType(), m_resultFieldName().toStdString(), "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTensorResults::setShowTensors(bool enableTensors)
{
    m_showTensors = enableTensors;
    updateConnectedEditors();
    updateUiIconFromState(enableTensors);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTensorResults::showTensors() const
{
    return m_showTensors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTensorResults::showPrincipal1() const
{
    return m_principal1();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTensorResults::showPrincipal2() const
{
    return m_principal2();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTensorResults::showPrincipal3() const
{
    return m_principal3();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimTensorResults::threshold() const
{
    return m_threshold();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimTensorResults::sizeScale() const
{
    return m_sizeScale();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTensorResults::TensorColors RimTensorResults::vectorColors() const
{
    return m_vectorColor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTensorResults::ScaleMethod RimTensorResults::scaleMethod() const
{
    return m_scaleMethod();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemResultPosEnum RimTensorResults::resultPositionType() const
{
    return m_resultPositionType();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTensorResults::resultFieldName() const
{
    return m_resultFieldName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RimTensorResults::getResultMetaDataForUIFieldSetting()
{
    std::vector<std::string> fieldNames;
    fieldNames.push_back("SE");
    fieldNames.push_back("ST");
    fieldNames.push_back("E");

    return fieldNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTensorResults::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_resultPositionTypeUiField)
    {
        std::vector<std::string> fieldCompNames = getResultMetaDataForUIFieldSetting();
        if (m_resultPositionTypeUiField() == m_resultPositionType())
        {
            m_resultFieldNameUiField = uiFieldName(m_resultFieldName());
        }
        else
        {
            m_resultFieldNameUiField = "";
        }
    }

    if (changedField == &m_resultFieldNameUiField)
    {
        m_resultPositionType = m_resultPositionTypeUiField;
        m_resultFieldName = fieldNameFromUi(m_resultFieldNameUiField);
    }
    if (changedField == &m_showTensors)
    {
        setShowTensors(m_showTensors);
    }

    RimGeoMechView* view;
    firstAncestorOrThisOfType(view);
    view->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTensorResults::objectToggleField()
{
    return &m_showTensors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimTensorResults::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if ( fieldNeedingOptions == &m_resultPositionTypeUiField)
    {
        using ResultEnum = caf::AppEnum<RigFemResultPosEnum>;

        options.push_back(caf::PdmOptionItemInfo(ResultEnum::uiText(RigFemResultPosEnum::RIG_ELEMENT_NODAL),
                                                RigFemResultPosEnum::RIG_ELEMENT_NODAL));

        options.push_back(caf::PdmOptionItemInfo(ResultEnum::uiText(RigFemResultPosEnum::RIG_INTEGRATION_POINT),
                                                 RigFemResultPosEnum::RIG_INTEGRATION_POINT));
    }
    else if (fieldNeedingOptions == &m_resultFieldNameUiField)
    {
        std::vector<std::string> fieldCompNames = getResultMetaDataForUIFieldSetting();

        for (size_t oIdx = 0; oIdx < fieldCompNames.size(); ++oIdx)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(fieldCompNames[oIdx]), QString::fromStdString(fieldCompNames[oIdx])));
        }

    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTensorResults::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultPositionTypeUiField);
    uiOrdering.add(&m_resultFieldNameUiField);

    caf::PdmUiGroup* visibilityGroup = uiOrdering.addNewGroup("Visibility");
    visibilityGroup->add(&m_principal1);
    visibilityGroup->add(&m_principal2);
    visibilityGroup->add(&m_principal3);
    visibilityGroup->add(&m_threshold);

    caf::PdmUiGroup* vectorColorsGroup = uiOrdering.addNewGroup("Vector Colors");
    vectorColorsGroup->add(&m_vectorColor);

    caf::PdmUiGroup* vectorSizeGroup = uiOrdering.addNewGroup("Vector Size");
    vectorSizeGroup->add(&m_scaleMethod);
    vectorSizeGroup->add(&m_sizeScale);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTensorResults::initAfterRead()
{
    m_resultPositionTypeUiField = m_resultPositionType;
    m_resultFieldNameUiField = uiFieldName(m_resultFieldName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTensorResults::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_resultFieldNameUiField)
    {
        caf::PdmUiListEditorAttribute* listEditAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>(attribute);
        if (listEditAttr)
        {
            listEditAttr->m_heightHint = 50;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTensorResults::uiFieldName(const QString& fieldName)
{
    if (fieldName == "NE")
    {
        return QString("E");
    }

    return fieldName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTensorResults::fieldNameFromUi(const QString& uiFieldName)
{
    if (uiFieldName == "E")
    {
        return QString("NE");
    }

    return uiFieldName;
}
