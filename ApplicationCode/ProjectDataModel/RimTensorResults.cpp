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
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "cafAppEnum.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT(RimTensorResults, "RimTensorResults");

namespace caf
{
template<>
void AppEnum<RimTensorResults::TensorColors>::setUp()
{
    addItem(RimTensorResults::WHITE_GRAY_BLACK, "WHITE_GRAY_BLACK", "White, Gray, Black");
    addItem(RimTensorResults::ORANGE_BLUE_WHITE, "ORANGE_BLUE_WHITE", "Orange, Blue, White");
    addItem(RimTensorResults::MAGENTA_BROWN_GRAY, "MAGENTA_BROWN_GRAY", "Magenta, Brown, Gray");
    addItem(RimTensorResults::RESULT_COLORS, "RESULT_COLORS", "Result Colors");

    setDefault(RimTensorResults::WHITE_GRAY_BLACK);
}

template<>
void AppEnum<RimTensorResults::ScaleMethod>::setUp()
{
    addItem(RimTensorResults::RESULT, "RESULT", "Result");
    addItem(RimTensorResults::CONSTANT, "CONSTANT", "Constant");

    setDefault(RimTensorResults::RESULT);
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTensorResults::RimTensorResults()
{
    CAF_PDM_InitObject("Element Tensor Results", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&arrowColorLegendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->arrowColorLegendConfig = new RimLegendConfig();
    arrowColorLegendConfig.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_resultFieldName, "ResultVariable", QString("ST"), "Value", "", "", "");
    m_resultFieldName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_resultFieldNameUiField, "ResultVariableUI", QString("ST"), "Value", "", "", "");
    m_resultFieldNameUiField.xmlCapability()->setIOWritable(false);
    m_resultFieldNameUiField.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitField(&m_showTensors, "ShowTensors", false, "", "", "", "");

    CAF_PDM_InitField(&m_principal1, "Principal1", true, "Principal 1", "", "", "");
    CAF_PDM_InitField(&m_principal2, "Principal2", true, "Principal 2", "", "", "");
    CAF_PDM_InitField(&m_principal3, "Principal3", true, "Principal 3", "", "", "");

    CAF_PDM_InitField(&m_threshold, "Threshold", 0.0f, "Threshold", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_vectorColor, "VectorColor", "Color", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_scaleMethod, "ScaleMethod", "Scale Method", "", "", "");
    CAF_PDM_InitField(&m_sizeScale, "SizeScale", 1.0f, "Size Scale", "", "", "");
    CAF_PDM_InitField(&m_rangeMode,
                      "RangeType",
                      RimLegendConfig::RangeModeEnum(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS),
                      "Range Type",
                      "",
                      "Switches between automatic and user defined range",
                      "");

    m_resultFieldNameUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_resultFieldNameUiField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTensorResults::~RimTensorResults() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimTensorResults::selectedTensorResult() const
{
    return RigFemResultAddress(resultPositionType(), m_resultFieldName().toStdString(), "");
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
void RimTensorResults::mappingRange(double* min, double* max) const
{
    *min = cvf::UNDEFINED_DOUBLE;
    *max = cvf::UNDEFINED_DOUBLE;

    if (scaleMethod() == RESULT)
    {
        Rim3dView* view = nullptr;
        firstAncestorOrThisOfType(view);

        int currentTimeStep = view->currentTimeStep();

        RimGeoMechView*              geoMechView      = dynamic_cast<RimGeoMechView*>(view);
        RigFemPartResultsCollection* resultCollection = geoMechView->geoMechCase()->geoMechData()->femPartResults();
        if (!resultCollection) return;

        if (m_rangeMode == RimLegendConfig::AUTOMATIC_ALLTIMESTEPS)
        {
            resultCollection->minMaxScalarValuesOverAllTensorComponents(selectedTensorResult(), min, max);
        }
        else if (m_rangeMode == RimLegendConfig::AUTOMATIC_CURRENT_TIMESTEP)
        {
            resultCollection->minMaxScalarValuesOverAllTensorComponents(selectedTensorResult(), currentTimeStep, min, max);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultPosEnum RimTensorResults::resultPositionType()
{
    return RIG_INTEGRATION_POINT;
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
void RimTensorResults::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue)
{
    if (changedField == &m_resultFieldNameUiField)
    {
        m_resultFieldName = fieldNameFromUi(m_resultFieldNameUiField);
    }
    else if (changedField == &m_showTensors)
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
QList<caf::PdmOptionItemInfo> RimTensorResults::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (fieldNeedingOptions == &m_resultFieldNameUiField)
    {
        std::vector<std::string> fieldCompNames = getResultMetaDataForUIFieldSetting();

        for (size_t oIdx = 0; oIdx < fieldCompNames.size(); ++oIdx)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::fromStdString(fieldCompNames[oIdx]),
                                                     QString::fromStdString(fieldCompNames[oIdx])));
        }
    }
    else if (fieldNeedingOptions == &m_rangeMode)
    {
        options.push_back(caf::PdmOptionItemInfo(RimLegendConfig::RangeModeEnum::uiText(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS),
                                                 RimLegendConfig::AUTOMATIC_ALLTIMESTEPS));
        options.push_back(
            caf::PdmOptionItemInfo(RimLegendConfig::RangeModeEnum::uiText(RimLegendConfig::AUTOMATIC_CURRENT_TIMESTEP),
                                   RimLegendConfig::AUTOMATIC_CURRENT_TIMESTEP));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTensorResults::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultFieldNameUiField);

    caf::PdmUiGroup* visibilityGroup = uiOrdering.addNewGroup("Visibility");
    visibilityGroup->add(&m_principal1);
    visibilityGroup->add(&m_principal2);
    visibilityGroup->add(&m_principal3);
    visibilityGroup->add(&m_threshold);

    caf::PdmUiGroup* vectorColorsGroup = uiOrdering.addNewGroup("Vector Colors");
    vectorColorsGroup->add(&m_vectorColor);

    caf::PdmUiGroup* vectorSizeGroup = uiOrdering.addNewGroup("Vector Size");
    vectorSizeGroup->add(&m_sizeScale);
    vectorSizeGroup->add(&m_scaleMethod);

    if (m_scaleMethod == RESULT)
    {
        vectorSizeGroup->add(&m_rangeMode);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTensorResults::initAfterRead()
{
    m_resultFieldNameUiField = uiFieldName(m_resultFieldName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTensorResults::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute)
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTensorResults::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    if (m_vectorColor() != RESULT_COLORS)
    {
        uiTreeOrdering.skipRemainingChildren();
    }
}
