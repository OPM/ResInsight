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

#include "RicSummaryCurveCreatorUiKeywords.h"
#include "RicSelectSummaryPlotUI.h"

#include "RifReaderEclipseSummary.h"
#include "RigSummaryCaseData.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCase.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCurveCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedData.h"

#include "RiuMainPlotWindow.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QInputDialog>

#include <algorithm>
#include <sstream>
#include <stack>
#include "RimSummaryCaseMainCollection.h"
#include "RimOilField.h"
#include "RimSummaryCaseCollection.h"


CAF_PDM_SOURCE_INIT(RicSummaryCurveCreator, "RicSummaryCurveCreator");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::RicSummaryCurveCreator() : m_identifierFieldsMap(
{
    { RifEclipseSummaryAddress::SUMMARY_FIELD, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_AQUIFER, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_NETWORK, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_MISC, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_REGION,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_REGION_NUMBER) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_REGION_NUMBER) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_REGION2_NUMBER) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_GROUP,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_GROUP_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_LGR,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_SEGMENT_NUMBER) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_BLOCK,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } }
})
{
    CAF_PDM_InitFieldNoDefault(&m_selectedCases, "SummaryCases", "Cases", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_currentSummaryCategory, "CurrentSummaryCategory", "Current Summary Category", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCategories, "SelectedSummaryCategories", "Summary Categories", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField(), "FieldVectors", "Field vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField(), "AquiferVectors", "Aquifer Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_NETWORK][0]->pdmField(), "NetworkVectors", "Network Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_MISC][0]->pdmField(), "MiscVectors", "Misc Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION][0]->pdmField(), "Regions", "Regions", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION][1]->pdmField(), "RegionsVectors", "Regions Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][0]->pdmField(), "Region2RegionRegions", "Regions", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField(), "Region2RegionRegion2s", "Region2s", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][2]->pdmField(), "Region2RegionVectors", "Region2s Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][0]->pdmField(), "WellGroupWellGroupNames", "Well groups", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][1]->pdmField(), "WellGroupVectors", "Well Group Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL][0]->pdmField(), "WellWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL][1]->pdmField(), "WellVectors", "Well Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][0]->pdmField(), "WellCompletionWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][1]->pdmField(), "WellCompletionIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][2]->pdmField(), "WellCompletionVectors", "Well Completion Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField(), "WellCompletionLgrLgrName", "LGR Names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField(), "WellCompletionLgrWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField(), "WellCompletionLgrIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField(), "WellCompletionLgrVectors", "Well Completion Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][0]->pdmField(), "WellLgrLgrName", "LGR Names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][1]->pdmField(), "WellLgrWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][2]->pdmField(), "WellLgrVectors", "Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][0]->pdmField(), "WellSegmentWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][1]->pdmField(), "WellSegmentNumber", "Segments", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][2]->pdmField(), "WellSegmentVectors", "Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK][0]->pdmField(), "BlockIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK][1]->pdmField(), "BlockVectors", "Block Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][0]->pdmField(), "BlockLgrLgrName", "LGR Names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][1]->pdmField(), "BlockLgrIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][2]->pdmField(), "BlockLgrVectors", "Block Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_targetPlot, "TargetPlot", "Target Plot", "", "", "");

    CAF_PDM_InitField(&m_useAutoAppearanceAssignment, "UseAutoAppearanceAssignment", true, "Auto", "", "", "");
    CAF_PDM_InitField(&m_appearanceApplyButton, "AppearanceApplyButton", false, "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_caseAppearanceType, "CaseAppearanceType", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variableAppearanceType, "VariableAppearanceType", "Vector", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellAppearanceType, "WellAppearanceType", "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupAppearanceType, "GroupAppearanceType", "Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionAppearanceType, "RegionAppearanceType", "Region", "", "", "");

    m_previewPlot = new RimSummaryPlot();

    for (const auto& itemTypes : m_identifierFieldsMap)
    {
        for (const auto& itemInputType : itemTypes.second)
        {
            itemInputType->pdmField()->uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
            itemInputType->pdmField()->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        }
        itemTypes.second.back()->pdmField()->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    }

    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCases.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_selectedSummaryCategories.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSummaryCategories.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_currentSummaryCategory.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "", "", "", "");
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_closeButtonField, "Close", "", "", "", "");
    m_closeButtonField = false;
    m_closeButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_closeButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_createNewPlot, "CreateNewPlot", false, "Create New Plot", "", "", "");
    m_createNewPlot.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_createNewPlot.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_appearanceApplyButton = false;
    m_appearanceApplyButton.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_appearanceApplyButton.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LEFT);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::~RicSummaryCurveCreator()
{
    for (const auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        for (const auto& identifierAndField : identifierAndFieldList.second)
        {
            delete identifierAndField->pdmField();
        }
    }

    delete m_previewPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateFromSummaryPlot(RimSummaryPlot* targetPlot)
{
    if (m_targetPlot != targetPlot)
    {
        resetAllFields();
    }
    
    m_targetPlot = targetPlot;
    m_useAutoAppearanceAssignment = true;

    if (m_targetPlot)
    {
        populateCurveCreator(*m_targetPlot);
        loadDataAndUpdatePlot();
    }

    caf::PdmUiItem::updateConnectedEditors();
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
    if (changedField == &m_applyButtonField)
    {
        m_applyButtonField = false;

        updateTargetPlot();
    }
    else if (changedField == &m_createNewPlot)
    {
        m_createNewPlot = false;

        RimProject* proj = RiaApplication::instance()->project();

        RimSummaryPlotCollection* summaryPlotColl = proj->mainPlotCollection()->summaryPlotCollection();
        if (summaryPlotColl)
        {
            QString summaryPlotName = QString("SummaryPlot %1").arg(summaryPlotColl->summaryPlots().size() + 1);

            bool ok = false;
            summaryPlotName = QInputDialog::getText(NULL, "New Summary Plot Name", "New Summary Plot Name", QLineEdit::Normal, summaryPlotName, &ok);
            if (ok)
            {
                RimSummaryPlot* plot = new RimSummaryPlot();
                summaryPlotColl->summaryPlots().push_back(plot);

                plot->setDescription(summaryPlotName);
                plot->loadDataAndUpdate();

                summaryPlotColl->updateConnectedEditors();

                RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
                if (mainPlotWindow)
                {
                    mainPlotWindow->selectAsCurrentItem(plot);
                    mainPlotWindow->setExpanded(plot, true);
                }

                m_createNewPlot = false;
                m_targetPlot = plot;

                updateTargetPlot();
            }
        }
    }
    else if (changedField == &m_appearanceApplyButton)
    {
        applyAppearanceToAllPreviewCurves();
        m_previewPlot->loadDataAndUpdate();
        m_appearanceApplyButton = false;
    }
    else
    {
        // Lookup item type input field
        auto identifierAndField = lookupIdentifierAndFieldFromFieldHandle(changedField);
        if (changedField == &m_selectedCases ||
            changedField == &m_selectedSummaryCategories ||
            identifierAndField != nullptr)
        {
            loadDataAndUpdatePlot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSummaryCurveCreator::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_selectedCases)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCase*> topLevelCases;
        std::vector<RimOilField*> oilFields;

        proj->allOilFields(oilFields);
        for (RimOilField* oilField : oilFields)
        {
            RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
            if (sumCaseMainColl)
            {
                // Top level cases
                for (const auto& sumCase : sumCaseMainColl->topLevelSummaryCases())
                {
                    options.push_back(caf::PdmOptionItemInfo(sumCase->caseName(), sumCase));
                }

                // Grouped cases
                for (const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections())
                {
                    options.push_back(caf::PdmOptionItemInfo::createHeader(sumCaseColl->name(), true));

                    for (const auto& sumCase : sumCaseColl->allSummaryCases())
                    {
                        auto optionItem = caf::PdmOptionItemInfo(sumCase->caseName(), sumCase);
                        optionItem.setLevel(1);
                        options.push_back(optionItem);
                    }
                }

                // Observed data
                auto observedDataColl = oilField->observedDataCollection();
                if (observedDataColl->allObservedData().size() > 0)
                {
                    options.push_back(caf::PdmOptionItemInfo::createHeader("Observed Data", true));

                    for (const auto& obsData : observedDataColl->allObservedData())
                    {
                        auto optionItem = caf::PdmOptionItemInfo(obsData->caseName(), obsData);
                        optionItem.setLevel(1);
                        options.push_back(optionItem);
                    }
                }
            }
        }
    }
    else if (fieldNeedingOptions == &m_targetPlot)
    {
        RimProject* proj = RiaApplication::instance()->project();
        
        RimSummaryPlotCollection* summaryPlotColl = proj->mainPlotCollection()->summaryPlotCollection();
        if (summaryPlotColl)
        {
            summaryPlotColl->summaryPlotItemInfos(&options);
        }
    }
    else if (fieldNeedingOptions == &m_selectedSummaryCategories)
    {
        for (size_t i = 0; i < caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::size(); ++i)
        {
            options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiTextFromIndex(i),
                                                     caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::fromIndex(i)));
        }
    }
    else
    {
        // Lookup item type input field
        auto identifierAndField = lookupIdentifierAndFieldFromFieldHandle(fieldNeedingOptions);
        if (identifierAndField != nullptr)
        {
            enum {SUM_CASES, OBS_DATA};
            bool includeObservedData = identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;
            std::set<RifEclipseSummaryAddress> addrUnion[2];
            addrUnion[SUM_CASES] = findPossibleSummaryAddressesFromSelectedCases(identifierAndField);
            addrUnion[OBS_DATA] = includeObservedData ?
                findPossibleSummaryAddressesFromSelectedObservedData(identifierAndField) : std::set<RifEclipseSummaryAddress>();

            auto pdmField = identifierAndField->pdmField();
            std::set<QString> itemNames;

            for(int i = 0; i < 2; i++)
            {
                for (const auto& address : addrUnion[i])
                {
                    auto name = QString::fromStdString(address.uiText(identifierAndField->summaryIdentifier()));
                    if (!name.isEmpty())
                        itemNames.insert(name);
                }

                // Create headers only when observed data is selected
                bool createHeaders = addrUnion[OBS_DATA].size() > 0;
                if (createHeaders)
                {
                    auto headerText = i == SUM_CASES ? QString("Simulated Data") : QString("Observed Data");
                    options.push_back(caf::PdmOptionItemInfo::createHeader(headerText, true));
                }
                for (const auto& iName : itemNames)
                {
                    auto optionItem = caf::PdmOptionItemInfo(iName, iName);
                    if (createHeaders)
                        optionItem.setLevel(1);
                    options.push_back(optionItem);
                }

                if (!includeObservedData) break;
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", RicSummaryCurveCreatorUiKeywords::sources());
    sourcesGroup->add(&m_selectedCases);

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroupWithKeyword("Summary Types", RicSummaryCurveCreatorUiKeywords::summaryTypes());
    itemTypesGroup->add(&m_selectedSummaryCategories);

    caf::PdmField<std::vector<QString>>* summaryiesField = nullptr;

    RifEclipseSummaryAddress::SummaryVarCategory sumCategory = m_currentSummaryCategory();
    if (sumCategory == RifEclipseSummaryAddress::SUMMARY_FIELD)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_AQUIFER)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_NETWORK)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_NETWORK][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_MISC)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_MISC][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_REGION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Regions");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION][0]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION][1]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Regions");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][2]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Well Groups");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][0]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][1]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL][0]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL][1]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Completions");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][1]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][2]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("LGR Completions");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("LGR Wells");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][1]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][2]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Well Segments");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][1]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][2]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_BLOCK)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blocks");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK][0]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK][1]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("LGR Blocks");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][0]->pdmField());
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][1]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][2]->pdmField();
    }

    CAF_ASSERT(summaryiesField);
    caf::PdmUiGroup* summariesGroup = uiOrdering.addNewGroupWithKeyword("Summaries", RicSummaryCurveCreatorUiKeywords::summaries());
    summariesGroup->add(summaryiesField);


    // Appearance settings
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroupWithKeyword("Appearance Settings", RicSummaryCurveCreatorUiKeywords::appearance());
    appearanceGroup->setCollapsedByDefault(true);
    appearanceGroup->add(&m_useAutoAppearanceAssignment);
    appearanceGroup->add(&m_caseAppearanceType);
    appearanceGroup->add(&m_variableAppearanceType);
    appearanceGroup->add(&m_wellAppearanceType);
    appearanceGroup->add(&m_groupAppearanceType);
    appearanceGroup->add(&m_regionAppearanceType);
    appearanceGroup->add(&m_appearanceApplyButton);

    // Appearance option sensitivity
    {
        m_caseAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_variableAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_wellAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_groupAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_regionAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
    }

    // Fields to be displayed directly in UI
    uiOrdering.add(&m_createNewPlot);
    uiOrdering.add(&m_targetPlot);
    uiOrdering.add(&m_applyButtonField);
    uiOrdering.add(&m_closeButtonField);

    m_targetPlot.uiCapability()->setUiReadOnly(m_createNewPlot);

    uiOrdering.skipRemainingFields(true);
}

std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddressesFromSelectedCases(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<RimSummaryCase*> cases;
    for (const auto& sumCase: m_selectedCases)
    {
        if(typeid(sumCase) == typeid(RimObservedData)) continue;
        cases.push_back(sumCase);
    }
    return findPossibleSummaryAddresses(cases, identifierAndField);
}

std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddressesFromSelectedObservedData(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<RimSummaryCase*> obsData;
    for (const auto& sumCase : m_selectedCases)
    {
        if (typeid(sumCase) == typeid(RimObservedData))
        {
            obsData.push_back(sumCase);
        }
    }
    return findPossibleSummaryAddresses(obsData, identifierAndField);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddresses(const std::vector<RimSummaryCase*> &selectedCases, 
                                                                                        const SummaryIdentifierAndField *identifierAndField)
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    auto isVectorField = identifierAndField != nullptr && identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;
    auto controllingIdentifierAndField = identifierAndField != nullptr ? lookupControllingField(identifierAndField) : nullptr;
    if (!isVectorField && controllingIdentifierAndField != nullptr && controllingIdentifierAndField->pdmField()->v().size() == 0)
    {
        return addrUnion;
    }

    for (RimSummaryCase* currCase : selectedCases)
    {
        RifReaderEclipseSummary* reader = nullptr;
        if (currCase && currCase->caseData()) reader = currCase->caseData()->summaryReader();
        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress>& allAddresses = reader->allResultAddresses();
            int addressCount = static_cast<int>(allAddresses.size());

            bool applySelections = identifierAndField == nullptr || (!isVectorField && controllingIdentifierAndField != nullptr);
            std::vector<SummaryIdentifierAndField*> controllingFields;
            if (applySelections)
            {
                // Build selections vector
                controllingFields = buildControllingFieldList(identifierAndField);
            }

            for (int i = 0; i < addressCount; i++)
            {
                if (allAddresses[i].category() == m_currentSummaryCategory())
                {
                    bool addressSelected = applySelections ? isAddressCompatibleWithControllingFieldSelection(allAddresses[i], controllingFields) : true;
                    if (addressSelected)
                    {
                        addrUnion.insert(allAddresses[i]);
                    }
                }
            }
        }
    }
    return addrUnion;
}

//--------------------------------------------------------------------------------------------------
/// Build a list of relevant selections
//--------------------------------------------------------------------------------------------------
std::vector<RicSummaryCurveCreator::SummaryIdentifierAndField*> RicSummaryCurveCreator::buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<RicSummaryCurveCreator::SummaryIdentifierAndField*> controllingFields;
    auto identifierAndFieldList = m_identifierFieldsMap[m_currentSummaryCategory()];
    for (const auto& identifierAndFieldItem : identifierAndFieldList)
    {
        if (identifierAndFieldItem == identifierAndField)
        {
            break;
        }
        controllingFields.push_back(identifierAndFieldItem);
    }
    return controllingFields;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::lookupIdentifierAndFieldFromFieldHandle(const caf::PdmFieldHandle* pdmFieldHandle)
{
    for (const auto& itemTypes : m_identifierFieldsMap)
    {
        for (const auto& itemTypeInput : itemTypes.second)
        {
            if (pdmFieldHandle == itemTypeInput->pdmField())
            {
                return itemTypeInput;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Returns the Controlling pdm field info for the specified pdm field info.
/// Controlling means the field controlling the dependent field
/// If the specified pdm field info is the topmost (i.e. index is 0), null pointer is returned
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::lookupControllingField(const RicSummaryCurveCreator::SummaryIdentifierAndField *dependentField)
{
    for (const auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        int index = 0;
        for (const auto& iaf : identifierAndFieldList.second)
        {
            if (iaf == dependentField)
            {
                return index > 0 ? identifierAndFieldList.second[index - 1] : nullptr;
            }
            index++;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::isAddressCompatibleWithControllingFieldSelection(const RifEclipseSummaryAddress &address, const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList)
{
    for (const auto& identifierAndField : identifierAndFieldList)
    {
        bool match = false;
        for (const auto& selectedText : identifierAndField->pdmField()->v())
        {
            if (QString::compare(QString::fromStdString(address.uiText(identifierAndField->summaryIdentifier())), selectedText) == 0)
            {
                match = true;
                break;
            }
        }

        if (!match)
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::buildAddressListFromSelections()
{
    std::set<RifEclipseSummaryAddress> addressSet;
    for (const auto& category : m_selectedSummaryCategories())
    {
        auto identifierAndFieldList = m_identifierFieldsMap[category];
        std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>> selectionStack;
        buildAddressListForCategoryRecursively(category, identifierAndFieldList.begin(), addressSet, selectionStack);
    }
    return addressSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::buildAddressListForCategoryRecursively(RifEclipseSummaryAddress::SummaryVarCategory category,
                                                 std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr, 
                                                 std::set<RifEclipseSummaryAddress>& addressSet,
                                                 std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath)
{
    for (const auto& identifierText : (*identifierAndFieldItr)->pdmField()->v())
    {
        identifierPath.push_back(std::make_pair((*identifierAndFieldItr)->summaryIdentifier(), identifierText));
        if ((*identifierAndFieldItr)->summaryIdentifier() != RifEclipseSummaryAddress::INPUT_VECTOR_NAME)
        {
            buildAddressListForCategoryRecursively(category, std::next(identifierAndFieldItr, 1), addressSet, identifierPath);
        }
        else
        {
            std::map<RifEclipseSummaryAddress::SummaryIdentifierType, std::string> selectedIdentifiers;
            for (const auto& identifier : identifierPath)
            {
                selectedIdentifiers.insert(std::make_pair(identifier.first, identifier.second.toStdString()));
            }
            auto address = RifEclipseSummaryAddress(category, selectedIdentifiers);
            addressSet.insert(address);
        }
        identifierPath.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::loadDataAndUpdatePlot()
{
    syncPreviewCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::syncPreviewCurvesFromUiSelection()
{
    // Create a search map containing whats supposed to be curves

    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > allCurveDefinitions;

    // Populate the newCurveDefinitions from the Gui

    std::set<RifEclipseSummaryAddress> selectedAddresses = buildAddressListFromSelections();

    // Find the addresses to display
    std::set<RifEclipseSummaryAddress> addrUnion;
    for (RimSummaryCase* currCase : m_selectedCases)
    {
        RifReaderEclipseSummary* reader = nullptr;
        if (currCase && currCase->caseData()) reader = currCase->caseData()->summaryReader();
        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress>& allAddresses = reader->allResultAddresses();
            int addressCount = static_cast<int>(allAddresses.size());

            for (int i = 0; i < addressCount; i++)
            {
                if (selectedAddresses.count(allAddresses[i]) > 0)
                {
                    addrUnion.insert(allAddresses[i]);
                    allCurveDefinitions.insert(std::make_pair(currCase, allAddresses[i]));
                }
            }
        }
    }

    std::vector<RimSummaryCurve*> currentCurvesInPreviewPlot = m_previewPlot->summaryCurves();
    if (allCurveDefinitions.size() != currentCurvesInPreviewPlot.size())
    {
        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> currentCurveDefs;
        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> newCurveDefs;
        std::set<RimSummaryCurve*> curvesToDelete;

        for (const auto& curve : currentCurvesInPreviewPlot)
        {
            currentCurveDefs.insert(std::make_pair(curve->summaryCase(), curve->summaryAddress()));
        }
        
        if (allCurveDefinitions.size() < currentCurvesInPreviewPlot.size())
        {
            // Determine which curves to delete from plot
            std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> deleteCurveDefs;
            std::set_difference(currentCurveDefs.begin(), currentCurveDefs.end(),
                                allCurveDefinitions.begin(), allCurveDefinitions.end(),
                                std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

            for (const auto& curve : currentCurvesInPreviewPlot)
            {
                std::pair<RimSummaryCase*, RifEclipseSummaryAddress> curveDef = std::make_pair(curve->summaryCase(), curve->summaryAddress());
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
void RicSummaryCurveCreator::updatePreviewCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& allCurveDefsToDisplay, 
                                                                     const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefsToAdd,
                                                                     const std::set<RimSummaryCurve*>& curvesToDelete)
{
    RimSummaryCase* prevCase = nullptr;
    RimPlotCurve::LineStyleEnum lineStyle = RimPlotCurve::STYLE_SOLID;
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
        RimSummaryCase* currentCase = curveDef.first;
        RimSummaryCurve* curve = new RimSummaryCurve();
        curve->setSummaryCase(currentCase);
        curve->setSummaryAddress(curveDef.second);
        m_previewPlot->addCurve(curve);
        curveLookCalc.setupCurveLook(curve);
    }

    m_previewPlot->loadDataAndUpdate();
    m_previewPlot->zoomAll();
    updateEditorsConnectedToPreviewPlot();
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
        RifReaderEclipseSummary* reader = nullptr;
        if (rimCase && rimCase->caseData())
        {
            reader = rimCase->caseData()->summaryReader();
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
    else if (&m_createNewPlot == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->m_buttonText = "New Plot";
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
    else if (&m_selectedSummaryCategories == field)
    {
        caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*> (attribute);
        if (attrib)
        {
            attrib->fieldToReceiveCurrentItemValue = &m_currentSummaryCategory;
            attrib->showTextFilter = false;
            attrib->showToggleAllCheckbox = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Populate curve creator from the given curve collection
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::populateCurveCreator(const RimSummaryPlot& sourceSummaryPlot)
{
    m_selectedSummaryCategories.v().clear();
    for (size_t i = 0; i < caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::size(); ++i)
    {
        m_selectedSummaryCategories.v().push_back(caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::fromIndex(i));
    }

    m_previewPlot->deleteAllSummaryCurves();
    for (const auto& curve : sourceSummaryPlot.summaryCurves())
    {
        // Select case if not already selected
        if (std::find(m_selectedCases.begin(), m_selectedCases.end(), curve->summaryCase()) == m_selectedCases.end())
        {
            m_selectedCases.push_back(curve->summaryCase());
        }

        auto identifierAndFieldList = m_identifierFieldsMap[curve->summaryAddress().category()];
        for (const auto& identifierAndField : identifierAndFieldList)
        {
            QString uiText = QString::fromStdString(curve->summaryAddress().uiText(identifierAndField->summaryIdentifier()));
            const auto& currentSelectionVector = identifierAndField->pdmField()->v();
            if (std::find(currentSelectionVector.begin(), currentSelectionVector.end(), uiText) == currentSelectionVector.end())
            {
                std::vector<QString> newSelectionVector(currentSelectionVector.begin(), currentSelectionVector.end());
                newSelectionVector.push_back(uiText);
                (*identifierAndField->pdmField()) = newSelectionVector;
            }
        }

        // Copy curve object to the preview plot
        copyCurveAndAddToPlot(curve, m_previewPlot, true);
    }

    syncPreviewCurvesFromUiSelection();

    // Set visibility for imported curves which were not checked in source plot
    std::set <std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> sourceCurveDefs;
    for (const auto& curve : sourceSummaryPlot.summaryCurves())
    {
        sourceCurveDefs.insert(std::make_pair(curve->summaryCase(), curve->summaryAddress()));
    }
    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        auto curveDef = std::make_pair(curve->summaryCase(), curve->summaryAddress());
        if (sourceCurveDefs.count(curveDef) == 0)
            curve->setCurveVisiblity(false);
    }
    updateEditorsConnectedToPreviewPlot();
    updateAppearanceEditor();
}

//--------------------------------------------------------------------------------------------------
/// Copy curves from preview plot to target plot
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateTargetPlot()
{
    if (m_targetPlot == nullptr)
        m_targetPlot = new RimSummaryPlot();

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
        curveCopy->setCurveVisiblity(true);
    plot->addCurve(curveCopy);

    // Resolve references after object has been inserted into the project data model
    curveCopy->resolveReferencesRecursively();

    // The curve creator is not a descendant of the project, and need to be set manually
    curveCopy->setSummaryCase(curve->summaryCase());
    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::resetAllFields()
{
    m_selectedCases.clear();

    m_previewPlot->deleteAllSummaryCurves();
    m_targetPlot = nullptr;

    // clear all state in fields
    for (auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        for (auto a : identifierAndFieldList.second)
        {
            a->pdmField()->v().clear();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateEditorsConnectedToPreviewPlot()
{
    m_previewPlot->updateConnectedEditors();
    m_previewPlot->summaryCurveCollection()->updateConnectedEditors();
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
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> allCurveDefs = allPreviewCurveDefs();
    
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
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> allCurveDefs = allPreviewCurveDefs();

    RimSummaryCurveAppearanceCalculator curveLookCalc(allCurveDefs, getAllSummaryCaseNames(), getAllSummaryWellNames());
    initCurveAppearanceCalculator(curveLookCalc);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> RicSummaryCurveCreator::allPreviewCurveDefs() const
{
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> allCurveDefs;

    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        allCurveDefs.insert(std::make_pair(curve->summaryCase(), curve->summaryAddress()));
    }
    return allCurveDefs;
}