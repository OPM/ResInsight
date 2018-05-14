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
#include "RiaColorTables.h"
#include "RiaSummaryCurveDefinition.h"

#include "RicSelectSummaryPlotUI.h"
#include "RiuSummaryCurveDefinitionKeywords.h"

#include "RifReaderEclipseSummary.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimMainPlotCollection.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryCurveDefSelection.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QInputDialog>

#include <algorithm>
#include <sstream>

CAF_PDM_SOURCE_INIT(RicSummaryCurveCreator, "RicSummaryCurveCreator");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RicSummaryCurveCreator::CONFIGURATION_NAME = "CurveCreatorCfg";

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

    CAF_PDM_InitFieldNoDefault(&m_useAutoPlotTitleProxy, "UseAutoPlotTitle", "Auto Plot Title", "", "", "");
    m_useAutoPlotTitleProxy.registerGetMethod(this, &RicSummaryCurveCreator::proxyPlotAutoTitle);
    m_useAutoPlotTitleProxy.registerSetMethod(this, &RicSummaryCurveCreator::proxyEnablePlotAutoTitle);

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

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setFieldChangedHandler(
        [this]() { this->selectionEditorFieldChanged(); });
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setMultiSelectionMode(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::~RicSummaryCurveCreator() {}

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
void RicSummaryCurveCreator::updateFromSummaryPlot(RimSummaryPlot* targetPlot, const std::vector<RimSummaryCase*>& defaultCases)
{
    if (targetPlot == nullptr || m_targetPlot != targetPlot)
    {
        resetAllFields();
    }

    m_targetPlot                  = targetPlot;
    m_useAutoAppearanceAssignment = true;

    if (m_targetPlot)
    {
        populateCurveCreator(*m_targetPlot);
        syncPreviewCurvesFromUiSelection();
        setInitialCurveVisibility(targetPlot);
        m_previewPlot->loadDataAndUpdate();
    }
    else
    {
        setDefaultCurveSelection(defaultCases);
        m_previewPlot->enableAutoPlotTitle(true);
        syncPreviewCurvesFromUiSelection();
    }

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
void RicSummaryCurveCreator::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
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

            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem(m_targetPlot);
            RiuPlotMainWindowTools::setExpanded(m_targetPlot);
        }

        m_applyButtonField = false;
        m_okButtonField    = false;

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
    else if (changedField == &m_useAutoPlotTitleProxy)
    {
        m_previewPlot->updatePlotTitle();

        m_previewPlot->summaryCurveCollection()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSummaryCurveCreator::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                            bool*                      useOptionsOnly)
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
    caf::PdmUiGroup* appearanceGroup =
        uiOrdering.addNewGroupWithKeyword("Curve Appearance Assignment", RiuSummaryCurveDefinitionKeywords::appearance());

    caf::PdmUiGroup* appearanceSubGroup = appearanceGroup->addNewGroup("Appearance Type Assignment");
    appearanceSubGroup->setCollapsedByDefault(true);

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
    caf::PdmUiGroup* autoNameGroup =
        uiOrdering.addNewGroupWithKeyword("Plot and Curve Name Configuration", RiuSummaryCurveDefinitionKeywords::nameConfig());
    autoNameGroup->setCollapsedByDefault(true);

    autoNameGroup->add(&m_useAutoPlotTitleProxy);

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
    std::vector<RiaSummaryCurveDefinition> allCurveDefinitionsVector =
        m_summaryCurveSelectionEditor->summaryAddressSelection()->selectedCurveDefinitions();
    std::set<RiaSummaryCurveDefinition> allCurveDefinitions =
        std::set<RiaSummaryCurveDefinition>(allCurveDefinitionsVector.begin(), allCurveDefinitionsVector.end());

    std::vector<RimSummaryCurve*> currentCurvesInPreviewPlot = m_previewPlot->summaryAndEnsembleCurves();
    if (allCurveDefinitions.size() != currentCurvesInPreviewPlot.size())
    {
        std::set<RiaSummaryCurveDefinition> currentCurveDefs;
        std::set<RiaSummaryCurveDefinition> newCurveDefs;
        std::set<RimSummaryCurve*>          curvesToDelete;

        for (const auto& curve : currentCurvesInPreviewPlot)
        {
            currentCurveDefs.insert(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));
        }

        if (allCurveDefinitions.size() < currentCurvesInPreviewPlot.size())
        {
            // Determine which curves to delete from plot
            std::set<RiaSummaryCurveDefinition> deleteCurveDefs;
            std::set_difference(currentCurveDefs.begin(),
                                currentCurveDefs.end(),
                                allCurveDefinitions.begin(),
                                allCurveDefinitions.end(),
                                std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

            for (const auto& curve : currentCurvesInPreviewPlot)
            {
                RiaSummaryCurveDefinition curveDef = RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY());
                if (deleteCurveDefs.count(curveDef) > 0) curvesToDelete.insert(curve);
            }
        }
        else
        {
            // Determine which curves are new since last time
            std::set_difference(allCurveDefinitions.begin(),
                                allCurveDefinitions.end(),
                                currentCurveDefs.begin(),
                                currentCurveDefs.end(),
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
                                                                     const std::set<RimSummaryCurve*>&          curvesToDelete)
{
    RimSummaryCase*                     prevCase = nullptr;
    std::set<RiaSummaryCurveDefinition> summaryCurveDefsToDisplay;

    // Ignore curve sets when assigning colors to singe summary curves
    for (const auto& def : allCurveDefsToDisplay)
    {
        if (!def.isEnsembleCurve()) summaryCurveDefsToDisplay.insert(def);
    }

    RimSummaryCurveAppearanceCalculator curveLookCalc(summaryCurveDefsToDisplay, getAllSummaryCaseNames(), getAllSummaryWellNames());
    initCurveAppearanceCalculator(curveLookCalc);

    // Delete curves
    for (const auto& curve : curvesToDelete)
    {
        m_previewPlot->deleteCurve(curve);
    }

    // Add new curves
    for (const auto& curveDef : curveDefsToAdd)
    {
        RimSummaryCase*  currentCase = curveDef.summaryCase();
        RimSummaryCurve* curve       = new RimSummaryCurve();
        curve->setSummaryCaseY(currentCase);
        curve->setSummaryAddressY(curveDef.summaryAddress());
        curve->applyCurveAutoNameSettings(*m_curveNameConfig());

        if (curveDef.isEnsembleCurve())
        {
            // Find curveSet
            RimEnsembleCurveSet* curveSet = nullptr;
            for (const auto& cs : m_previewPlot->ensembleCurveSetCollection()->curveSets())
            {
                if (cs->summaryCaseCollection() == curveDef.ensemble() && cs->summaryAddress() == curveDef.summaryAddress())
                {
                    curveSet = cs;
                    break;
                }
            }
            if (!curveSet)
            {
                curveSet = new RimEnsembleCurveSet();
                curveSet->setSummaryCaseCollection(curveDef.ensemble());
                curveSet->setSummaryAddress(curveDef.summaryAddress());
                m_previewPlot->ensembleCurveSetCollection()->addCurveSet(curveSet);

                // Set single curve set color
                size_t colorIndex = m_previewPlot->ensembleCurveSetCollection()->curveSetCount();
                curveSet->setColor(RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(colorIndex));
            }
            curveSet->addCurve(curve);
        }
        else
        {
            m_previewPlot->addCurveNoUpdate(curve);
            curveLookCalc.setupCurveLook(curve);
        }
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
    RimProject*           proj = RiaApplication::instance()->project();

    std::vector<RimSummaryCase*> cases = proj->allSummaryCases();
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
    RimProject*           proj = RiaApplication::instance()->project();

    std::vector<RimSummaryCase*> cases = proj->allSummaryCases();
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
void RicSummaryCurveCreator::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute)
{
    if (&m_applyButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if (&m_closeButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Cancel";
        }
    }
    else if (&m_okButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "OK";
        }
    }
    else if (&m_appearanceApplyButton == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (attrib)
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if (&m_targetPlot == field)
    {
        caf::PdmUiComboBoxEditorAttribute* attrib = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
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
    m_previewPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    for (const auto& curve : sourceSummaryPlot.summaryCurves())
    {
        curveDefs.push_back(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY()));

        // Copy curve object to the preview plot
        copyCurveAndAddToPlot(curve, m_previewPlot.get(), true);
    }

    RimEnsembleCurveSetCollection* previewCurveSetColl = m_previewPlot->ensembleCurveSetCollection();
    for (const auto& curveSet : sourceSummaryPlot.ensembleCurveSetCollection()->curveSets())
    {
        RimEnsembleCurveSet* newCurveSet = curveSet->clone();
        previewCurveSetColl->addCurveSet(newCurveSet);

        RimSummaryCaseCollection* ensemble = curveSet->summaryCaseCollection();
        for (const auto& curve : curveSet->curves())
        {
            curveDefs.push_back(RiaSummaryCurveDefinition(curve->summaryCaseY(), curve->summaryAddressY(), ensemble));

            // Copy curve object to the preview plot
            copyEnsembleCurveAndAddToCurveSet(curve, newCurveSet, true);
        }
    }

    m_previewPlot->copyAxisPropertiesFromOther(sourceSummaryPlot);
    m_previewPlot->enableAutoPlotTitle(sourceSummaryPlot.autoPlotTitle());
    m_previewPlot->updatePlotTitle();
    m_previewPlot->updateAxes();

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setSelectedCurveDefinitions(curveDefs);

    updateAppearanceEditor();
}

//--------------------------------------------------------------------------------------------------
/// Copy curves from preview plot to target plot
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateTargetPlot()
{
    if (m_targetPlot == nullptr) m_targetPlot = new RimSummaryPlot();

    m_targetPlot->deleteAllSummaryCurves();
    m_targetPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    // Add edited curves to target plot
    for (const auto& editedCurve : m_previewPlot->summaryCurves())
    {
        if (!editedCurve->isCurveVisible())
        {
            continue;
        }
        copyCurveAndAddToPlot(editedCurve, m_targetPlot);
    }

    for (const auto& editedCurveSet : m_previewPlot->ensembleCurveSetCollection()->curveSets())
    {
        if (!editedCurveSet->isCurvesVisible())
        {
            continue;
        }

        RimEnsembleCurveSet* newCurveSet = editedCurveSet->clone();
        m_targetPlot->ensembleCurveSetCollection()->addCurveSet(newCurveSet);
        for (const auto& editedCurve : newCurveSet->curves())
        {
            copyEnsembleCurveAndAddToCurveSet(editedCurve, editedCurveSet);
        }

        newCurveSet->setParentQwtPlotNoReplot(m_targetPlot->qwtPlot());
    }

    m_targetPlot->enableAutoPlotTitle(m_useAutoPlotTitleProxy());

    m_targetPlot->loadDataAndUpdate();

    m_targetPlot->updatePlotTitle();
    m_targetPlot->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::copyCurveAndAddToPlot(const RimSummaryCurve* curve, RimSummaryPlot* plot, bool forceVisible)
{
    RimSummaryCurve* curveCopy =
        dynamic_cast<RimSummaryCurve*>(curve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(curveCopy);

    if (forceVisible)
    {
        curveCopy->setCurveVisiblity(true);
    }

    plot->addCurveNoUpdate(curveCopy);

    // The curve creator is not a descendant of the project, and need to be set manually
    curveCopy->setSummaryCaseY(curve->summaryCaseY());
    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::copyEnsembleCurveAndAddToCurveSet(const RimSummaryCurve* curve,
                                                               RimEnsembleCurveSet*   curveSet,
                                                               bool                   forceVisible)
{
    RimSummaryCurve* curveCopy =
        dynamic_cast<RimSummaryCurve*>(curve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(curveCopy);

    if (forceVisible)
    {
        curveCopy->setCurveVisiblity(true);
    }

    curveSet->addCurve(curveCopy);

    // The curve creator is not a descendant of the project, and need to be set manually
    curveCopy->setSummaryCaseY(curve->summaryCaseY());
    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::setDefaultCurveSelection(const std::vector<RimSummaryCase*>& defaultCases)
{
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setDefaultSelection(defaultCases);
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

        curveAppearanceCalc.getDimensions(&caseAppearance, &variAppearance, &wellAppearance, &gropAppearance, &regiAppearance);

        m_caseAppearanceType     = caseAppearance;
        m_variableAppearanceType = variAppearance;
        m_wellAppearanceType     = wellAppearance;
        m_groupAppearanceType    = gropAppearance;
        m_regionAppearanceType   = regiAppearance;
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

    for (const auto& curve : m_previewPlot->summaryAndEnsembleCurves())
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
        RimSummaryPlot* newSummaryPlot = nullptr;
        if (m_useAutoPlotTitleProxy())
        {
            newSummaryPlot = summaryPlotColl->createSummaryPlotWithAutoTitle();
        }
        else
        {
            QString candidatePlotName;
            if (m_previewPlot)
            {
                candidatePlotName = m_previewPlot->generatedPlotTitleFromVisibleCurves();
            }

            {
                bool ok           = false;
                candidatePlotName = QInputDialog::getText(nullptr,
                                                          "New Summary Plot Name",
                                                          "New Summary Plot Name",
                                                          QLineEdit::Normal,
                                                          candidatePlotName,
                                                          &ok,
                                                          RiuTools::defaultDialogFlags());
                if (!ok)
                {
                    return;
                }

                newSummaryPlot = summaryPlotColl->createNamedSummaryPlot(candidatePlotName);
            }
        }

        if (newSummaryPlot)
        {
            newSummaryPlot->loadDataAndUpdate();

            summaryPlotColl->updateConnectedEditors();

            m_targetPlot = newSummaryPlot;
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
bool RicSummaryCurveCreator::isObservedData(RimSummaryCase* sumCase) const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::proxyEnablePlotAutoTitle(const bool& enable)
{
    m_previewPlot->enableAutoPlotTitle(enable);
    m_previewPlot->enableShowPlotTitle(enable);
    m_previewPlot->updateCurveNames();
    m_previewPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::proxyPlotAutoTitle() const
{
    return m_previewPlot->autoPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::setInitialCurveVisibility(const RimSummaryPlot* targetPlot)
{
    // Set visibility for imported curves which were not checked in source plot
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> sourceCurveDefs;
    for (const auto& curve : targetPlot->summaryCurves())
    {
        sourceCurveDefs.insert(std::make_pair(curve->summaryCaseY(), curve->summaryAddressY()));
    }

    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        auto curveDef = std::make_pair(curve->summaryCaseY(), curve->summaryAddressY());
        if (sourceCurveDefs.count(curveDef) == 0)
        {
            curve->setCurveVisiblity(false);
        }
    }

    std::set<std::pair<RimSummaryCaseCollection*, RifEclipseSummaryAddress>> sourceCurveSetDefs;
    for (const auto& curveSet : targetPlot->ensembleCurveSetCollection()->curveSets())
    {
        sourceCurveSetDefs.insert(std::make_pair(curveSet->summaryCaseCollection(), curveSet->summaryAddress()));
    }

    for (const auto& curveSet : m_previewPlot->ensembleCurveSetCollection()->curveSets())
    {
        auto curveDef = std::make_pair(curveSet->summaryCaseCollection(), curveSet->summaryAddress());
        if (sourceCurveSetDefs.count(curveDef) == 0)
        {
            curveSet->showCurves(false);
        }
    }
}
