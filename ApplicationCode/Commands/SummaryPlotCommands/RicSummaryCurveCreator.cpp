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

#include <sstream>
#include "RicSummaryCurveCreator.h"

#include "RiaApplication.h"
#include "RifReaderEclipseSummary.h"
#include "RigSummaryCaseData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCase.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "RimSummaryPlot.h"


CAF_PDM_SOURCE_INIT(RicSummaryCurveCreator, "RicSummaryCurveCreator");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::RicSummaryCurveCreator() : m_selectedIdentifiers(
{
    { RifEclipseSummaryAddress::SUMMARY_FIELD, {
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_AQUIFER, {
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_NETWORK, {
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_MISC, {
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_REGION,{
        { new SummaryIdentifierAndField(INPUT_REGION_NUMBER) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION,{
        { new SummaryIdentifierAndField(INPUT_REGION_NUMBER) },
        { new SummaryIdentifierAndField(INPUT_REGION2_NUMBER) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_GROUP,{
        { new SummaryIdentifierAndField(INPUT_WELL_GROUP_NAME) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL,{
        { new SummaryIdentifierAndField(INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION,{
        { new SummaryIdentifierAndField(INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR,{
        { new SummaryIdentifierAndField(INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_LGR,{
        { new SummaryIdentifierAndField(INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT,{
        { new SummaryIdentifierAndField(INPUT_WELL_NAME) },
        { new SummaryIdentifierAndField(INPUT_SEGMENT_NUMBER) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_BLOCK,{
        { new SummaryIdentifierAndField(INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR,{
        { new SummaryIdentifierAndField(INPUT_LGR_NAME) },
        { new SummaryIdentifierAndField(INPUT_CELL_IJK) },
        { new SummaryIdentifierAndField(INPUT_VECTOR_NAME) }
    } }
})
{

    //CAF_PDM_InitObject("Curve Filter", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedCases, "SummaryCases", "Cases", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCategory, "IdentifierTypes", "Identifier types", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField(), "FieldVectors", "Field vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField(), "AquiferVectors", "Aquifer vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_NETWORK][0]->pdmField(), "NetworkVectors", "Network vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_MISC][0]->pdmField(), "MiscVectors", "Misc vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION][0]->pdmField(), "Regions", "Regions", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION][1]->pdmField(), "RegionsVectors", "Regions vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][0]->pdmField(), "Region2RegionRegions", "Regions", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField(), "Region2RegionRegion2s", "Region2s", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][2]->pdmField(), "Region2RegionVectors", "Region2s vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][0]->pdmField(), "WellGroupWellGroupNames", "Well groups", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][1]->pdmField(), "WellGroupVectors", "Well group vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL][0]->pdmField(), "WellWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL][1]->pdmField(), "WellVectors", "Well vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][0]->pdmField(), "WellCompletionWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][1]->pdmField(), "WellCompletionIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][2]->pdmField(), "WellCompletionVectors", "Well completion vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField(), "WellCompletionLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField(), "WellCompletionLgrWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField(), "WellCompletionLgrIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField(), "WellCompletionLgrVectors", "Well completion vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][0]->pdmField(), "WellLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][1]->pdmField(), "WellLgrWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][2]->pdmField(), "WellLgrVectors", "Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][0]->pdmField(), "WellSegmentWellName", "Wells", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][1]->pdmField(), "WellSegmentNumber", "Segments", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][2]->pdmField(), "WellSegmentVectors", "Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK][0]->pdmField(), "BlockIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK][1]->pdmField(), "BlockVectors", "Block vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][0]->pdmField(), "BlockLgrLgrName", "Lgr names", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][1]->pdmField(), "BlockLgrIjk", "Cell IJK", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][2]->pdmField(), "BlockLgrVectors", "Block vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedCurves, "Curves", "Curves", "", "", "");

    for (const auto& itemTypes : m_selectedIdentifiers)
    {
        for (const auto& itemInputType : itemTypes.second)
        {
            itemInputType->pdmField()->uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
            itemInputType->pdmField()->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        }
    }

    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCases.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedSummaryCategory.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedSummaryCategory.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCurves.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedCurves.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::~RicSummaryCurveCreator()
{
    for (const auto& itemTypes : m_selectedIdentifiers)
    {
        for (const auto& itemTypeInput : itemTypes.second)
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
    else
    {
        // Lookup item type input field
        auto identifierAndField = findIdentifierAndField(fieldNeedingOptions);
        if (identifierAndField != nullptr)
        {
            auto pdmField = identifierAndField->pdmField();
            std::set<RifEclipseSummaryAddress> addrUnion = 
                findPossibleSummaryAddresses(identifierAndField);
            std::set<QString> itemNames;

            for (const auto& address : addrUnion)
            {
                auto name = getIdentifierTextFromAddress(identifierAndField->summaryIdentifier(), address);
                if (!name.isEmpty())
                    itemNames.insert(name);
            }
            for (const auto& iName : itemNames)
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
    itemTypesGroup->add(&m_selectedSummaryCategory);


    RifEclipseSummaryAddress::SummaryVarCategory sumCategory = m_selectedSummaryCategory();
    if (sumCategory == RifEclipseSummaryAddress::SUMMARY_FIELD)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blank");
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_AQUIFER)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blank");
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_NETWORK)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blank");
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_NETWORK][0]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_MISC)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blank");
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_MISC][0]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_REGION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Regions");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION][0]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION][1]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Regions");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][1]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION][2]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Well Groups");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][0]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_GROUP][1]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL][0]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL][1]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][1]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION][2]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][1]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_LGR][2]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Wells");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][1]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT][2]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_BLOCK)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blocks");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK][0]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK][1]->pdmField());
        }
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Blocks");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][0]->pdmField());
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][1]->pdmField());
        }

        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Vectors");
            myGroup->add(m_selectedIdentifiers[RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR][2]->pdmField());
        }
    }



    // Dynamic item input editors
/*
    auto pdmFields = m_selectedIdentifiers[m_selectedSummaryCategory()];
    if (pdmFields.size() > 0)
    {
        auto groupLabel = QString("%1 input").arg(m_selectedSummaryCategory().uiText());
        caf::PdmUiGroup* itemInputGroup = uiOrdering.addNewGroup(groupLabel);
        for (const auto& pdmField : pdmFields)
            itemInputGroup->add(pdmField->pdmField());
    }
*/


    caf::PdmUiGroup* curvesGroup = uiOrdering.addNewGroup("Curves");
    curvesGroup->add(&m_selectedCurves);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddresses(const SummaryIdentifierAndField *identifierAndField)
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    auto isVectorField = identifierAndField != nullptr && identifierAndField->summaryIdentifier() == INPUT_VECTOR_NAME;
    auto controllingIdentifierAndField = identifierAndField != nullptr ? lookupControllingField(identifierAndField) : nullptr;
    if (!isVectorField && controllingIdentifierAndField != nullptr && controllingIdentifierAndField->pdmField()->v().size() == 0)
        return addrUnion;

    for (RimSummaryCase* currCase : m_selectedCases)
    {
        RifReaderEclipseSummary* reader = nullptr;
        if (currCase && currCase->caseData()) reader = currCase->caseData()->summaryReader();
        if (reader)
        {
            const std::vector<RifEclipseSummaryAddress>& allAddresses = reader->allResultAddresses();
            int addressCount = static_cast<int>(allAddresses.size());

            bool applySelections = identifierAndField == nullptr || (!isVectorField && controllingIdentifierAndField != nullptr);
            std::vector<SummaryIdentifierAndField*> selections;
            if (applySelections)
            {
                // Build selections vector
                selections = buildControllingFieldList(identifierAndField);
            }

            for (int i = 0; i < addressCount; i++)
            {
                if (allAddresses[i].category() == m_selectedSummaryCategory())
                {
                    bool addressSelected = applySelections ? isAddressSelected(allAddresses[i], selections) : true;

                    // Todo: Add text filter
                    //if (!m_summaryFilter->isIncludedByFilter(allAddresses[i])) continue;

                    if (addressSelected)
                        addrUnion.insert(allAddresses[i]);
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
    std::vector<RicSummaryCurveCreator::SummaryIdentifierAndField*> selections;
    auto identifierAndFieldList = m_selectedIdentifiers[m_selectedSummaryCategory()];
    for (const auto& identifierAndFieldItem : identifierAndFieldList)
    {
        if (identifierAndFieldItem == identifierAndField)
            break;
        selections.push_back(identifierAndFieldItem);
    }
    return selections;
}

//--------------------------------------------------------------------------------------------------
/// Returns the stringified value for the specified value in the specfied address object
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCreator::getIdentifierTextFromAddress(SummaryIdentifierType itemTypeInput, const RifEclipseSummaryAddress &address)
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
    case INPUT_VECTOR_NAME: return QString::fromStdString(address.quantityName());
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// Returns pdm field info from the specified pdm field
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::findIdentifierAndField(const caf::PdmFieldHandle* pdmFieldHandle)
{
    for (const auto& itemTypes : m_selectedIdentifiers)
    {
        for (const auto& itemTypeInput : itemTypes.second)
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
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::lookupControllingField(const RicSummaryCurveCreator::SummaryIdentifierAndField *identifierAndField)
{
    for (const auto& identifierAndFieldList : m_selectedIdentifiers)
    {
        int index = 0;
        for (const auto& iaf : identifierAndFieldList.second)
        {
            if (iaf == identifierAndField)
            {
                return index > 0 ? identifierAndFieldList.second[index - 1] : nullptr;
            }
            index++;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Returns true if the specified address object matches the selections
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCreator::isAddressSelected(const RifEclipseSummaryAddress &address, const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList)
{
    for (const auto& identifierAndField : identifierAndFieldList)
    {
        bool match = false;
        for (const auto& selection : identifierAndField->pdmField()->v())
        {
            if (QString::compare(getIdentifierTextFromAddress(identifierAndField->summaryIdentifier(), address), selection) == 0)
            {
                match = true;
                break;
            }
        }
        if (!match)
            return false;
    }
    return true;
}
