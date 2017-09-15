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
#include "RifReaderEclipseSummary.h"
#include "RigSummaryCaseData.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCase.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <algorithm>
#include <sstream>
#include <stack>
#include "RicSelectSummaryPlotUI.h"


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
    //CAF_PDM_InitObject("Curve Filter", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedCases, "SummaryCases", "Cases", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCategory, "IdentifierTypes", "Identifier types", "", "", "");

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

    CAF_PDM_InitFieldNoDefault(&m_previewPlot, "PreviewPlot", "PreviewPlot", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_targetPlot, "TargetPlot", "Target Plot", "", "", "");

    CAF_PDM_InitField(&m_useAutoAppearanceAssignment, "UseAutoAppearanceAssignment", true, "Auto", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_caseAppearanceType, "CaseAppearanceType", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variableAppearanceType, "VariableAppearanceType", "Vector", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellAppearanceType, "WellAppearanceType", "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupAppearanceType, "GroupAppearanceType", "Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionAppearanceType, "RegionAppearanceType", "Region", "", "", "");

    //CAF_PDM_InitFieldNoDefault(&m_selectedCurveTexts, "CurveTexts", "Selected Curves", "", "", "");
    //m_selectedCurveTexts.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

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

    m_selectedSummaryCategory.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedSummaryCategory.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_previewPlot.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    //m_previewPlot.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "", "", "", "");
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_closeButtonField, "Close", "", "", "", "");
    m_closeButtonField = false;
    m_closeButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_closeButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_createNewPlot, "CreateNewPlot", false, "Create New Plot", "", "", "");
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::setTargetPlot(RimSummaryPlot* targetPlot)
{
    m_targetPlot = targetPlot;
    if (targetPlot != nullptr)
    {
        populateCurveCreator(*targetPlot);
        updateConnectedEditors();
    }
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
    else if (changedField == &m_closeButtonField)
    {
        m_closeButtonField = false;
    }
    else
    {
        // Lookup item type input field
        auto identifierAndField = findIdentifierAndField(changedField);
        if (changedField == &m_selectedCases ||
            changedField == &m_useAutoAppearanceAssignment ||
            changedField == &m_caseAppearanceType ||
            changedField == &m_variableAppearanceType ||
            changedField == &m_wellAppearanceType ||
            changedField == &m_groupAppearanceType ||
            changedField == &m_regionAppearanceType ||
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
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (RimSummaryCase* rimCase : cases)
        {
            options.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), rimCase));
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
                auto name = QString::fromStdString(address.uiText(identifierAndField->summaryIdentifier()));
                if (!name.isEmpty())
                    itemNames.insert(name);
            }
            for (const auto& iName : itemNames)
            {
                options.push_back(caf::PdmOptionItemInfo(iName, iName));
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

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroupWithKeyword("Identifier Types", RicSummaryCurveCreatorUiKeywords::summaryTypes());
    itemTypesGroup->add(&m_selectedSummaryCategory);

    caf::PdmField<std::vector<QString>>* summaryiesField = nullptr;

    RifEclipseSummaryAddress::SummaryVarCategory sumCategory = m_selectedSummaryCategory();
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
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroupWithKeyword("Regions", "RegionsKeyword");
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
    //appearanceGroup->setCollapsedByDefault(true);
    appearanceGroup->add(&m_useAutoAppearanceAssignment);
    appearanceGroup->add(&m_caseAppearanceType);
    appearanceGroup->add(&m_variableAppearanceType);
    appearanceGroup->add(&m_wellAppearanceType);
    appearanceGroup->add(&m_groupAppearanceType);
    appearanceGroup->add(&m_regionAppearanceType);
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

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::findPossibleSummaryAddresses(const SummaryIdentifierAndField *identifierAndField)
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    auto isVectorField = identifierAndField != nullptr && identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;
    auto controllingIdentifierAndField = identifierAndField != nullptr ? lookupControllingField(identifierAndField) : nullptr;
    if (!isVectorField && controllingIdentifierAndField != nullptr && controllingIdentifierAndField->pdmField()->v().size() == 0)
    {
        return addrUnion;
    }

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
    std::vector<RicSummaryCurveCreator::SummaryIdentifierAndField*> selections;
    auto identifierAndFieldList = m_identifierFieldsMap[m_selectedSummaryCategory()];
    for (const auto& identifierAndFieldItem : identifierAndFieldList)
    {
        if (identifierAndFieldItem == identifierAndField)
        {
            break;
        }
        selections.push_back(identifierAndFieldItem);
    }
    return selections;
}

//--------------------------------------------------------------------------------------------------
/// Returns pdm field info from the specified pdm field
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::findIdentifierAndField(const caf::PdmFieldHandle* pdmFieldHandle)
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
/// Returns the parent pdm field info for the specified pdm field info.
/// If the specified pdm field info is the topmost (i.e. index is 0), null pointer is returned
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCreator::SummaryIdentifierAndField* RicSummaryCurveCreator::lookupControllingField(const RicSummaryCurveCreator::SummaryIdentifierAndField *identifierAndField)
{
    for (const auto& identifierAndFieldList : m_identifierFieldsMap)
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

std::set<RifEclipseSummaryAddress> RicSummaryCurveCreator::buildAddressListFromSelections()
{
    std::set<RifEclipseSummaryAddress> addressSet;
    for (const auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>> selectionStack;
        addSelectionAddress(identifierAndFieldList.first, identifierAndFieldList.second.begin(), addressSet, selectionStack);
    }
    return addressSet;
}

void RicSummaryCurveCreator::addSelectionAddress(RifEclipseSummaryAddress::SummaryVarCategory category,
                                                 std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr, 
                                                 std::set<RifEclipseSummaryAddress>& addressSet,
                                                 std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath)
{
    for (const auto& identifierText : (*identifierAndFieldItr)->pdmField()->v())
    {
        identifierPath.push_back(std::make_pair((*identifierAndFieldItr)->summaryIdentifier(), identifierText));
        if ((*identifierAndFieldItr)->summaryIdentifier() != RifEclipseSummaryAddress::INPUT_VECTOR_NAME)
        {
            addSelectionAddress(category, std::next(identifierAndFieldItr, 1), addressSet, identifierPath);
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
    syncCurvesFromUiSelection();
    //loadDataAndUpdate();

    //RimSummaryPlot* plot = nullptr;
    //firstAncestorOrThisOfType(plot);
    //plot->updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::syncCurvesFromUiSelection()
{
    // Create a search map containing whats supposed to be curves

    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > allCurveDefinitions;

    // Populate the newCurveDefinitions from the Gui

    std::set<RifEclipseSummaryAddress> selectedAddresses = buildAddressListFromSelections();

    // Todo: Move to separate method
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

                // Todo: Add text filter
                //if (!m_summaryFilter->isIncludedByFilter(allAddresses[i])) continue;
            }
        }
    }

    std::vector<RimSummaryCurve*> currentCurvesInPlot = m_previewPlot->summaryCurves();
    if (allCurveDefinitions.size() != currentCurvesInPlot.size())
    {
        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> currentCurveDefs;
        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> newCurveDefs;
        std::set<RimSummaryCurve*> deleteCurves;

        for (const auto& curve : currentCurvesInPlot)
        {
            currentCurveDefs.insert(std::make_pair(curve->summaryCase(), curve->summaryAddress()));
        }
        
        if (allCurveDefinitions.size() < currentCurvesInPlot.size())
        {
            // Determine which curves to delete from plot
            std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> deleteCurveDefs;
            std::set_difference(currentCurveDefs.begin(), currentCurveDefs.end(),
                                allCurveDefinitions.begin(), allCurveDefinitions.end(),
                                std::inserter(deleteCurveDefs, deleteCurveDefs.end()));

            for (const auto& curve : currentCurvesInPlot)
            {
                std::pair<RimSummaryCase*, RifEclipseSummaryAddress> curveDef = std::make_pair(curve->summaryCase(), curve->summaryAddress());
                if (deleteCurveDefs.count(curveDef))
                    deleteCurves.insert(curve);
            }
        }
        else
        {
            // Determine which curves are new since last time
            std::set_difference(allCurveDefinitions.begin(), allCurveDefinitions.end(),
                                currentCurveDefs.begin(), currentCurveDefs.end(),
                                std::inserter(newCurveDefs, newCurveDefs.end()));
        }
        updateCurvesFromCurveDefinitions(newCurveDefs, deleteCurves);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefsToAdd,
                                                              const std::set<RimSummaryCurve*>& curvesToDelete)
{
    RimSummaryCase* prevCase = nullptr;
    RimPlotCurve::LineStyleEnum lineStyle = RimPlotCurve::STYLE_SOLID;
    RimSummaryCurveAppearanceCalculator curveLookCalc(curveDefsToAdd, getAllSummaryCaseNames(), getAllSummaryWellNames());

    if (!m_useAutoAppearanceAssignment())
    {
        curveLookCalc.assignDimensions(m_caseAppearanceType(),
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

        curveLookCalc.getDimensions(&caseAppearance,
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

    // Delete curves
    for (const auto& curve : curvesToDelete)
    {
        m_previewPlot->deleteCurve(curve);
    }

    // Add new curves
    //m_previewPlot->deleteAllTopLevelCurves();
    for (const auto& curveDef : curveDefsToAdd)
    {
        RimSummaryCase* currentCase = curveDef.first;
        RimSummaryCurve* curve = new RimSummaryCurve();
        curve->setSummaryCase(currentCase);
        curve->setSummaryAddress(curveDef.second);
        m_previewPlot->addCurve(curve);
        curveLookCalc.setupCurveLook(curve);
        //m_currentCurvesInPlot.insert(std::make_pair(curveDef, curve));
        //curveTexts.push_back(QString::fromStdString( curveDef.second.uiText()));
    }

    //m_selectedCurveTexts = curveTexts;
    m_previewPlot->loadDataAndUpdate();
    m_previewPlot->updateConnectedEditors();
    m_previewPlot->zoomAll();

    //for (auto& caseAddrPair : curveDefinitions)
    //{
    //    RimSummaryCase* currentCase = caseAddrPair.first;

    //    RimSummaryCurve* curve = new RimSummaryCurve();
    //    curve->setParentQwtPlot(m_parentQwtPlot);
    //    curve->setSummaryCase(currentCase);
    //    curve->setSummaryAddress(caseAddrPair.second);
    //    curve->setYAxis(m_plotAxis());
    //    curve->applyCurveAutoNameSettings(*m_curveNameConfig());

    //    m_curves.push_back(curve);

    //    curveLookCalc.setupCurveLook(curve);
    //}
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
            attrib->m_buttonText = "Close";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Populate curve creator from the given curve collection
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::populateCurveCreator(const RimSummaryPlot& sourceSummaryPlot)
{
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
        copyCurveAndAddToPlot(curve, m_previewPlot);
    }
    m_previewPlot->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Copy curves from preview plot to target plot
// Todo: Do not copy curves already in target plot (?)
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::updateTargetPlot()
{
    if (m_targetPlot == nullptr)
        m_targetPlot = new RimSummaryPlot();
    
    for (const auto& curve : m_previewPlot->summaryCurves())
    {
        copyCurveAndAddToPlot(curve, m_targetPlot);
    }
    m_targetPlot->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCreator::copyCurveAndAddToPlot(const RimSummaryCurve *curve, RimSummaryPlot *plot)
{
    RimSummaryCurve* curveCopy = dynamic_cast<RimSummaryCurve*>(curve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(curveCopy);

    plot->addCurve(curveCopy);

    // Resolve references after object has been inserted into the project data model
    curveCopy->resolveReferencesRecursively();

    // The curve creator is not a descendant of the project, and need to be set manually
    curveCopy->setSummaryCase(curve->summaryCase());
    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate();
}
