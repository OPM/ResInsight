/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RiuSummaryCurveDefSelection.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveDefinition.h"

#include "RiuSummaryCurveDefinitionKeywords.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <algorithm>



CAF_PDM_SOURCE_INIT(RiuSummaryCurveDefSelection, "RicSummaryAddressSelection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelection::RiuSummaryCurveDefSelection() : m_identifierFieldsMap(
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
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_REGION_2_REGION) },
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
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField(), "Region2RegionVectors", "Region2s Vectors", "", "", "");

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


    for (const auto& itemTypes : m_identifierFieldsMap)
    {
        for (const auto& itemInputType : itemTypes.second)
        {
            itemInputType->pdmField()->uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

            itemInputType->pdmField()->uiCapability()->setUiLabelPosition(itemTypes.second.size() > 2 ?
                                                                          caf::PdmUiItemInfo::TOP : caf::PdmUiItemInfo::HIDDEN);

            itemInputType->pdmField()->uiCapability()->setAutoAddingOptionFromValue(false);
        }
        itemTypes.second.back()->pdmField()->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    }

    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCases.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_selectedSummaryCategories.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSummaryCategories.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_currentSummaryCategory.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelection::~RiuSummaryCurveDefSelection()
{
    for (const auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        for (const auto& identifierAndField : identifierAndFieldList.second)
        {
            delete identifierAndField->pdmField();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryCurveDefSelection::selectedCurveDefinitions() const
{
    std::vector<RiaSummaryCurveDefinition> caseAndAddressVector;
    
    {
        std::set<RiaSummaryCurveDefinition> caseAndAddressPairs;

        std::set<RifEclipseSummaryAddress> selectedAddressesFromUi = buildAddressListFromSelections();

        for (RimSummaryCase* currCase : m_selectedCases)
        {
            if (currCase && currCase->summaryReader())
            {
                RifSummaryReaderInterface* reader = currCase->summaryReader();

                const std::vector<RifEclipseSummaryAddress>& readerAddresses = reader->allResultAddresses();
                for (const auto& readerAddress : readerAddresses)
                {
                    if (selectedAddressesFromUi.count(readerAddress) > 0)
                    {
                        caseAndAddressPairs.insert(RiaSummaryCurveDefinition(currCase, readerAddress));
                    }
                }
            }
        }

        std::copy(caseAndAddressPairs.begin(), caseAndAddressPairs.end(), std::back_inserter(caseAndAddressVector));
    }

    return caseAndAddressVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::setSelectedCurveDefinitions(const std::vector<RiaSummaryCurveDefinition>& curveDefinitions)
{
    resetAllFields();

    for (const auto& caseAddressPair : curveDefinitions)
    {
        RimSummaryCase* summaryCase = caseAddressPair.summaryCase();
        const RifEclipseSummaryAddress& summaryAddress = caseAddressPair.summaryAddress();

        // Select summary category if not already selected
        auto& selectedCategories = m_selectedSummaryCategories();
        if (std::find(selectedCategories.begin(), selectedCategories.end(),
                      summaryAddress.category()) == selectedCategories.end())
        {
            m_selectedSummaryCategories.v().push_back(summaryAddress.category());
        }

        // Select case if not already selected
        if (std::find(m_selectedCases.begin(), m_selectedCases.end(), summaryCase) == m_selectedCases.end())
        {
            m_selectedCases.push_back(summaryCase);
        }

        bool isObservedDataCase = isObservedData(summaryCase);

        auto identifierAndFieldList = m_identifierFieldsMap[summaryAddress.category()];
        for (const auto& identifierAndField : identifierAndFieldList)
        {
            bool isVectorField = identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;
            QString avalue = QString::fromStdString(summaryAddress.uiText(identifierAndField->summaryIdentifier()));
            if (isVectorField && isObservedDataCase)
            {
                avalue = avalue + QString(OBSERVED_DATA_AVALUE_POSTFIX);
            }
            const auto& currentSelectionVector = identifierAndField->pdmField()->v();
            if (std::find(currentSelectionVector.begin(), currentSelectionVector.end(), avalue) == currentSelectionVector.end())
            {
                std::vector<QString> newSelectionVector(currentSelectionVector.begin(), currentSelectionVector.end());
                newSelectionVector.push_back(avalue);
                (*identifierAndField->pdmField()) = newSelectionVector;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiuSummaryCurveDefSelection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
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
            std::set<RifEclipseSummaryAddress> addrUnion[2];
            addrUnion[SUM_CASES] = findPossibleSummaryAddressesFromSelectedCases(identifierAndField);
            addrUnion[OBS_DATA] =  findPossibleSummaryAddressesFromSelectedObservedData(identifierAndField);

            std::set<QString> itemNames[2];
            for (int i = 0; i < 2; i++)
            {
                for (const auto& address : addrUnion[i])
                {
                    auto name = QString::fromStdString(address.uiText(identifierAndField->summaryIdentifier()));
                    if (!name.isEmpty())
                        itemNames[i].insert(name);
                }
            }

            bool isVectorField = identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;

            // Merge sets for all other fields than vector fields
            if (!isVectorField)
            {
                itemNames[SUM_CASES].insert(itemNames[OBS_DATA].begin(), itemNames[OBS_DATA].end());
                itemNames[OBS_DATA].clear();
            }

            auto pdmField = identifierAndField->pdmField();
            for(int i = 0; i < 2; i++)
            {
                // Create headers only for vector fields when observed data is selected
                bool hasObservedData = itemNames[OBS_DATA].size() > 0;
                bool groupItems = isVectorField && hasObservedData;
                if (groupItems)
                {
                    auto headerText = i == SUM_CASES ? QString("Simulated Data") : QString("Observed Data");
                    options.push_back(caf::PdmOptionItemInfo::createHeader(headerText, true));
                }

                auto itemPostfix = (isVectorField && i == OBS_DATA) ? QString(OBSERVED_DATA_AVALUE_POSTFIX) : QString("");
                for (const auto& iName : itemNames[i])
                {
                    auto optionItem = caf::PdmOptionItemInfo(iName, iName + itemPostfix);
                    if (groupItems)
                        optionItem.setLevel(1);
                    options.push_back(optionItem);
                }

                if (itemNames[OBS_DATA].size() == 0) break;
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword("Sources", RiuSummaryCurveDefinitionKeywords::sources());
    sourcesGroup->add(&m_selectedCases);

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroupWithKeyword("Summary Types", RiuSummaryCurveDefinitionKeywords::summaryTypes());
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
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField();
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
    caf::PdmUiGroup* summariesGroup = uiOrdering.addNewGroupWithKeyword("Summaries", RiuSummaryCurveDefinitionKeywords::summaries());
    summariesGroup->add(summaryiesField);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddressesFromSelectedCases(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<RimSummaryCase*> cases;
    for (const auto& sumCase: m_selectedCases)
    {
        if(isObservedData(sumCase)) continue;
        cases.push_back(sumCase);
    }
    return findPossibleSummaryAddresses(cases, identifierAndField);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddressesFromSelectedObservedData(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<RimSummaryCase*> obsData;
    for (const auto& sumCase : m_selectedCases)
    {
        if (isObservedData(sumCase))
        {
            obsData.push_back(sumCase);
        }
    }
    return findPossibleSummaryAddresses(obsData, identifierAndField);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddresses(const std::vector<RimSummaryCase*> &selectedCases, 
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
        RifSummaryReaderInterface* reader = nullptr;
        if (currCase) reader = currCase->summaryReader();
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
std::vector<RiuSummaryCurveDefSelection::SummaryIdentifierAndField*> RiuSummaryCurveDefSelection::buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField) const
{
    std::vector<RiuSummaryCurveDefSelection::SummaryIdentifierAndField*> controllingFields;
    const auto& identifierAndFieldList = m_identifierFieldsMap.at(m_currentSummaryCategory());
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
RiuSummaryCurveDefSelection::SummaryIdentifierAndField* RiuSummaryCurveDefSelection::lookupIdentifierAndFieldFromFieldHandle(const caf::PdmFieldHandle* pdmFieldHandle) const
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
RiuSummaryCurveDefSelection::SummaryIdentifierAndField* RiuSummaryCurveDefSelection::lookupControllingField(const RiuSummaryCurveDefSelection::SummaryIdentifierAndField *dependentField) const
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
bool RiuSummaryCurveDefSelection::isAddressCompatibleWithControllingFieldSelection(const RifEclipseSummaryAddress &address, const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList) const
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
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::buildAddressListFromSelections() const
{
    std::set<RifEclipseSummaryAddress> addressSet;
    for (const auto& category : m_selectedSummaryCategories())
    {
        if (category == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_INVALID) continue;

        const auto& identifierAndFieldList = m_identifierFieldsMap.at(category);
        std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>> selectionStack;
        buildAddressListForCategoryRecursively(category, identifierAndFieldList.begin(), selectionStack, addressSet);
    }
    return addressSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::buildAddressListForCategoryRecursively(RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                    std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr,
                                                                    std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath,
                                                                    std::set<RifEclipseSummaryAddress>& addressSet) const

{
    for (const auto& identifierText : (*identifierAndFieldItr)->pdmField()->v())
    {
        auto idText = identifierText;
        idText.remove(OBSERVED_DATA_AVALUE_POSTFIX);
        identifierPath.push_back(std::make_pair((*identifierAndFieldItr)->summaryIdentifier(), idText));
        if ((*identifierAndFieldItr)->summaryIdentifier() != RifEclipseSummaryAddress::INPUT_VECTOR_NAME)
        {
            buildAddressListForCategoryRecursively(category, std::next(identifierAndFieldItr, 1), identifierPath, addressSet);
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
void RiuSummaryCurveDefSelection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_selectedSummaryCategories == field)
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
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::resetAllFields()
{
    m_selectedCases.clear();
    m_selectedSummaryCategories = std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>();

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
bool RiuSummaryCurveDefSelection::isObservedData(RimSummaryCase *sumCase) const
{
    return dynamic_cast<RimObservedData*>(sumCase) != nullptr;
}
