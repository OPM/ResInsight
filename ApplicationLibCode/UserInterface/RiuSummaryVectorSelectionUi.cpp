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

#include "RiuSummaryVectorSelectionUi.h"

#include "RiaCurveSetDefinition.h"
#include "RiaOptionItemFactory.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryDefines.h"

#include "RifEclipseSummaryAddress.h"
#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuSummaryCurveDefinitionKeywords.h"
#include "RiuSummaryQuantityNameInfoProvider.h"

#include "cafPdmPointer.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDebug>

#include <algorithm>

using namespace RifEclipseSummaryAddressDefines;

CAF_PDM_SOURCE_INIT( RiuSummaryVectorSelectionUi, "RicSummaryAddressSelection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SummaryIdentifierAndField
{
public:
    SummaryIdentifierAndField()
        : m_summaryIdentifier( (SummaryIdentifierType)0 )
        , m_pdmField( nullptr )
    {
    }

    SummaryIdentifierAndField( const SummaryIdentifierAndField& ) = delete;

    SummaryIdentifierAndField( SummaryIdentifierType summaryIdentifier )
        : m_summaryIdentifier( summaryIdentifier )
        , m_pdmField( new caf::PdmField<std::vector<QString>>() )
    {
    }

    virtual ~SummaryIdentifierAndField();

    SummaryIdentifierType                summaryIdentifier() const { return m_summaryIdentifier; }
    caf::PdmField<std::vector<QString>>* pdmField() { return m_pdmField; }

private:
    SummaryIdentifierType                m_summaryIdentifier;
    caf::PdmField<std::vector<QString>>* m_pdmField;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SummaryIdentifierAndField::~SummaryIdentifierAndField()
{
    delete m_pdmField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorSelectionUi::RiuSummaryVectorSelectionUi()
    : m_identifierFieldsMap( {
          { SummaryCategory::SUMMARY_FIELD,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_AQUIFER,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_AQUIFER_NUMBER ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_NETWORK,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_NETWORK_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_MISC,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_REGION,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_REGION_NUMBER ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_REGION_2_REGION,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_REGION_2_REGION ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_GROUP,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_GROUP_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_WELL,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_WELL_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_WELL_COMPLETION,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_WELL_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_CELL_IJK ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_WELL_COMPLETION_LGR,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_LGR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_WELL_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_CELL_IJK ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_WELL_LGR,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_LGR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_WELL_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_WELL_SEGMENT,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_WELL_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_SEGMENT_NUMBER ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_BLOCK,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_CELL_IJK ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_BLOCK_LGR,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_LGR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_CELL_IJK ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },
          { SummaryCategory::SUMMARY_IMPORTED,
            { new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_VECTOR_NAME ),
              new SummaryIdentifierAndField( SummaryIdentifierType::INPUT_ID ) } },

      } )
    , m_showIndividualEnsembleCases( true )
{
    CAF_PDM_InitFieldNoDefault( &m_selectedSources, "SummaryCases", "Cases" );
    m_selectedSources.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_currentSummaryCategory, "CurrentSummaryCategory", "Current Summary Category" );
    CAF_PDM_InitFieldNoDefault( &m_selectedSummaryCategories, "SelectedSummaryCategories", "Summary Categories" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_FIELD][0]->pdmField(), "FieldVectors", "Field vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_FIELD][1]->pdmField(), "FieldCalculationIds", "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_AQUIFER][0]->pdmField(), "Aquifers", "Aquifers" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_AQUIFER][1]->pdmField(), "AquiferVectors", "Aquifer Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_AQUIFER][2]->pdmField(),
                                "AquifierCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_NETWORK][0]->pdmField(), "NetworkNames", "Networks" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_NETWORK][1]->pdmField(), "NetworkVectors", "Network Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_NETWORK][2]->pdmField(),
                                "NetworkCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_MISC][0]->pdmField(), "MiscVectors", "Misc Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_MISC][1]->pdmField(), "MiscCalculationIds", "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION][0]->pdmField(), "Regions", "Regions" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION][1]->pdmField(), "RegionsVectors", "Regions Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION][2]->pdmField(), "RegionCalculationIds", "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION_2_REGION][0]->pdmField(),
                                "Region2RegionRegions",
                                "Regions" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION_2_REGION][1]->pdmField(),
                                "Region2RegionVectors",
                                "Region2s Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION_2_REGION][2]->pdmField(),
                                "Region2RegionCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_GROUP][0]->pdmField(), "WellGroupWellGroupNames", "Groups" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_GROUP][1]->pdmField(), "WellGroupVectors", "Well Group Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_GROUP][2]->pdmField(),
                                "WellGroupCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL][0]->pdmField(), "WellWellName", "Wells" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL][1]->pdmField(), "WellVectors", "Well Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL][2]->pdmField(), "WellCalculationIds", "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][0]->pdmField(),
                                "WellCompletionWellName",
                                "Wells" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][1]->pdmField(), "WellCompletionIjk", "Cell IJK" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][2]->pdmField(),
                                "WellCompletionVectors",
                                "Well Completion Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][3]->pdmField(),
                                "WellCompletionCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField(),
                                "WellCompletionLgrLgrName",
                                "LGR Names" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField(),
                                "WellCompletionLgrWellName",
                                "Wells" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField(),
                                "WellCompletionLgrIjk",
                                "Cell IJK" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField(),
                                "WellCompletionLgrVectors",
                                "Well Completion Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][4]->pdmField(),
                                "WellCompletionLgrCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][0]->pdmField(), "WellLgrLgrName", "LGR Names" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][1]->pdmField(), "WellLgrWellName", "Wells" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][2]->pdmField(), "WellLgrVectors", "Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][3]->pdmField(),
                                "WellLgrCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][0]->pdmField(), "WellSegmentWellName", "Wells" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][1]->pdmField(), "WellSegmentNumber", "Segments" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][2]->pdmField(), "WellSegmentVectors", "Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][3]->pdmField(),
                                "WellSegmentCalculationIds",
                                "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK][0]->pdmField(), "BlockIjk", "Cell IJK" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK][1]->pdmField(), "BlockVectors", "Block Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK][2]->pdmField(), "BlockCalculationIds", "Calculation Ids" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][0]->pdmField(), "BlockLgrLgrName", "LGR Names" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][1]->pdmField(), "BlockLgrIjk", "Cell IJK" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][2]->pdmField(), "BlockLgrVectors", "Block Vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][3]->pdmField(),
                                "BlockLgrCalculationIds",
                                "CalculationIds" );

    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_IMPORTED][0]->pdmField(), "ImportedVectors", "Imported vectors" );
    CAF_PDM_InitFieldNoDefault( m_identifierFieldsMap[SummaryCategory::SUMMARY_IMPORTED][1]->pdmField(),
                                "ImportedCalculationIds",
                                "Calculation Ids" );

    for ( const auto& itemTypes : m_identifierFieldsMap )
    {
        for ( const auto& itemInputType : itemTypes.second )
        {
            itemInputType->pdmField()->uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

            itemInputType->pdmField()->uiCapability()->setUiLabelPosition( itemTypes.second.size() > 2 ? caf::PdmUiItemInfo::TOP
                                                                                                       : caf::PdmUiItemInfo::HIDDEN );

            itemInputType->pdmField()->uiCapability()->setAutoAddingOptionFromValue( false );
        }
        itemTypes.second.back()->pdmField()->uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    }

    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_selectedSummaryCategories.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedSummaryCategories.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_currentSummaryCategory.uiCapability()->setUiHidden( true );
    m_multiSelectionMode = false;
    m_hideEnsembles      = false;
    m_hideSummaryCases   = false;

    m_hideDifferenceVectors                 = false;
    m_hideHistoryVectors                    = false;
    m_hideVectorsWithoutHistory             = false;
    m_hideCalculationIncompatibleCategories = false;

    m_prevCurveCount    = 0;
    m_prevCurveSetCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryVectorSelectionUi::~RiuSummaryVectorSelectionUi()
{
    for ( const auto& identifierAndFieldList : m_identifierFieldsMap )
    {
        for ( const auto& identifierAndField : identifierAndFieldList.second )
        {
            delete identifierAndField;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryVectorSelectionUi::allCurveDefinitionsFromSelection() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefVector;

    {
        std::set<RiaSummaryCurveDefinition> curveDefinitions;
        std::set<RifEclipseSummaryAddress>  selectedAddressesFromUi = buildAddressListFromSelections();

        for ( SummarySource* currSource : selectedSummarySources() )
        {
            RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>( currSource );
            if ( sumCase == nullptr ) continue;

            std::set<RifEclipseSummaryAddress> addressesFromSource;
            std::vector<RimSummaryCase*>       casesFromSource;

            RifSummaryReaderInterface* reader = sumCase ? sumCase->summaryReader() : nullptr;
            if ( reader )
            {
                addressesFromSource.insert( reader->allResultAddresses().begin(), reader->allResultAddresses().end() );
                casesFromSource.push_back( sumCase );
            }

            for ( auto caseFromSource : casesFromSource )
            {
                for ( const auto& addressFromSource : addressesFromSource )
                {
                    if ( selectedAddressesFromUi.count( addressFromSource ) > 0 )
                    {
                        curveDefinitions.insert( RiaSummaryCurveDefinition( caseFromSource, addressFromSource, false ) );
                    }
                }
            }
        }

        std::copy( curveDefinitions.begin(), curveDefinitions.end(), std::back_inserter( curveDefVector ) );
    }

    return curveDefVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaCurveSetDefinition> RiuSummaryVectorSelectionUi::allCurveSetDefinitionsFromSelections() const
{
    std::vector<RiaCurveSetDefinition> curveSetDefVector;
    std::set<RiaCurveSetDefinition>    curveSetDefinitions;
    std::set<RifEclipseSummaryAddress> selectedAddressesFromUi = buildAddressListFromSelections();

    for ( SummarySource* currSource : selectedSummarySources() )
    {
        RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>( currSource );
        if ( !ensemble ) continue;

        std::set<RifEclipseSummaryAddress> addressesFromSource;

        // Build case list
        auto addresses = ensemble->ensembleSummaryAddresses();
        addressesFromSource.insert( addresses.begin(), addresses.end() );

        for ( const auto& addressFromSource : addressesFromSource )
        {
            if ( selectedAddressesFromUi.count( addressFromSource ) > 0 )
            {
                curveSetDefinitions.insert( RiaCurveSetDefinition( ensemble, addressFromSource ) );
            }
        }
    }

    std::copy( curveSetDefinitions.begin(), curveSetDefinitions.end(), std::back_inserter( curveSetDefVector ) );
    return curveSetDefVector;
}

//--------------------------------------------------------------------------------------------------
/// One CurveDefinition pr ensemble curve set,
/// but if enableIndividualEnsembleCaseSelection has been enabled,
/// the ensembles are never available for direct selection, and this method will always return the
/// "expanded" set of curvedefinitions
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RiuSummaryVectorSelectionUi::selection() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefSelection;
    std::set<RifEclipseSummaryAddress>     selectedAddressesFromUi = buildAddressListFromSelections();
    for ( SummarySource* currSource : selectedSummarySources() )
    {
        RimSummaryCaseCollection* ensemble   = dynamic_cast<RimSummaryCaseCollection*>( currSource );
        RimSummaryCase*           sourceCase = dynamic_cast<RimSummaryCase*>( currSource );

        if ( ensemble )
        {
            std::set<RifEclipseSummaryAddress> addressUnion = ensemble->ensembleSummaryAddresses();
            for ( const auto& addr : selectedAddressesFromUi )
            {
                if ( addressUnion.count( addr ) )
                {
                    curveDefSelection.push_back( RiaSummaryCurveDefinition( ensemble, addr ) );
                }
            }
        }
        else
        {
            if ( !( sourceCase && sourceCase->summaryReader() ) ) continue;

            const std::set<RifEclipseSummaryAddress>& readerAddresses = sourceCase->summaryReader()->allResultAddresses();
            for ( const auto& addr : selectedAddressesFromUi )
            {
                if ( readerAddresses.count( addr ) )
                {
                    curveDefSelection.push_back( RiaSummaryCurveDefinition( sourceCase, addr, false ) );
                }
            }
        }
    }

    return curveDefSelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::setMultiSelectionMode( bool multiSelectionMode )
{
    m_multiSelectionMode = multiSelectionMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideEnsembles( bool hide )
{
    m_hideEnsembles = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideSummaryCases( bool hide )
{
    m_hideSummaryCases = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::enableIndividualEnsembleCaseSelection( bool enable )
{
    m_showIndividualEnsembleCases = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideDifferenceVectors( bool hide )
{
    m_hideDifferenceVectors = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideHistoryVectors( bool hide )
{
    m_hideHistoryVectors = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideVectorsWithoutHistory( bool hide )
{
    m_hideVectorsWithoutHistory = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::hideCalculationIncompatibleCategories( bool hide )
{
    m_hideCalculationIncompatibleCategories = hide;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::setFieldChangedHandler( const std::function<void()>& handlerFunc )
{
    m_toggleChangedHandler = handlerFunc;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::setDefaultSelection( const std::vector<SummarySource*>& defaultSources )
{
    RimProject* proj        = RimProject::current();
    auto        allSumCases = proj->allSummaryCases();

    if ( !allSumCases.empty() )
    {
        RifEclipseSummaryAddress defaultAddress;

        std::vector<SummarySource*> selectTheseSources = defaultSources;
        if ( selectTheseSources.empty() ) selectTheseSources.push_back( allSumCases[0] );

        std::vector<RiaSummaryCurveDefinition> curveDefs;
        for ( SummarySource* s : selectTheseSources )
        {
            RimSummaryCase*           sumCase  = dynamic_cast<RimSummaryCase*>( s );
            RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>( s );
            if ( ensemble )
            {
                curveDefs.push_back( RiaSummaryCurveDefinition( ensemble, defaultAddress ) );
            }
            else
            {
                curveDefs.push_back( RiaSummaryCurveDefinition( sumCase, defaultAddress, false ) );
            }
        }

        setSelectedCurveDefinitions( curveDefs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RiuSummaryVectorSelectionUi::optionsForSummaryDataSource( bool hideSummaryCases, bool hideEnsembles, bool showIndividualEnsembleCases )
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject*               proj      = RimProject::current();
    std::vector<RimOilField*> oilFields = proj->allOilFields();
    for ( RimOilField* oilField : oilFields )
    {
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if ( sumCaseMainColl )
        {
            if ( !hideSummaryCases )
            {
                // Top level cases
                for ( const auto& sumCase : sumCaseMainColl->topLevelSummaryCases() )
                {
                    options.push_back( caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase ) );
                }
            }

            // Ensembles
            if ( !hideEnsembles )
            {
                bool ensembleHeaderCreated = false;
                for ( const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections() )
                {
                    if ( !sumCaseColl->isEnsemble() ) continue;

                    if ( !ensembleHeaderCreated )
                    {
                        options.push_back( caf::PdmOptionItemInfo::createHeader( "Ensembles", true ) );
                        ensembleHeaderCreated = true;
                    }
                    // Ensemble level
                    {
                        auto optionItem = caf::PdmOptionItemInfo( sumCaseColl->name(), sumCaseColl );
                        optionItem.setLevel( 1 );
                        options.push_back( optionItem );
                    }
                }
            }

            if ( !hideSummaryCases )
            {
                // Grouped cases
                for ( const auto& sumCaseColl : sumCaseMainColl->summaryCaseCollections() )
                {
                    if ( sumCaseColl->isEnsemble() && !showIndividualEnsembleCases ) continue;

                    options.push_back( caf::PdmOptionItemInfo::createHeader( sumCaseColl->name(), true ) );

                    for ( const auto& sumCase : sumCaseColl->allSummaryCases() )
                    {
                        auto optionItem = caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase );
                        optionItem.setLevel( 1 );
                        options.push_back( optionItem );
                    }
                }

                // Observed data
                auto observedDataColl = oilField->observedDataCollection();
                if ( !observedDataColl->allObservedSummaryData().empty() )
                {
                    options.push_back( caf::PdmOptionItemInfo::createHeader( "Observed Data", true ) );

                    for ( const auto& obsData : observedDataColl->allObservedSummaryData() )
                    {
                        auto optionItem = caf::PdmOptionItemInfo( obsData->caseName(), obsData );
                        optionItem.setLevel( 1 );
                        options.push_back( optionItem );
                    }
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::setSelectedCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions )
{
    resetAllFields();

    std::set<SummaryCategory> categories;
    for ( const auto& curveDef : curveDefinitions )
    {
        if ( !( curveDef.summaryCaseY() || curveDef.isEnsembleCurve() ) ) continue;

        RimSummaryCase* summaryCase = curveDef.summaryCaseY();

        RifEclipseSummaryAddress summaryAddress = curveDef.summaryAddressY();

        // Ignore ensemble statistics curves
        if ( summaryAddress.category() == SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS ) continue;

        // Select summary category if not already selected
        auto& selectedCategories = m_selectedSummaryCategories();

        if ( std::find( selectedCategories.begin(), selectedCategories.end(), summaryAddress.category() ) == selectedCategories.end() )
        {
            if ( summaryAddress.category() != SummaryCategory::SUMMARY_INVALID )
            {
                m_selectedSummaryCategories.v().push_back( summaryAddress.category() );
            }
            else
            {
                // Use field category as fall back category to avoid an empty list of vectors
                summaryAddress = RifEclipseSummaryAddress::fieldAddress( "" );
            }
        }

        // Select case if not already selected
        if ( curveDef.isEnsembleCurve() )
        {
            if ( curveDef.ensemble() && !m_hideEnsembles )
            {
                if ( std::find( m_selectedSources.begin(), m_selectedSources.end(), curveDef.ensemble() ) == m_selectedSources.end() )
                {
                    m_selectedSources.push_back( curveDef.ensemble() );
                }
            }
        }
        else
        {
            if ( curveDef.summaryCaseY() && ( !curveDef.ensemble() || m_showIndividualEnsembleCases ) )
            {
                if ( std::find( m_selectedSources.begin(), m_selectedSources.end(), curveDef.summaryCaseY() ) == m_selectedSources.end() )
                {
                    m_selectedSources.push_back( curveDef.summaryCaseY() );
                }
            }
        }

        bool isObservedDataCase = isObservedData( summaryCase );

        auto identifierAndFieldList = m_identifierFieldsMap[summaryAddress.category()];
        for ( const auto& identifierAndField : identifierAndFieldList )
        {
            bool    isVectorField = identifierAndField->summaryIdentifier() == SummaryIdentifierType::INPUT_VECTOR_NAME;
            QString avalue = QString::fromStdString( summaryAddress.addressComponentUiText( identifierAndField->summaryIdentifier() ) );
            if ( isVectorField && isObservedDataCase )
            {
                avalue = avalue + QString( OBSERVED_DATA_AVALUE_POSTFIX );
            }

            if ( isVectorField && summaryAddress.isCalculated() )
            {
                // Append calculation id to input vector name calculated data.
                avalue = avalue + QString( ":%1" ).arg( summaryAddress.id() );
            }

            const auto& currentSelectionVector = identifierAndField->pdmField()->v();
            if ( std::find( currentSelectionVector.begin(), currentSelectionVector.end(), avalue ) == currentSelectionVector.end() )
            {
                std::vector<QString> newSelectionVector( currentSelectionVector.begin(), currentSelectionVector.end() );
                newSelectionVector.push_back( avalue );
                ( *identifierAndField->pdmField() ) = newSelectionVector;
            }
        }

        categories.insert( curveDef.summaryAddressY().category() );
    }

    if ( !categories.empty() )
    {
        SummaryCategory cat = *( categories.begin() );
        m_currentSummaryCategory.setValue( cat );
    }

    m_prevCurveCount    = allCurveDefinitionsFromSelection().size();
    m_prevCurveSetCount = allCurveSetDefinitionsFromSelections().size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField != &m_selectedSources && changedField != &m_selectedSummaryCategories && changedField != &m_currentSummaryCategory )
    {
        SummaryCategory currentCategory = m_currentSummaryCategory();
        if ( currentCategory != SummaryCategory::SUMMARY_INVALID )
        {
            // When a summary vector is selected, make sure the summary category for the summary vector is in the
            // selection Note that we use the size of the variant to avoid this operation when an item in unchecked

            if ( newValue.toList().size() > oldValue.toList().size() )
            {
                if ( std::find( m_selectedSummaryCategories.v().begin(), m_selectedSummaryCategories.v().end(), currentCategory ) ==
                     m_selectedSummaryCategories.v().end() )
                {
                    m_selectedSummaryCategories.v().push_back( currentCategory );
                }
            }

            if ( !newValue.toList().empty() )
            {
                auto identifierAndFieldList = m_identifierFieldsMap[currentCategory];
                for ( const auto identifierAndField : identifierAndFieldList )
                {
                    if ( identifierAndField->summaryIdentifier() == SummaryIdentifierType::INPUT_VECTOR_NAME ) continue;

                    auto v = identifierAndField->pdmField()->value();
                    if ( v.empty() )
                    {
                        auto field = identifierAndField->pdmField();

                        QList<caf::PdmOptionItemInfo> options;
                        appendOptionItemsForSubCategoriesAndVectors( options, identifierAndField );

                        if ( !options.isEmpty() )
                        {
                            std::vector<QString> values;

                            auto    firstOption = options.front();
                            auto    firstValue  = firstOption.value();
                            QString valueText   = firstValue.toString();

                            values.push_back( valueText );
                            field->setValue( values );
                        }
                    }
                }
            }
        }
    }

    size_t curveCount    = allCurveDefinitionsFromSelection().size();
    size_t curveSetCount = allCurveSetDefinitionsFromSelections().size();

    if ( m_toggleChangedHandler != nullptr )
    {
        // Do nothing if the curve count and curve set count is identical
        if ( !m_multiSelectionMode || ( curveCount != m_prevCurveCount || curveSetCount != m_prevCurveSetCount ) )
        {
            m_toggleChangedHandler();

            m_prevCurveCount    = curveCount;
            m_prevCurveSetCount = curveSetCount;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RiuSummaryVectorSelectionUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedSources )
    {
        appendOptionItemsForSources( options );
    }
    else if ( fieldNeedingOptions == &m_selectedSummaryCategories )
    {
        appendOptionItemsForCategories( options );
    }
    else
    {
        // Lookup item type input field
        auto identifierAndField = lookupIdentifierAndFieldFromFieldHandle( fieldNeedingOptions );
        appendOptionItemsForSubCategoriesAndVectors( options, identifierAndField );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* sourcesGroup = uiOrdering.addNewGroupWithKeyword( "Sources", RiuSummaryCurveDefinitionKeywords::sources() );
    sourcesGroup->add( &m_selectedSources );

    caf::PdmUiGroup* itemTypesGroup = uiOrdering.addNewGroupWithKeyword( "Summary Types", RiuSummaryCurveDefinitionKeywords::summaryTypes() );
    itemTypesGroup->add( &m_selectedSummaryCategories );

    caf::PdmField<std::vector<QString>>* summaryiesField = nullptr;

    SummaryCategory sumCategory = m_currentSummaryCategory();
    if ( sumCategory == SummaryCategory::SUMMARY_INVALID )
    {
        sumCategory = SummaryCategory::SUMMARY_FIELD;
    }

    if ( sumCategory == SummaryCategory::SUMMARY_FIELD )
    {
        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_FIELD][0]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_AQUIFER )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryAquifer() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_AQUIFER][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_AQUIFER][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_NETWORK )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryNetwork() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_NETWORK][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_NETWORK][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_MISC )
    {
        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_MISC][0]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_REGION )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryRegion() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_REGION_2_REGION )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( "Regions" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION_2_REGION][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_REGION_2_REGION][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_GROUP )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryWellGroup() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_GROUP][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_GROUP][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_WELL )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryWell() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_WELL_COMPLETION )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryCompletion() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][0]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][1]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION][2]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_WELL_COMPLETION_LGR )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryLgrCompletion() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][0]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][1]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][2]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_COMPLETION_LGR][3]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_WELL_LGR )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryLgrWell() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][0]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][1]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_LGR][2]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_WELL_SEGMENT )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryWellSegment() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][0]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][1]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_WELL_SEGMENT][2]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_BLOCK )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryBlock() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK][0]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK][1]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_BLOCK_LGR )
    {
        {
            caf::PdmUiGroup* myGroup = uiOrdering.addNewGroup( RiaDefines::summaryLgrBlock() + "s" );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][0]->pdmField() );
            myGroup->add( m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][1]->pdmField() );
        }

        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_BLOCK_LGR][2]->pdmField();
    }
    else if ( sumCategory == SummaryCategory::SUMMARY_IMPORTED )
    {
        summaryiesField = m_identifierFieldsMap[SummaryCategory::SUMMARY_IMPORTED][0]->pdmField();
    }

    caf::PdmUiGroup* summariesGroup = uiOrdering.addNewGroupWithKeyword( "Summaries", RiuSummaryCurveDefinitionKeywords::summaries() );
    if ( summaryiesField )
    {
        summariesGroup->add( summaryiesField );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RiuSummaryVectorSelectionUi::findPossibleSummaryAddressesFromSelectedCases( const SummaryIdentifierAndField* identifierAndField ) const
{
    std::vector<SummarySource*> sources;
    for ( const auto& source : m_selectedSources.value() )
    {
        RimSummaryCase*           sumCase  = dynamic_cast<RimSummaryCase*>( source.p() );
        RimSummaryCaseCollection* ensemble = dynamic_cast<RimSummaryCaseCollection*>( source.p() );

        if ( sumCase )
        {
            if ( !isObservedData( sumCase ) )
            {
                sources.push_back( sumCase );
            }
        }
        else if ( ensemble )
        {
            sources.push_back( ensemble );
        }
    }
    return findPossibleSummaryAddresses( sources, identifierAndField );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RiuSummaryVectorSelectionUi::findPossibleSummaryAddressesFromSelectedObservedData( const SummaryIdentifierAndField* identifierAndField ) const
{
    std::vector<SummarySource*> obsData;
    for ( const auto& source : m_selectedSources.value() )
    {
        RimSummaryCase* sumCase = dynamic_cast<RimSummaryCase*>( source.p() );

        if ( sumCase && isObservedData( sumCase ) )
        {
            obsData.push_back( sumCase );
        }
    }
    return findPossibleSummaryAddresses( obsData, identifierAndField );
}

//--------------------------------------------------------------------------------------------------
/// Returns the summary addresses that match the selected item type and input selections made in GUI
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RiuSummaryVectorSelectionUi::findPossibleSummaryAddresses( const std::vector<SummarySource*>& selectedSources,
                                                               const SummaryIdentifierAndField*   identifierAndField ) const
{
    std::set<RifEclipseSummaryAddress> addrUnion;

    auto isVectorField = identifierAndField != nullptr && identifierAndField->summaryIdentifier() == SummaryIdentifierType::INPUT_VECTOR_NAME;
    auto controllingIdentifierAndField = identifierAndField != nullptr ? lookupControllingField( identifierAndField ) : nullptr;
    if ( !isVectorField && controllingIdentifierAndField != nullptr && controllingIdentifierAndField->pdmField()->v().empty() )
    {
        return addrUnion;
    }

    for ( SummarySource* currSource : selectedSources )
    {
        std::set<RifEclipseSummaryAddress> allAddresses;

        RimSummaryCase*           currCase     = dynamic_cast<RimSummaryCase*>( currSource );
        RimSummaryCaseCollection* currEnsemble = dynamic_cast<RimSummaryCaseCollection*>( currSource );

        if ( currCase )
        {
            RifSummaryReaderInterface* reader = currCase->summaryReader();
            if ( reader ) allAddresses = reader->allResultAddresses();
        }
        else if ( currEnsemble )
        {
            allAddresses = currEnsemble->ensembleSummaryAddresses();
        }

        bool applySelections = identifierAndField == nullptr || ( !isVectorField && controllingIdentifierAndField != nullptr );
        std::vector<SummaryIdentifierAndField*> controllingFields;
        if ( applySelections )
        {
            // Build selections vector
            controllingFields = buildControllingFieldList( identifierAndField );
        }

        for ( auto& address : allAddresses )
        {
            if ( address.category() == m_currentSummaryCategory() )
            {
                bool addressSelected = applySelections ? isAddressCompatibleWithControllingFieldSelection( address, controllingFields ) : true;
                if ( addressSelected )
                {
                    addrUnion.insert( address );
                }
            }
        }
    }

    if ( m_hideHistoryVectors || m_hideDifferenceVectors )
    {
        std::set<RifEclipseSummaryAddress> filteredAddresses;

        for ( const auto& adr : addrUnion )
        {
            if ( m_hideHistoryVectors && adr.isHistoryVector() ) continue;

            if ( m_hideDifferenceVectors )
            {
                const auto diffText = RifEclipseSummaryAddressDefines::differenceIdentifier();
                if ( RiaStdStringTools::endsWith( adr.vectorName(), diffText ) ) continue;
            }

            if ( m_hideVectorsWithoutHistory )
            {
                auto candidateName = adr.vectorName() + RifEclipseSummaryAddressDefines::historyIdentifier();

                bool found = false;
                for ( const auto& ad : addrUnion )
                {
                    if ( ad.vectorName() == candidateName ) found = true;
                }

                if ( !found ) continue;
            }

            filteredAddresses.insert( adr );
        }

        addrUnion.swap( filteredAddresses );
    }

    return addrUnion;
}

//--------------------------------------------------------------------------------------------------
/// Build a list of relevant selections
//--------------------------------------------------------------------------------------------------
std::vector<SummaryIdentifierAndField*>
    RiuSummaryVectorSelectionUi::buildControllingFieldList( const SummaryIdentifierAndField* identifierAndField ) const
{
    std::vector<SummaryIdentifierAndField*> controllingFields;
    const auto&                             identifierAndFieldList = m_identifierFieldsMap.at( m_currentSummaryCategory() );
    for ( const auto& identifierAndFieldItem : identifierAndFieldList )
    {
        if ( identifierAndFieldItem == identifierAndField )
        {
            break;
        }
        controllingFields.push_back( identifierAndFieldItem );
    }
    return controllingFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SummaryIdentifierAndField* RiuSummaryVectorSelectionUi::lookupIdentifierAndFieldFromFieldHandle( const caf::PdmFieldHandle* pdmFieldHandle ) const
{
    for ( const auto& itemTypes : m_identifierFieldsMap )
    {
        for ( const auto& itemTypeInput : itemTypes.second )
        {
            if ( pdmFieldHandle == itemTypeInput->pdmField() )
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
SummaryIdentifierAndField* RiuSummaryVectorSelectionUi::lookupControllingField( const SummaryIdentifierAndField* dependentField ) const
{
    for ( const auto& identifierAndFieldList : m_identifierFieldsMap )
    {
        int index = 0;
        for ( const auto& iaf : identifierAndFieldList.second )
        {
            if ( iaf == dependentField )
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
bool RiuSummaryVectorSelectionUi::isAddressCompatibleWithControllingFieldSelection(
    const RifEclipseSummaryAddress&                address,
    const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList ) const
{
    for ( const auto& identifierAndField : identifierAndFieldList )
    {
        bool match = false;
        for ( const auto& selectedText : identifierAndField->pdmField()->v() )
        {
            if ( QString::compare( QString::fromStdString( address.addressComponentUiText( identifierAndField->summaryIdentifier() ) ),
                                   selectedText ) == 0 )
            {
                match = true;
                break;
            }
        }

        if ( !match )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiuSummaryVectorSelectionUi::buildAddressListFromSelections() const
{
    std::set<RifEclipseSummaryAddress> addressSet;
    for ( const auto& category : m_selectedSummaryCategories() )
    {
        if ( m_identifierFieldsMap.at( category ).empty() || category == SummaryCategory::SUMMARY_INVALID ) continue;

        const auto&                                            identifierAndFieldList = m_identifierFieldsMap.at( category );
        std::vector<std::pair<SummaryIdentifierType, QString>> selectionStack;
        buildAddressListForCategoryRecursively( category, identifierAndFieldList.begin(), selectionStack, addressSet );
    }
    return addressSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::buildAddressListForCategoryRecursively(
    SummaryCategory                                         category,
    std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr,
    std::vector<std::pair<SummaryIdentifierType, QString>>& identifierPath,
    std::set<RifEclipseSummaryAddress>&                     addressSet ) const

{
    for ( const auto& identifierText : ( *identifierAndFieldItr )->pdmField()->v() )
    {
        auto idText = identifierText;

        // Calculated results have a id appended. E.g. "Calculation_3 ( NORNE_ATW2013, WOPR:B-4H ):3"
        if ( ( *identifierAndFieldItr )->summaryIdentifier() == SummaryIdentifierType::INPUT_VECTOR_NAME )
        {
            // Extract the calculation id
            QStringList tokens = idText.split( ":" );
            if ( tokens.size() > 1 )
            {
                QString calculationId = tokens.last();

                // Put the input vector name back together
                tokens.pop_back();
                idText = tokens.join( ":" );

                identifierPath.push_back( std::make_pair( SummaryIdentifierType::INPUT_ID, calculationId ) );
            }
        }

        idText.remove( OBSERVED_DATA_AVALUE_POSTFIX );
        identifierPath.push_back( std::make_pair( ( *identifierAndFieldItr )->summaryIdentifier(), idText ) );

        if ( ( *identifierAndFieldItr )->summaryIdentifier() != SummaryIdentifierType::INPUT_VECTOR_NAME )
        {
            buildAddressListForCategoryRecursively( category, std::next( identifierAndFieldItr, 1 ), identifierPath, addressSet );
        }
        else
        {
            std::map<SummaryIdentifierType, std::string> selectedIdentifiers;
            for ( const auto& identifier : identifierPath )
            {
                selectedIdentifiers.insert( std::make_pair( identifier.first, identifier.second.toStdString() ) );
            }
            auto address = RifEclipseSummaryAddress( category, selectedIdentifiers );
            addressSet.insert( address );
        }

        if ( !identifierPath.empty() )
        {
            identifierPath.pop_back();

            if ( !identifierPath.empty() && identifierPath.back().first == SummaryIdentifierType::INPUT_ID )
            {
                // If the last identifier is an id, remove it as we get two ids for calculated results. One entry for the input vector name
                // and one for the calculation id
                identifierPath.pop_back();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTreeSelectionEditorAttribute* attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
    if ( attrib )
    {
        if ( &m_selectedSummaryCategories == field )
        {
            attrib->currentIndexFieldHandle          = &m_currentSummaryCategory;
            attrib->showTextFilter                   = false;
            attrib->showToggleAllCheckbox            = false;
            attrib->setCurrentIndexWhenItemIsChecked = true;
        }

        // All tree selection editors are set in specified selection mode
        attrib->singleSelectionMode = !m_multiSelectionMode;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::resetAllFields()
{
    m_selectedSources.clearWithoutDelete();
    m_selectedSummaryCategories = std::vector<caf::AppEnum<SummaryCategory>>();

    // clear all state in fields
    for ( auto& identifierAndFieldList : m_identifierFieldsMap )
    {
        for ( auto a : identifierAndFieldList.second )
        {
            a->pdmField()->v().clear();
        }
    }

    m_prevCurveCount    = 0;
    m_prevCurveSetCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuSummaryVectorSelectionUi::isObservedData( const RimSummaryCase* sumCase ) const
{
    return dynamic_cast<const RimObservedSummaryData*>( sumCase ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SummarySource*> RiuSummaryVectorSelectionUi::selectedSummarySources() const
{
    std::vector<SummarySource*> sources;

    for ( const auto& source : m_selectedSources )
    {
        sources.push_back( source );
    }

    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::appendOptionItemsForSources( QList<caf::PdmOptionItemInfo>& options ) const
{
    auto dataSourceOptions = optionsForSummaryDataSource( m_hideSummaryCases, m_hideEnsembles, m_showIndividualEnsembleCases );

    for ( auto& o : dataSourceOptions )
    {
        options.push_back( o );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::appendOptionItemsForCategories( QList<caf::PdmOptionItemInfo>& options ) const
{
    std::vector<SummaryCategory> sortedCategoriesForUi;

    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_FIELD );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_AQUIFER );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_NETWORK );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_MISC );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_REGION );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_REGION_2_REGION );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_GROUP );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_WELL );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_WELL_COMPLETION );
    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_BLOCK );

    if ( !m_hideCalculationIncompatibleCategories )
    {
        sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_WELL_SEGMENT );
        sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_WELL_LGR );
        sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_WELL_COMPLETION_LGR );
        sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_BLOCK_LGR );
    }

    sortedCategoriesForUi.push_back( SummaryCategory::SUMMARY_IMPORTED );

    // NB SUMMARY_ENSEMBLE_STATISTICS is intentionally excluded
    // categoriesForUiDisplay.push_back(SummaryVarCategory::SUMMARY_ENSEMBLE_STATISTICS);

    for ( auto category : sortedCategoriesForUi )
    {
        auto option = RiaOptionItemFactory::optionItemFromSummaryType( category );

        options.push_back( option );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryVectorSelectionUi::appendOptionItemsForSubCategoriesAndVectors( QList<caf::PdmOptionItemInfo>& options,
                                                                               SummaryIdentifierAndField*     identifierAndField ) const
{
    if ( identifierAndField == nullptr ) return;

    enum
    {
        SUM_CASES,
        OBS_DATA,
    };

    const int itemCount = OBS_DATA + 1;

    std::set<RifEclipseSummaryAddress> addrUnion[itemCount];
    addrUnion[SUM_CASES] = findPossibleSummaryAddressesFromSelectedCases( identifierAndField );
    addrUnion[OBS_DATA]  = findPossibleSummaryAddressesFromSelectedObservedData( identifierAndField );

    bool isVectorField = identifierAndField->summaryIdentifier() == SummaryIdentifierType::INPUT_VECTOR_NAME;

    std::set<std::string> itemNames[itemCount];
    for ( int i = 0; i < itemCount; i++ )
    {
        for ( const auto& address : addrUnion[i] )
        {
            if ( address.isErrorResult() ) continue;

            auto name = address.addressComponentUiText( identifierAndField->summaryIdentifier() );
            if ( !name.empty() )
            {
                if ( isVectorField && address.isCalculated() )
                {
                    name += QString( ":%1" ).arg( address.id() ).toStdString();
                }
                itemNames[i].insert( name );
            }
        }
    }

    // Merge sets for all other fields than vector fields
    if ( !isVectorField )
    {
        itemNames[SUM_CASES].insert( itemNames[OBS_DATA].begin(), itemNames[OBS_DATA].end() );
        itemNames[OBS_DATA].clear();
    }

    for ( int i = 0; i < itemCount; i++ )
    {
        // Create headers only for vector fields when observed data is selected
        bool hasObservedData = !itemNames[OBS_DATA].empty();
        bool groupItems      = isVectorField && hasObservedData;
        if ( groupItems )
        {
            QString headerText;
            if ( i == SUM_CASES )
            {
                headerText = QString( "Simulated Data" );
            }
            else if ( i == OBS_DATA )
            {
                headerText = QString( "Observed Data" );
            }

            if ( !headerText.isEmpty() )
            {
                options.push_back( caf::PdmOptionItemInfo::createHeader( headerText, true ) );
            }
        }

        auto itemPostfix = ( isVectorField && i == OBS_DATA ) ? QString( OBSERVED_DATA_AVALUE_POSTFIX ) : QString( "" );

        // Sort numeric identifiers by numeric val
        std::vector<std::string> itemNamesVector;
        {
            switch ( identifierAndField->summaryIdentifier() )
            {
                case SummaryIdentifierType::INPUT_REGION_NUMBER:
                case SummaryIdentifierType::INPUT_SEGMENT_NUMBER:
                case SummaryIdentifierType::INPUT_AQUIFER_NUMBER:
                {
                    std::set<int> values;
                    for ( const std::string& itemName : itemNames[i] )
                    {
                        values.insert( RiaStdStringTools::toInt( itemName ) );
                    }
                    for ( int v : values )
                    {
                        itemNamesVector.push_back( std::to_string( v ) );
                    }
                    break;
                }
                default:
                    itemNamesVector.insert( itemNamesVector.end(), itemNames[i].begin(), itemNames[i].end() );
                    break;
            }
        }

        for ( const auto& itemName : itemNamesVector )
        {
            QString displayName;

            if ( isVectorField )
            {
                std::string longVectorName = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromVectorName( itemName );

                if ( longVectorName.empty() )
                {
                    displayName = QString::fromStdString( itemName );
                }
                else
                {
                    displayName = QString::fromStdString( longVectorName );
                    displayName += QString( " (%1)" ).arg( QString::fromStdString( itemName ) );
                }
            }
            else
            {
                displayName = QString::fromStdString( itemName );
            }

            auto optionItem = caf::PdmOptionItemInfo( displayName, QString::fromStdString( itemName ) + itemPostfix );
            if ( groupItems ) optionItem.setLevel( 1 );
            options.push_back( optionItem );
        }
    }
}
