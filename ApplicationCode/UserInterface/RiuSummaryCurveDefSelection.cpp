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
#include "RiaStdStringTools.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaCurveSetDefinition.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RimCalculatedSummaryCase.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuSummaryCurveDefinitionKeywords.h"
#include "RiuSummaryVectorDescriptionMap.h"

#include "cafPdmUiTreeSelectionEditor.h"
//#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <algorithm>



CAF_PDM_SOURCE_INIT(RiuSummaryCurveDefSelection, "RicSummaryAddressSelection");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class SummaryIdentifierAndField
{
public:
    SummaryIdentifierAndField() :
        m_summaryIdentifier((RifEclipseSummaryAddress::SummaryIdentifierType)0),
        m_pdmField(nullptr)
    {}

    SummaryIdentifierAndField(RifEclipseSummaryAddress::SummaryIdentifierType summaryIdentifier) :
        m_summaryIdentifier(summaryIdentifier),
        m_pdmField(new caf::PdmField<std::vector<QString>>())
    {}

    virtual ~SummaryIdentifierAndField() { delete m_pdmField; }

    RifEclipseSummaryAddress::SummaryIdentifierType summaryIdentifier() const { return m_summaryIdentifier; }
    caf::PdmField<std::vector<QString>>*            pdmField() { return m_pdmField; }

private:
    RifEclipseSummaryAddress::SummaryIdentifierType m_summaryIdentifier;
    caf::PdmField<std::vector<QString>> *           m_pdmField;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSummaryCurveDefSelection::RiuSummaryCurveDefSelection() : m_identifierFieldsMap(
{
    { RifEclipseSummaryAddress::SUMMARY_FIELD, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_AQUIFER, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_AQUIFER_NUMBER) },
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
    } },
    { RifEclipseSummaryAddress::SUMMARY_CALCULATED, {
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
    { RifEclipseSummaryAddress::SUMMARY_IMPORTED,{
        { new SummaryIdentifierAndField(RifEclipseSummaryAddress::INPUT_VECTOR_NAME) }
    } },
})
{
    CAF_PDM_InitFieldNoDefault(&m_selectedSources, "SummaryCases", "Cases", "", "", "");
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue(false);


    CAF_PDM_InitFieldNoDefault(&m_currentSummaryCategory, "CurrentSummaryCategory", "Current Summary Category", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCategories, "SelectedSummaryCategories", "Summary Categories", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField(), "FieldVectors", "Field vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField(), "Aquifers", "Aquifers", "", "", "");
    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][1]->pdmField(), "AquiferVectors", "Aquifer Vectors", "", "", "");

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

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_CALCULATED][0]->pdmField(), "CalculatedVectors", "Calculated Vectors", "", "", "");

    CAF_PDM_InitFieldNoDefault(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_IMPORTED][0]->pdmField(), "ImportedVectors", "Imported vectors", "", "", "");

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

    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedSources.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSources.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_selectedSummaryCategories.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_selectedSummaryCategories.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_currentSummaryCategory.uiCapability()->setUiHidden(true);
    m_multiSelectionMode = false;
    m_hideEnsembles = false;
    m_hideSummaryCases = false;

    m_prevCurveCount = 0;
    m_prevCurveSetCount = 0;
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
std::vector<RiaSummaryCurveDefinition> RiuSummaryCurveDefSelection::allCurveDefinitionsFromSelection() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefVector;
    
    {
        std::set<RiaSummaryCurveDefinition> curveDefinitions;
        std::set<RifEclipseSummaryAddress> selectedAddressesFromUi = buildAddressListFromSelections();

        for (SummarySource* currSource : selectedSummarySources())
        {
            RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>(currSource);
            RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>(currSource);

            std::set<RifEclipseSummaryAddress> addressesFromSource;
            std::vector<RimSummaryCase*>       casesFromSource;

            // Build case list
            if (ensemble)
            {
                auto addresses = ensemble->ensembleSummaryAddresses();
                addressesFromSource.insert(addresses.begin(), addresses.end());
                auto ensembleCases = ensemble->allSummaryCases();
                casesFromSource.insert(casesFromSource.end(), ensembleCases.begin(), ensembleCases.end());
            }
            else
            {
                RifSummaryReaderInterface* reader = sumCase ? sumCase->summaryReader() : nullptr;
                if (reader)
                {
                    addressesFromSource.insert(reader->allResultAddresses().begin(), reader->allResultAddresses().end());
                    casesFromSource.push_back(sumCase);
                }
            }

            for (auto caseFromSource : casesFromSource)
            {
                for (const auto& addressFromSource : addressesFromSource)
                {
                    if (selectedAddressesFromUi.count(addressFromSource) > 0)
                    {
                        curveDefinitions.insert(RiaSummaryCurveDefinition(caseFromSource, addressFromSource, ensemble));
                    }
                }
            }
        }

        std::copy(curveDefinitions.begin(), curveDefinitions.end(), std::back_inserter(curveDefVector));
    }

    return curveDefVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RiaCurveSetDefinition> RiuSummaryCurveDefSelection::allCurveSetDefinitionsFromSelections() const
{
    std::vector<RiaCurveSetDefinition> curveSetDefVector;
    std::set<RiaCurveSetDefinition> curveSetDefinitions;
    std::set<RifEclipseSummaryAddress> selectedAddressesFromUi = buildAddressListFromSelections();

    for (SummarySource* currSource : selectedSummarySources())
    {
        RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>(currSource);
        if (!ensemble) continue;

        std::set<RifEclipseSummaryAddress> addressesFromSource;

        // Build case list
        auto addresses = ensemble->ensembleSummaryAddresses();
        addressesFromSource.insert(addresses.begin(), addresses.end());

        for (const auto& addressFromSource : addressesFromSource)
        {
            if (selectedAddressesFromUi.count(addressFromSource) > 0)
            {
                curveSetDefinitions.insert(RiaCurveSetDefinition(ensemble, addressFromSource));
            }
        }
    }

    std::copy(curveSetDefinitions.begin(), curveSetDefinitions.end(), std::back_inserter(curveSetDefVector));
    return curveSetDefVector;
}

//--------------------------------------------------------------------------------------------------
/// One CurveDefinition pr ensemble curve set
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryCurveDefSelection::selection() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefSelection;
    std::set<RifEclipseSummaryAddress> selectedAddressesFromUi = buildAddressListFromSelections();
    for (SummarySource* currSource : selectedSummarySources())
    {
        RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>(currSource);
        RimSummaryCase* sourceCase = dynamic_cast<RimSummaryCase*>(currSource);

        if (ensemble)
        {
            std::set<RifEclipseSummaryAddress> addressUnion = ensemble->ensembleSummaryAddresses();
            for ( const auto& addr : selectedAddressesFromUi)
            {
                if (addressUnion.count(addr))
                {
                    curveDefSelection.push_back(RiaSummaryCurveDefinition(nullptr, addr, ensemble));
                }
            }
        }
        else
        {
            if (!(sourceCase &&  sourceCase->summaryReader())) continue;

            const std::set<RifEclipseSummaryAddress>& readerAddresses = sourceCase->summaryReader()->allResultAddresses();
            for ( const auto& addr : selectedAddressesFromUi)
            {
                if (readerAddresses.count(addr))
                {
                    curveDefSelection.push_back(RiaSummaryCurveDefinition(sourceCase, addr, nullptr));
                }
            }
        }
    }

    return curveDefSelection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::setMultiSelectionMode(bool multiSelectionMode)
{
    m_multiSelectionMode = multiSelectionMode;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::hideEnsembles(bool hide)
{
    m_hideEnsembles = hide;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::hideSummaryCases(bool hide)
{
    m_hideSummaryCases = hide;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::setFieldChangedHandler(const std::function<void()>& handlerFunc)
{
    m_toggleChangedHandler = handlerFunc;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::setDefaultSelection(const std::vector<SummarySource*>& defaultSources)
{
    RimProject* proj = RiaApplication::instance()->project();
    auto allSumCases = proj->allSummaryCases();
    auto allSumGroups = proj->summaryGroups();

    if (allSumCases.size() > 0)
    {
        RifEclipseSummaryAddress defaultAddress = RifEclipseSummaryAddress::fieldAddress("FOPT");

        std::vector<SummarySource*> selectTheseSources = defaultSources;
        if (selectTheseSources.empty()) selectTheseSources.push_back(allSumCases[0]);

        std::vector<RiaSummaryCurveDefinition> curveDefs;
        for(SummarySource* s : selectTheseSources)
        {
            RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>(s);
            RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>(s);

            RiaSummaryCurveDefinition curveDef(sumCase, defaultAddress, ensemble);
            curveDefs.push_back(curveDef);
        }

        setSelectedCurveDefinitions(curveDefs);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::setSelectedCurveDefinitions(const std::vector<RiaSummaryCurveDefinition>& curveDefinitions)
{
    resetAllFields();

    for (const auto& curveDef : curveDefinitions)
    {
        if (!(curveDef.summaryCase() || curveDef.isEnsembleCurve()) ) continue;

        RimSummaryCase* summaryCase = curveDef.summaryCase();

        RifEclipseSummaryAddress summaryAddress = curveDef.summaryAddress();
        if (summaryAddress.category() == RifEclipseSummaryAddress::SUMMARY_INVALID)
        {
            // If we have an invalid address, set the default address to Field
            summaryAddress = RifEclipseSummaryAddress::fieldAddress(summaryAddress.quantityName());
        }

        // Ignore ensemble statistics curves
        if (summaryAddress.category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS) continue;

        // Select summary category if not already selected
        auto& selectedCategories = m_selectedSummaryCategories();

        if (std::find(selectedCategories.begin(), selectedCategories.end(),
                      summaryAddress.category()) == selectedCategories.end())
        {
            m_selectedSummaryCategories.v().push_back(summaryAddress.category());
        }

        // Select case if not already selected
        SummarySource* summSource = curveDef.isEnsembleCurve() ? static_cast<SummarySource*>(curveDef.ensemble()) : summaryCase;
        if (std::find(m_selectedSources.begin(), m_selectedSources.end(), summSource) == m_selectedSources.end())
        {
            if (summaryCase != calculatedSummaryCase())
            {
                m_selectedSources.push_back(summSource);
            }
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
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddressesFromCalculated()
{
    std::set<RifEclipseSummaryAddress> addressSet;

    if (m_currentSummaryCategory == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
    {
        RimSummaryCase* calcSumCase = calculatedSummaryCase();

        const std::set<RifEclipseSummaryAddress> allAddresses = calcSumCase->summaryReader()->allResultAddresses();
        for (const auto& adr : allAddresses)
        {
            addressSet.insert(adr);
        }
    }

    return addressSet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField != &m_selectedSources && changedField != &m_selectedSummaryCategories &&
        changedField != &m_currentSummaryCategory)
    {
        RifEclipseSummaryAddress::SummaryVarCategory currentCategory = m_currentSummaryCategory();
        if (currentCategory != RifEclipseSummaryAddress::SUMMARY_INVALID)
        {
            // When a summary vector is selected, make sure the summary category for the summary vector is in the selection
            // Note that we use the size of the variant to avoid this operation when an item in unchecked

            if (newValue.toList().size() > oldValue.toList().size())
            {
                if (std::find(m_selectedSummaryCategories.v().begin(), m_selectedSummaryCategories.v().end(), currentCategory) ==
                    m_selectedSummaryCategories.v().end())
                {
                    m_selectedSummaryCategories.v().push_back(currentCategory);
                }
            }
        }
    }

    size_t curveCount = allCurveDefinitionsFromSelection().size();
    size_t curveSetCount = allCurveSetDefinitionsFromSelections().size();

    if (m_toggleChangedHandler != nullptr)
    {
        // Do nothing if the curve count and curve set count is identical
        if ((curveCount != m_prevCurveCount || curveSetCount != m_prevCurveSetCount))
        {
            m_toggleChangedHandler();

            m_prevCurveCount = curveCount;
            m_prevCurveSetCount = curveSetCount;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiuSummaryCurveDefSelection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_selectedSources)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimOilField*> oilFields;

        proj->allOilFields(oilFields);
        for (RimOilField* oilField : oilFields)
        {
            RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
            if (sumCaseMainColl)
            {
                if (!m_hideSummaryCases)
                {
                    // Top level cases
                    for (const auto& sumCase : sumCaseMainColl->topLevelSummaryCases())
                    {
                        options.push_back(caf::PdmOptionItemInfo(sumCase->caseName(), sumCase));
                    }
                }

                // Ensembles
                if (!m_hideEnsembles)
                {
                    bool ensembleHeaderCreated = false;
                    for (const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections())
                    {
                        if (!sumCaseColl->isEnsemble()) continue;

                        if (!ensembleHeaderCreated)
                        {
                            options.push_back(caf::PdmOptionItemInfo::createHeader("Ensembles", true));
                            ensembleHeaderCreated = true;
                        }

                        auto optionItem = caf::PdmOptionItemInfo(sumCaseColl->name(), sumCaseColl);
                        optionItem.setLevel(1);
                        options.push_back(optionItem);
                    }
                }

                if (!m_hideSummaryCases)
                {
                    // Grouped cases
                    for (const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections())
                    {
                        if (sumCaseColl->isEnsemble()) continue;

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
    }
    else if (fieldNeedingOptions == &m_selectedSummaryCategories)
    {
        std::vector<RifEclipseSummaryAddress::SummaryVarCategory> sortedCategoriesForUi;

        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_FIELD);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_AQUIFER);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_NETWORK);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_MISC);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION_2_REGION);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_GROUP);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_COMPLETION);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_SEGMENT);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_BLOCK);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_LGR);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_COMPLETION_LGR);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_BLOCK_LGR);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_CALCULATED);
        sortedCategoriesForUi.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_IMPORTED);
        // NB SUMMARY_ENSEMBLE_STATISTICS is intentionally excluded
        //categoriesForUiDisplay.push_back(RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_ENSEMBLE_STATISTICS);

        for (auto category : sortedCategoriesForUi)
        {
            auto uiText = caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText(category);
            options.push_back(caf::PdmOptionItemInfo(uiText, category));
        }
    }
    else
    {
        // Lookup item type input field
        auto identifierAndField = lookupIdentifierAndFieldFromFieldHandle(fieldNeedingOptions);
        if (identifierAndField != nullptr)
        {
            enum {SUM_CASES, OBS_DATA, CALCULATED_CURVES};

            const int itemCount = CALCULATED_CURVES + 1;

            std::set<RifEclipseSummaryAddress> addrUnion[itemCount];
            addrUnion[SUM_CASES] = findPossibleSummaryAddressesFromSelectedCases(identifierAndField);
            addrUnion[OBS_DATA] =  findPossibleSummaryAddressesFromSelectedObservedData(identifierAndField);
            addrUnion[CALCULATED_CURVES] =  findPossibleSummaryAddressesFromCalculated();

            std::set<std::string> itemNames[itemCount];
            for (int i = 0; i < itemCount; i++)
            {
                for (const auto& address : addrUnion[i])
                {
                    if (address.isErrorResult()) continue;

                    auto name = address.uiText(identifierAndField->summaryIdentifier());
                    if (name.size() > 0)
                    {
                        itemNames[i].insert(name);
                    }
                }
            }

            bool isVectorField = identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;

            // Merge sets for all other fields than vector fields
            if (!isVectorField)
            {
                itemNames[SUM_CASES].insert(itemNames[OBS_DATA].begin(), itemNames[OBS_DATA].end());
                itemNames[OBS_DATA].clear();
            }

            for(int i = 0; i < itemCount; i++)
            {
                // Create headers only for vector fields when observed data is selected
                bool hasObservedData = itemNames[OBS_DATA].size() > 0;
                bool groupItems = isVectorField && hasObservedData;
                if (groupItems)
                {
                    QString headerText;
                    if (i == SUM_CASES)
                    {
                        headerText = QString("Simulated Data");
                    }
                    else if (i == OBS_DATA)
                    {
                        headerText = QString("Observed Data");
                    }
                    else if (i == CALCULATED_CURVES)
                    {
                        headerText = QString("Calculated");
                    }

                    if (!headerText.isEmpty())
                    {
                        options.push_back(caf::PdmOptionItemInfo::createHeader(headerText, true));
                    }
                }

                auto itemPostfix = (isVectorField && i == OBS_DATA) ? QString(OBSERVED_DATA_AVALUE_POSTFIX) : QString("");

                // Sort numeric identifiers by numeric val
                std::vector<std::string> itemNamesVector;
                {
                    switch (identifierAndField->summaryIdentifier())
                    {
                    case RifEclipseSummaryAddress::INPUT_REGION_NUMBER:
                    case RifEclipseSummaryAddress::INPUT_SEGMENT_NUMBER:
                    case RifEclipseSummaryAddress::INPUT_AQUIFER_NUMBER:
                    {
                        std::set<int> values;
                        for (const std::string& itemName : itemNames[i])
                        {
                            values.insert(RiaStdStringTools::toInt(itemName));
                        }
                        for (int v : values)
                        {
                            itemNamesVector.push_back(std::to_string(v));
                        }
                        break;
                    }
                    default:
                        itemNamesVector.insert(itemNamesVector.end(), itemNames[i].begin(), itemNames[i].end());
                        break;
                    }
                }

                for (const auto& itemName : itemNamesVector)
                {
                    QString displayName;

                    if (isVectorField)
                    {
                        std::string longVectorName = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(itemName, true);
                        displayName = QString::fromStdString(longVectorName);
                        displayName += QString(" (%1)").arg(QString::fromStdString(itemName));
                    }
                    else
                    {
                        displayName = QString::fromStdString(itemName);
                    }

                    auto optionItem = caf::PdmOptionItemInfo(displayName, QString::fromStdString(itemName) + itemPostfix);
                    if (groupItems)
                        optionItem.setLevel(1);
                    options.push_back(optionItem);
                }
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
    sourcesGroup->add(&m_selectedSources);

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroupWithKeyword("Summary Types", RiuSummaryCurveDefinitionKeywords::summaryTypes());
    itemTypesGroup->add(&m_selectedSummaryCategories);

    caf::PdmField<std::vector<QString>>* summaryiesField = nullptr;

    RifEclipseSummaryAddress::SummaryVarCategory sumCategory = m_currentSummaryCategory();
    if (sumCategory == RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        sumCategory = RifEclipseSummaryAddress::SUMMARY_FIELD;
    }

    if (sumCategory == RifEclipseSummaryAddress::SUMMARY_FIELD)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_FIELD][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_AQUIFER)
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup("Aquifers");
            myGroup->add(m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][0]->pdmField());
        }

        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_AQUIFER][1]->pdmField();
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
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_CALCULATED][0]->pdmField();
    }
    else if (sumCategory == RifEclipseSummaryAddress::SUMMARY_IMPORTED)
    {
        summaryiesField = m_identifierFieldsMap[RifEclipseSummaryAddress::SUMMARY_IMPORTED][0]->pdmField();
    }

    caf::PdmUiGroup* summariesGroup = uiOrdering.addNewGroupWithKeyword("Summaries", RiuSummaryCurveDefinitionKeywords::summaries());
    if (summaryiesField)
    {
        summariesGroup->add(summaryiesField);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddressesFromSelectedCases(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<SummarySource*> sources;
    for (const auto& source : m_selectedSources())
    {
        RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>(source.p());
        RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>(source.p());

        if (sumCase)
        {
            if(!isObservedData(sumCase)) sources.push_back(sumCase);
        }
        else if (ensemble)
        {
            sources.push_back(ensemble);
        }
    }
    return findPossibleSummaryAddresses(sources, identifierAndField);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddressesFromSelectedObservedData(const SummaryIdentifierAndField *identifierAndField)
{
    std::vector<SummarySource*> obsData;
    for (const auto& source : m_selectedSources())
    {
        RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>(source.p());

        if (sumCase && isObservedData(sumCase))
        {
            obsData.push_back(sumCase);
        }
    }
    return findPossibleSummaryAddresses(obsData, identifierAndField);
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryCurveDefSelection::findPossibleSummaryAddresses(const std::vector<SummarySource*> &selectedSources, 
                                                                                        const SummaryIdentifierAndField *identifierAndField)
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    auto isVectorField = identifierAndField != nullptr && identifierAndField->summaryIdentifier() == RifEclipseSummaryAddress::INPUT_VECTOR_NAME;
    auto controllingIdentifierAndField = identifierAndField != nullptr ? lookupControllingField(identifierAndField) : nullptr;
    if (!isVectorField && controllingIdentifierAndField != nullptr && controllingIdentifierAndField->pdmField()->v().size() == 0)
    {
        return addrUnion;
    }

    for (SummarySource* currSource : selectedSources)
    {
        std::set<RifEclipseSummaryAddress> allAddresses;

        RimSummaryCase* currCase = dynamic_cast<RimSummaryCase*>(currSource);
        RimSummaryCaseCollection* currEnsemble = dynamic_cast<RimSummaryCaseCollection*>(currSource);

        if (currCase)
        {
            RifSummaryReaderInterface* reader = currCase->summaryReader();
            if (reader) allAddresses = reader->allResultAddresses();
        }
        else if (currEnsemble)
        {
            allAddresses = currEnsemble->ensembleSummaryAddresses();
        }

        bool applySelections = identifierAndField == nullptr || (!isVectorField && controllingIdentifierAndField != nullptr);
        std::vector<SummaryIdentifierAndField*> controllingFields;
        if (applySelections)
        {
            // Build selections vector
            controllingFields = buildControllingFieldList(identifierAndField);
        }

        for(auto& address : allAddresses)
        {
            if (address.category() == m_currentSummaryCategory())
            {
                bool addressSelected = applySelections ? isAddressCompatibleWithControllingFieldSelection(address, controllingFields) : true;
                if (addressSelected)
                {
                    addrUnion.insert(address);
                }
            }
        }
    }
    return addrUnion;
}

//--------------------------------------------------------------------------------------------------
/// Build a list of relevant selections
//--------------------------------------------------------------------------------------------------
std::vector<SummaryIdentifierAndField*> RiuSummaryCurveDefSelection::buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField) const
{
    std::vector<SummaryIdentifierAndField*> controllingFields;
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
SummaryIdentifierAndField* RiuSummaryCurveDefSelection::lookupIdentifierAndFieldFromFieldHandle(const caf::PdmFieldHandle* pdmFieldHandle) const
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
/// If the specified pdm field info is the topmost (i.e. index is 0), nullptr is returned
//--------------------------------------------------------------------------------------------------
SummaryIdentifierAndField* RiuSummaryCurveDefSelection::lookupControllingField(const SummaryIdentifierAndField *dependentField) const
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
        if (m_identifierFieldsMap.at(category).size() == 0 ||
            category == RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_INVALID) continue;
        
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

        if (!identifierPath.empty())
        {
            identifierPath.pop_back();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*> (attribute);
    if (attrib)
    {
        if (&m_selectedSummaryCategories == field)
        {
            attrib->fieldToReceiveCurrentItemValue = &m_currentSummaryCategory;
            attrib->showTextFilter = false;
            attrib->showToggleAllCheckbox = false;
            attrib->setCurrentIndexWhenItemIsChecked = true;
        }

        // All tree selection editors are set in specified selection mode
        attrib->singleSelectionMode = !m_multiSelectionMode;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSummaryCurveDefSelection::resetAllFields()
{
    m_selectedSources.clear();
    m_selectedSummaryCategories = std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>();

    // clear all state in fields
    for (auto& identifierAndFieldList : m_identifierFieldsMap)
    {
        for (auto a : identifierAndFieldList.second)
        {
            a->pdmField()->v().clear();
        }
    }

    m_prevCurveCount = 0;
    m_prevCurveSetCount = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuSummaryCurveDefSelection::isObservedData(const RimSummaryCase *sumCase) const
{
    return dynamic_cast<const RimObservedData*>(sumCase) != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<SummarySource*> RiuSummaryCurveDefSelection::selectedSummarySources() const
{
    std::vector<SummarySource*> sources;

    for (const auto& source : m_selectedSources)
    {
        sources.push_back(source);
    }

    // Always add the summary case for calculated curves as this case is not displayed in UI
    sources.push_back(RiuSummaryCurveDefSelection::calculatedSummaryCase());

    return sources;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiuSummaryCurveDefSelection::calculatedSummaryCase()
{
    RimSummaryCalculationCollection* calcColl = RiaApplication::instance()->project()->calculationCollection();

    return calcColl->calculationSummaryCase();
}
