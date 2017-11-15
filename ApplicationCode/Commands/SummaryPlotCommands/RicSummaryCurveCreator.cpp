/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSummaryCurveCreator.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveDefinition.h"

#include "RicSelectSummaryPlotUI.h"
#include "RiuSummaryCurveDefinitionKeywords.h"

#include "RifReaderEclipseSummary.h"

#include "RimMainPlotCollection.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCalculationCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryCurveDefSelection.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QInputDialog>

#include <algorithm>
#include <sstream>


CAF_PDM_SOURCE_INIT(RicSummaryCurveCreator, "RicSummaryCurveCreator");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::RicSummaryCurveCreator()
{
    CAF_PDM_InitFieldNoDefault(&m_targetPlot, "TargetPlot", "Target Plot", "", "", "");

    CAF_PDM_InitField(&m_useAutoAppearanceAssignment, "UseAutoAppearanceAssignment", true, "Auto", "", "", "");
    CAF_PDM_InitField(&m_appearanceApplyButton, "AppearanceApplyButton", false, "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_caseAppearanceType, "CaseAppearanceType", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variableAppearanceType, "VariableAppearanceType", "Vector", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellAppearanceType, "WellAppearanceType", "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupAppearanceType, "GroupAppearanceType", "Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionAppearanceType, "RegionAppearanceType", "Region", "", "", "");

    m_previewPlot.reset(new RimSummaryPlot());
    m_previewPlot->setShowDescription(false);

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "", "", "", "");
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_closeButtonField, "Close", "", "", "", "");
    m_closeButtonField = false;
    m_closeButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_closeButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_okButtonField, "OK", "", "", "", "");
    m_okButtonField = false;
    m_okButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_okButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_appearanceApplyButton = false;
    m_appearanceApplyButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_appearanceApplyButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LEFT);

    CAF_PDM_InitFieldNoDefault(&m_curveNameConfig, "SummaryCurveNameConfig", "SummaryCurveNameConfig", "", "", "");
    m_curveNameConfig = new RimSummaryCurveAutoName();
    m_curveNameConfig.uiCapability()->setUiHidden(true);
    m_curveNameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    m_summaryCurveSelectionEditor.reset(new RiuSummaryCurveDefSelectionEditor());

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setFieldChangedHandler([this]() { this->selectionEditorFieldChanged(); });
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setMultiSelectionMode(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::~RicSummaryCurveCreator()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryCurveCreator::previewPlot() const
{
    return m_previewPlot.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateFromSummaryPlot(RimSummaryPlot* targetPlot)
{
    if (targetPlot == nullptr || m_targetPlot != targetPlot)
    {
        resetAllFields();
    }
    
    m_targetPlot = targetPlot;
    m_useAutoAppearanceAssignment = true;

    if (m_targetPlot)
    {
        populateCurveCreator(*m_targetPlot);
    }

    syncPreviewCurvesFromUiSelection();

    caf::PdmUiItem::updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryCurveCreator::addressSelectionWidget(QWidget* parent)
{
    return m_summaryCurveSelectionEditor->getOrCreateWidget(parent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::isCloseButtonPressed() const
{
    return m_closeButtonField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::clearCloseButton()
{
    m_closeButtonField = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_applyButtonField || changedField == &m_okButtonField)
    {
        if (m_targetPlot == nullptr)
        {
            createNewPlot();
        }

        updateTargetPlot();

        if (changedField == &m_okButtonField)
        {
            m_closeButtonField = true;
            RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();
            mainPlotWindow->selectAsCurrentItem(m_targetPlot);
            mainPlotWindow->setExpanded(m_targetPlot);
        }

        m_applyButtonField = false;
        m_okButtonField = false;

        caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>(m_targetPlot->uiCapability()->objectToggleField());
        field->setValueWithFieldChanged(true);
    }
    else if (changedField == &m_useAutoAppearanceAssignment && m_useAutoAppearanceAssignment)
    {
        updateAppearanceEditor();
    }
    else if (changedField == &m_appearanceApplyButton)
    {
        applyAppearanceToAllPreviewCurves();
        m_previewPlot->loadDataAndUpdate();
        m_appearanceApplyButton = false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSummaryCurveCreator::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_targetPlot)
    {
        RimProject* proj = RiaApplication::instance()->project();
        
        RimSummaryPlotCollection* summaryPlotColl = proj->mainPlotCollection()->summaryPlotCollection();

        // Create New Plot item
        QString displayName = "( New Plot )";
        options.push_back(caf::PdmOptionItemInfo(displayName, nullptr));

        if (summaryPlotColl)
        {
            summaryPlotColl->summaryPlotItemInfos(&options);
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    // Appearance settings
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroupWithKeyword("Curve Appearance Assignment", RiuSummaryCurveDefinitionKeywords::appearance());
    caf::PdmUiGroup* appearanceSubGroup = appearanceGroup->addNewGroup("Appearance Type Assignment");
    appearanceGroup->setCollapsedByDefault(true);
    appearanceSubGroup->add(&m_useAutoAppearanceAssignment);
    appearanceSubGroup->add(&m_caseAppearanceType);
    appearanceSubGroup->add(&m_variableAppearanceType);
    appearanceSubGroup->add(&m_wellAppearanceType);
    appearanceSubGroup->add(&m_groupAppearanceType);
    appearanceSubGroup->add(&m_regionAppearanceType);
    appearanceGroup->add(&m_appearanceApplyButton);

    // Appearance option sensitivity
    {
        m_caseAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_variableAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_wellAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_groupAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_regionAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
    }

    // Name config
    caf::PdmUiGroup* autoNameGroup = uiOrdering.addNewGroupWithKeyword("Curve Name Configuration", RiuSummaryCurveDefinitionKeywords::nameConfig());
    autoNameGroup->setCollapsedByDefault(true);
    m_curveNameConfig->uiOrdering(uiConfigName, *autoNameGroup);

    // Fields to be displayed directly in UI
    uiOrdering.add(&m_targetPlot);
    uiOrdering.add(&m_okButtonField);
    uiOrdering.add(&m_applyButtonField);
    uiOrdering.add(&m_closeButtonField);

    uiOrdering.skipRemainingFields(true);

    syncPreviewCurvesFromUiSelection();

    m_summaryCurveSelectionEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::syncPreviewCurvesFromUiSelection()
{
    std::vector<RiaSummaryCurveDefinition> allCurveDefinitionsVector = m_summaryCurveSelectionEditor->summaryAddressSelection()->selectedCurveDefinitions();
    std::set<RiaSummaryCurveDefinition> allCurveDefinitions = std::set<RiaSummaryCurveDefinition>(allCurveDefinitionsVector.begin(), allCurveDefinitionsVector.end());

    std::vector<RimSummaryCurve*> currentCurvesInPreviewPlot = m_previewPlot->summaryCurves();
    if (allCurveDefinitions.size() != currentCurvesInPreviewPlot.size())
    {
        std::set<RiaSummaryCurveDefinition> currentCurveDefs;
        std::set<RiaSummaryCurveDefinition> newCurveDefs;
        std::set<RimSummaryCurve*> curvesToDelete;

        for (const auto& curve : currentCurvesInPreviewPlot)
        {
            currentCurveDefs.insert(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));
        }
        
        if (allCurveDefinitions.size() < currentCurvesInPreviewPlot.size())
        {
            // Determine which curves to delete from plot
            std::set<RiaSummaryCurveDefinition> deleteCurveDefs;
            std::set_difference(currentCurveDefs.begin(), currentCurveDefs.end(),
                                allCurveDefinitions.begin(), allCurveDefinitions.end(),
                                std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

            for (const auto& curve : currentCurvesInPreviewPlot)
            {
                RiaSummaryCurveDefinition curveDef = RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY());
                if (deleteCurveDefs.count(curveDef) > 0)
                    curvesToDelete.insert(curve);
            }
        }
        else
        {
            // Determine which curves are new since last time
            std::set_difference(allCurveDefinitions.begin(), allCurveDefinitions.end(),
                                currentCurveDefs.begin(), currentCurveDefs.end(),
                                std::inserter(newCurveDefs, newCurveDefs.end()));
        }

        updatePreviewCurvesFromCurveDefinitions(allCurveDefinitions, newCurveDefs, curvesToDelete);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updatePreviewCurvesFromCurveDefinitions(const std::set<RiaSummaryCurveDefinition>& allCurveDefsToDisplay, 
                                                                     const std::set<RiaSummaryCurveDefinition>& curveDefsToAdd,
                                                                     const std::set<RimSummaryCurve*>& curvesToDelete)
{
    RimSummaryCase* prevCase = nullptr;
    RimSummaryCurveAppearanceCalculator curveLookCalc(allCurveDefsToDisplay, getAllSummaryCaseNames(), getAllSummaryWellNames());

    initCurveAppearanceCalculator(curveLookCalc);

    // Delete curves
    for (const auto& curve : curvesToDelete)
    {
        m_previewPlot->deleteCurve(curve);
    }

    // Add new curves
    for (const auto& curveDef : curveDefsToAdd)
    {
        RimSummaryCase* currentCase = curveDef.summaryCase();
        RimSummaryCurve* curve = new RimSummaryCurve();
        curve->setSummaryCaseY(currentCase);
        curve->setSummaryAddressY(curveDef.summaryAddress());
        curve->applyCurveAutoNameSettings(*m_curveNameConfig());
        m_previewPlot->addCurveNoUpdate(curve);
        curveLookCalc.setupCurveLook(curve);
    }

    m_previewPlot->loadDataAndUpdate();
    m_previewPlot->zoomAll();
    m_previewPlot->updateConnectedEditors();
    m_previewPlot->summaryCurveCollection()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::string> RicSummaryCurveCreator::getAllSummaryCaseNames()
{
    std::set<std::string> summaryCaseHashes;
    RimProject* proj = RiaApplication::instance()->project();
    std::vector<RimSummaryCase*> cases;

    proj->allSummaryCases(cases);

    for (RimSummaryCase* rimCase : cases)
    {
        summaryCaseHashes.insert(rimCase->summaryHeaderFilename().toUtf8().constData());
    }

    return summaryCaseHashes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::string> RicSummaryCurveCreator::getAllSummaryWellNames()
{
    std::set<std::string> summaryWellNames;
    RimProject* proj = RiaApplication::instance()->project();
    std::vector<RimSummaryCase*> cases;

    proj->allSummaryCases(cases);
    for (RimSummaryCase* rimCase : cases)
    {
        RifSummaryReaderInterface* reader = nullptr;
        if (rimCase)
        {
            reader = rimCase->summaryReader();
        }

        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

            for (size_t i = 0; i < allAddresses.size(); i++)
            {
                if (allAddresses[i].category() == RifEclipseSummaryAddress::SUMMARY_WELL)
                {
                    summaryWellNames.insert(allAddresses[i].wellName());
                }
            }
        }
    }
    return summaryWellNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_applyButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if (&m_closeButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Cancel";
        }
    }
    else if (&m_okButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "OK";
        }
    }
    else if (&m_appearanceApplyButton == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if (&m_targetPlot == field)
    {
        caf::PdmUiComboBoxEditorAttribute* attrib = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->adjustWidthToContents = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Populate curve creator from the given curve collection
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::populateCurveCreator(const RimSummaryPlot& sourceSummaryPlot)
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;

    m_previewPlot->deleteAllSummaryCurves();
    for (const auto& curve : sourceSummaryPlot.summaryCurves())
    {
        bool isObservedDataCase = isObservedData(curve->summaryCaseY());

        curveDefs.push_back(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));

        // Copy curve object to the preview plot
        copyCurveAndAddToPlot(curve, m_previewPlot.get(), true);
    }

    // Set visibility for imported curves which were not checked in source plot
    std::set <std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> sourceCurveDefs;
    for (const auto& curve : sourceSummaryPlot.summaryCurves())
    {
        sourceCurveDefs.insert(std::make_pair(curve->summaryCaseY(), curve->summaryAddressY()));
    }

    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        auto curveDef = std::make_pair(curve->summaryCaseY(), curve->summaryAddressY());
        if (sourceCurveDefs.count(curveDef) == 0)
            curve->setCurveVisiblity(false);
    }

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setSelectedCurveDefinitions(curveDefs);

    updateAppearanceEditor();
}

//--------------------------------------------------------------------------------------------------
/// Copy curves from preview plot to target plot
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateTargetPlot()
{
    if (m_targetPlot == nullptr)  m_targetPlot = new RimSummaryPlot();

    m_targetPlot->deleteAllSummaryCurves();

    // Add edited curves to target plot
    for (const auto& editedCurve : m_previewPlot->summaryCurves())
    {
        if (!editedCurve->isCurveVisible())
        {
            continue;
        }
        copyCurveAndAddToPlot(editedCurve, m_targetPlot);
    }

    m_targetPlot->loadDataAndUpdate();
    m_targetPlot->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::copyCurveAndAddToPlot(const RimSummaryCurve *curve, RimSummaryPlot *plot, bool forceVisible)
{
    RimSummaryCurve* curveCopy = dynamic_cast<RimSummaryCurve*>(curve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(curveCopy);

    if (forceVisible) 
    {
        curveCopy->setCurveVisiblity(true);
    }

    plot->addCurveNoUpdate(curveCopy);

    // Resolve references after object has been inserted into the project data model
    curveCopy->resolveReferencesRecursively();

    // The curve creator is not a descendant of the project, and need to be set manually
    curveCopy->setSummaryCaseY(curve->summaryCaseY());
    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::resetAllFields()
{
    std::vector<RiaSummaryCurveDefinition> curveDefinitions;
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setSelectedCurveDefinitions(curveDefinitions);

    m_previewPlot->deleteAllSummaryCurves();
    m_targetPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::initCurveAppearanceCalculator(RimSummaryCurveAppearanceCalculator& curveAppearanceCalc)
{
    if (!m_useAutoAppearanceAssignment())
    {
        curveAppearanceCalc.assignDimensions(m_caseAppearanceType(),
                                             m_variableAppearanceType(),
                                             m_wellAppearanceType(),
                                             m_groupAppearanceType(),
                                             m_regionAppearanceType());
    }
    else
    {
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType caseAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType variAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType wellAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType gropAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType regiAppearance;

        curveAppearanceCalc.getDimensions(&caseAppearance,
                                          &variAppearance,
                                          &wellAppearance,
                                          &gropAppearance,
                                          &regiAppearance);

        m_caseAppearanceType = caseAppearance;
        m_variableAppearanceType = variAppearance;
        m_wellAppearanceType = wellAppearance;
        m_groupAppearanceType = gropAppearance;
        m_regionAppearanceType = regiAppearance;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::applyAppearanceToAllPreviewCurves()
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs = allPreviewCurveDefs();
    
    RimSummaryCurveAppearanceCalculator curveLookCalc(allCurveDefs, getAllSummaryCaseNames(), getAllSummaryWellNames());
    initCurveAppearanceCalculator(curveLookCalc);

    for (auto& curve : m_previewPlot->summaryCurves())
    {
        curve->resetAppearance();
        curveLookCalc.setupCurveLook(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateAppearanceEditor()
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs = allPreviewCurveDefs();

    RimSummaryCurveAppearanceCalculator curveLookCalc(allCurveDefs, getAllSummaryCaseNames(), getAllSummaryWellNames());
    initCurveAppearanceCalculator(curveLookCalc);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RiaSummaryCurveDefinition> RicSummaryCurveCreator::allPreviewCurveDefs() const
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs;

    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        allCurveDefs.insert(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));
    }
    return allCurveDefs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::createNewPlot()
{
    RimProject* proj = RiaApplication::instance()->project();

    RimSummaryPlotCollection* summaryPlotColl = proj->mainPlotCollection()->summaryPlotCollection();
    if (summaryPlotColl)
    {
        QString summaryPlotName = QString("Summary Plot %1").arg(summaryPlotColl->summaryPlots().size() + 1);

        bool ok = false;
        summaryPlotName = QInputDialog::getText(NULL, "New Summary Plot Name", "New Summary Plot Name", QLineEdit::Normal, summaryPlotName, &ok);
        if (ok)
        {
            RimSummaryPlot* plot = new RimSummaryPlot();
            summaryPlotColl->summaryPlots().push_back(plot);

            plot->setDescription(summaryPlotName);
            plot->loadDataAndUpdate();

            summaryPlotColl->updateConnectedEditors();

            m_targetPlot = plot;
            updateTargetPlot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateCurveNames()
{
    for (RimSummaryCurve* curve : m_previewPlot->summaryCurves())
    {
        curve->applyCurveAutoNameSettings(*m_curveNameConfig());
        curve->updateCurveNameNoLegendUpdate();
    }

   if (m_previewPlot && m_previewPlot->qwtPlot()) m_previewPlot->qwtPlot()->updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::isObservedData(RimSummaryCase *sumCase) const
{
    return dynamic_cast<RimObservedData*>(sumCase) != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RicSummaryCurveCreator::calculatedSummaryCase()
{
    RimSummaryCalculationCollection* calcColl = RiaApplication::instance()->project()->calculationCollection();

    return calcColl->calculationSummaryCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::selectionEditorFieldChanged()
{
    syncPreviewCurvesFromUiSelection();
}
