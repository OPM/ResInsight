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
#include "RifReaderEclipseSummary.h"
#include "RigSummaryCaseData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCase.h"
#include "cafPdmUiListEditor.h"


namespace caf
{
    template<>
    void caf::AppEnum<RicSummaryCurveCreator::ItemType>::setUp()
    {
        addItem(RicSummaryCurveCreator::SUM_FILTER_FIELD, "SUM_FILTER_FIELD", "Field");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL, "SUM_FILTER_WELL", "Well");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL_GROUP, "SUM_FILTER_WELL_GROUP", "Group");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL_COMPLETION, "SUM_FILTER_WELL_COMPLETION", "Completion");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL_SEGMENT, "SUM_FILTER_SEGMENT", "Segment");
        addItem(RicSummaryCurveCreator::SUM_FILTER_BLOCK, "SUM_FILTER_BLOCK", "Block");
        addItem(RicSummaryCurveCreator::SUM_FILTER_REGION, "SUM_FILTER_REGION", "Region");
        addItem(RicSummaryCurveCreator::SUM_FILTER_REGION_2_REGION, "SUM_FILTER_REGION_2_REGION", "Region-Region");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL_LGR, "SUM_FILTER_WELL_LGR", "Lgr-Well");
        addItem(RicSummaryCurveCreator::SUM_FILTER_WELL_COMPLETION_LGR, "SUM_FILTER_WELL_COMPLETION_LGR", "Lgr-Completion");
        addItem(RicSummaryCurveCreator::SUM_FILTER_BLOCK_LGR, "SUM_FILTER_BLOCK_LGR", "Lgr-Block");
        addItem(RicSummaryCurveCreator::SUM_FILTER_MISC, "SUM_FILTER_MISC", "Misc");
        addItem(RicSummaryCurveCreator::SUM_FILTER_AQUIFER, "SUM_FILTER_AQUIFER", "Aquifer");
        addItem(RicSummaryCurveCreator::SUM_FILTER_NETWORK, "SUM_FILTER_NETWORK", "Network");
        setDefault(RicSummaryCurveCreator::SUM_FILTER_FIELD);
    }
}


CAF_PDM_SOURCE_INIT(RicSummaryCurveCreator, "RicSummaryCurveCreator");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::RicSummaryCurveCreator() : m_itemTypePdmFields(
{
    { SUM_FILTER_REGION,{
        { new PdmFieldInfo(INPUT_REGION_NUMBER, 0)}
    } },
    { SUM_FILTER_REGION_2_REGION,{
        { new PdmFieldInfo(INPUT_REGION_NUMBER, 0)},
        { new PdmFieldInfo(INPUT_REGION2_NUMBER, 1)}
    } },
    { SUM_FILTER_WELL_GROUP,{
        { new PdmFieldInfo(INPUT_WELL_GROUP_NAME, 0)}
    } },
    { SUM_FILTER_WELL,{
        { new PdmFieldInfo(INPUT_WELL_NAME, 0)}
    } },
    { SUM_FILTER_WELL_COMPLETION,{
        { new PdmFieldInfo(INPUT_WELL_NAME, 0)},
        { new PdmFieldInfo(INPUT_CELL_IJK, 1)}
    } },
    { SUM_FILTER_WELL_COMPLETION_LGR,{
        { new PdmFieldInfo(INPUT_LGR_NAME, 0)},
        { new PdmFieldInfo(INPUT_WELL_NAME, 1)},
        { new PdmFieldInfo(INPUT_CELL_IJK, 2)}
    } },
    { SUM_FILTER_WELL_LGR,{
        { new PdmFieldInfo(INPUT_LGR_NAME, 0)},
        { new PdmFieldInfo(INPUT_WELL_NAME, 1)}
    } },
    { SUM_FILTER_WELL_SEGMENT,{
        { new PdmFieldInfo(INPUT_WELL_NAME, 0)},
        { new PdmFieldInfo(INPUT_SEGMENT_NUMBER, 1)}
    } },
    { SUM_FILTER_BLOCK,{
        { new PdmFieldInfo(INPUT_CELL_IJK, 0)}
    } },
    { SUM_FILTER_BLOCK_LGR,{
        { new PdmFieldInfo(INPUT_LGR_NAME, 0)},
        { new PdmFieldInfo(INPUT_CELL_IJK, 1)}
    } }
})
{

    //CAF_PDM_InitObject("Curve Filter", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedCases, "SummaryCases", "Cases", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedItemType, "ItemTypes", "Item types", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedVectors, "Vectors", "Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_REGION][0]->pdmField(), "Regions", "Regions", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_REGION_2_REGION][0]->pdmField(), "Region2RegionRegions", "Regions", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_REGION_2_REGION][1]->pdmField(), "Region2RegionRegion2s", "Region2s", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_GROUP][0]->pdmField(), "WellGroupWellGroupNames", "Well groups", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL][0]->pdmField(), "WellWellName", "Wells", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_COMPLETION][0]->pdmField(), "WellCompletionWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_COMPLETION][1]->pdmField(), "WellCompletionIjk", "Cell IJK", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_COMPLETION_LGR][0]->pdmField(), "WellCompletionLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_COMPLETION_LGR][1]->pdmField(), "WellCompletionLgrWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_COMPLETION_LGR][2]->pdmField(), "WellCompletionLgrIjk", "Cell IJK", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_LGR][0]->pdmField(), "WellLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_LGR][1]->pdmField(), "WellLgrWellName", "Wells", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_SEGMENT][0]->pdmField(), "WellSegmentWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_WELL_SEGMENT][1]->pdmField(), "WellSegmentNumber", "Segments", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_BLOCK][0]->pdmField(), "BlockIjk", "Cell IJK", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_BLOCK_LGR][0]->pdmField(), "BlockLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_itemTypePdmFields[SUM_FILTER_BLOCK_LGR][1]->pdmField(), "BlockLgrIjk", "Cell IJK", "", "", "");

    // Todo: Init all objects

    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedItemType.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedItemType.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::~RicSummaryCurveCreator()
{
    for (auto itemTypes : m_itemTypePdmFields)
    {
        for (auto itemTypeInput : itemTypes.second)
        {
            delete itemTypeInput->pdmField();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

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
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (RimSummaryCase* rimCase : cases)
        {
            options.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), rimCase));
        }
    }
    else if (fieldNeedingOptions == &m_selectedVectors)
    {
        std::set<RifEclipseSummaryAddress> addrUnion = findPossibleSummaryAddresses();

        std::vector<QString> quantityNames;
        for (const auto& address : addrUnion)
        {
            std::string name = address.quantityName();
            quantityNames.push_back(QString::fromStdString(name));
        }
        std::sort(quantityNames.begin(), quantityNames.end());
        quantityNames.erase(std::unique(quantityNames.begin(), quantityNames.end()), quantityNames.end());
        for (auto qName : quantityNames)
            options.push_back(caf::PdmOptionItemInfo(qName, qName));
    }
    else
    {
        // Lookup item type input field(s)
        auto pdmFieldInfo = findPdmFieldInfo(fieldNeedingOptions);
        if (pdmFieldInfo != nullptr)
        {
            auto pdmField = pdmFieldInfo->pdmField();
            auto parentPdmField = findParentPdmFieldInfo(pdmFieldInfo);
            auto selections = buildSelectionVector(pdmField);
            std::set<RifEclipseSummaryAddress> addrUnion = findPossibleSummaryAddresses(selections, pdmFieldInfo->index() == 0);
            std::vector<QString> itemNames;

            for (const auto& address : addrUnion)
            {
                auto name = getItemTypeValueFromAddress(pdmFieldInfo->itemTypeInput(), address);
                if (!name.isEmpty())
                    itemNames.push_back(name);
            }
            std::sort(itemNames.begin(), itemNames.end());
            itemNames.erase(std::unique(itemNames.begin(), itemNames.end()), itemNames.end());
            for (auto iName : itemNames)
                options.push_back(caf::PdmOptionItemInfo(iName, iName));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroup("Sources");
    sourcesGroup->add(&m_selectedCases);

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroup("Items Types");
    itemTypesGroup->add(&m_selectedItemType);

    // Dynamic item input editors
    auto pdmFields = m_itemTypePdmFields[m_selectedItemType()];
    if (pdmFields.size() > 0)
    {
        auto groupLabel = QString("%1 input").arg(m_selectedItemType().uiText());
        caf::PdmUiGroup* itemInputGroup = uiOrdering.addNewGroup(groupLabel);
        for (auto pdmField : pdmFields)
            itemInputGroup->add(pdmField->pdmField());
    }

    caf::PdmUiGroup* vectorsGroup = uiOrdering.addNewGroup("Vectors");
    vectorsGroup->add(&m_selectedVectors);


    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddresses()
{
    return findPossibleSummaryAddresses(std::vector<Selection>(), true);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddresses(const std::vector<Selection> &selections, bool ignoreSelections)
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    for (RimSummaryCase* currCase : m_selectedCases)
    {
        RifReaderEclipseSummary* reader = nullptr;
        if (currCase && currCase->caseData()) reader = currCase->caseData()->summaryReader();

        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();
            int addressCount = static_cast<int>(allAddresses.size());

            auto selectedItemType = mapItemType(m_selectedItemType.v());

            for (int i = 0; i < addressCount; i++)
            {
                bool addressSelected = !ignoreSelections ? isAddressSelected(allAddresses[i], selections) : true;

                // Todo: Add text filter
                //if (!m_summaryFilter->isIncludedByFilter(allAddresses[i])) continue;

                if (allAddresses[i].category() == selectedItemType && addressSelected)
                    addrUnion.insert(allAddresses[i]);
            }
        }
    }
    return addrUnion;
}

//--------------------------------------------------------------------------------------------------
/// Maps ItemType to SummaryVarCategory
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::SummaryVarCategory RicSummaryCurveCreator::mapItemType(ItemType itemType)
{
    switch (itemType)
    {
    case SUM_FILTER_FIELD: return RifEclipseSummaryAddress::SUMMARY_FIELD;
    case SUM_FILTER_AQUIFER: return RifEclipseSummaryAddress::SUMMARY_AQUIFER;
    case SUM_FILTER_NETWORK: return RifEclipseSummaryAddress::SUMMARY_NETWORK;
    case SUM_FILTER_MISC: return RifEclipseSummaryAddress::SUMMARY_MISC;
    case SUM_FILTER_REGION: return RifEclipseSummaryAddress::SUMMARY_REGION;
    case SUM_FILTER_REGION_2_REGION: return RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION;
    case SUM_FILTER_WELL_GROUP: return RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
    case SUM_FILTER_WELL: return RifEclipseSummaryAddress::SUMMARY_WELL;
    case SUM_FILTER_WELL_COMPLETION: return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
    case SUM_FILTER_WELL_COMPLETION_LGR: return RifEclipseSummaryAddress::SUMMARY_WELL_LGR;
    case SUM_FILTER_WELL_LGR: return RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR;
    case SUM_FILTER_WELL_SEGMENT: return RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
    case SUM_FILTER_BLOCK: return RifEclipseSummaryAddress::SUMMARY_BLOCK;
    case SUM_FILTER_BLOCK_LGR: return RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
    default: return RifEclipseSummaryAddress::SUMMARY_FIELD;
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the stringified value for the specified value in the specfied address object
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCreator::getItemTypeValueFromAddress(ItemTypeInput itemTypeInput, const RifEclipseSummaryAddress &address)
{
    switch (itemTypeInput)
    {
    case INPUT_REGION_NUMBER: return QString("%1").arg(address.regionNumber());
    case INPUT_REGION2_NUMBER: return QString("%1").arg(address.regionNumber2());
    case INPUT_WELL_NAME: return QString::fromStdString(address.wellName());
    case INPUT_WELL_GROUP_NAME: return QString::fromStdString(address.wellGroupName());
    case INPUT_CELL_IJK: return QString("%1,%2,%3").arg(QString::number(address.cellI()),
        QString::number(address.cellJ()), QString::number(address.cellK()));
    case INPUT_LGR_NAME: return QString::fromStdString(address.lgrName());
    case INPUT_SEGMENT_NUMBER: return QString("%1").arg(address.wellSegmentNumber());
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// Returns pdm field info from teh specified pdm field
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::PdmFieldInfo* RicSummaryCurveCreator::findPdmFieldInfo(const caf::PdmFieldHandle* pdmFieldHandle)
{
    for (auto itemTypes : m_itemTypePdmFields)
    {
        for (auto itemTypeInput : itemTypes.second)
        {
            if (pdmFieldHandle == itemTypeInput->pdmField())
                return itemTypeInput;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Returns the parent pdm field info for the specified pdm field info.
/// If the specified pdm field info is the topmost (i.e. index is 0), null pointer is returned
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::PdmFieldInfo* RicSummaryCurveCreator::findParentPdmFieldInfo(const RicSummaryCurveCreator::PdmFieldInfo *pdmFieldInfo)
{
    if (pdmFieldInfo->index() > 0)
    {
        for (auto itemTypes : m_itemTypePdmFields)
        {
            for (auto info : itemTypes.second)
            {
                if (info->index() == pdmFieldInfo->index() - 1)
                    return info;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Returns true if the specified address object matches at least one of the selections
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::isAddressSelected(const RifEclipseSummaryAddress &address, const std::vector<Selection> &selections)
{
    for (auto sel : selections)
    {
        if (QString::compare(getItemTypeValueFromAddress(sel.first, address), sel.second) == 0)
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns a list of all selections made in the pdm fields 'above' the specified pdm field
//--------------------------------------------------------------------------------------------------
std::vector<RicSummaryCurveCreator::Selection> RicSummaryCurveCreator::buildSelectionVector(const caf::PdmFieldHandle *pdmField)
{
    std::vector<Selection> selections;
    auto itemType = findItemTypeFromPdmField(pdmField);
    auto itemTypeInputs = m_itemTypePdmFields[itemType];
    for (auto itemTypeInput : itemTypeInputs)
    {
        if (itemTypeInput->pdmField() == pdmField)
            break;
        for (auto sel : itemTypeInput->pdmField()->v())
        {
            selections.push_back(std::make_pair(itemTypeInput->itemTypeInput(), sel));
        }
    }
    return selections;
}

//--------------------------------------------------------------------------------------------------
/// REturns the ItemType for the specified pdm field
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::ItemType RicSummaryCurveCreator::findItemTypeFromPdmField(const caf::PdmFieldHandle *pdmField)
{
    for (auto itemTypes : m_itemTypePdmFields)
    {
        for (auto itemTypeInput : itemTypes.second)
        {
            if (pdmField == itemTypeInput->pdmField())
                return itemTypes.first;
        }
    }
    // Default?
    return ItemType::SUM_FILTER_FIELD;
}
