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

#include "RimSummaryAddress.h"

#include "RiaLogging.h"
#include "Summary/RiaSummaryDefines.h"

#include <QRegularExpression>

namespace caf
{
template <>
void caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>::setUp()
{
    using RifAdr = RifEclipseSummaryAddressDefines::SummaryCategory;

    addItem( RifAdr::SUMMARY_FIELD, "SUMMARY_FIELD", RiaDefines::summaryField() );
    addItem( RifAdr::SUMMARY_AQUIFER, "SUMMARY_AQUIFER", RiaDefines::summaryAquifer() );
    addItem( RifAdr::SUMMARY_NETWORK, "SUMMARY_NETWORK", RiaDefines::summaryNetwork() );
    addItem( RifAdr::SUMMARY_MISC, "SUMMARY_MISC", RiaDefines::summaryMisc() );
    addItem( RifAdr::SUMMARY_REGION, "SUMMARY_REGION", RiaDefines::summaryRegion() );
    addItem( RifAdr::SUMMARY_REGION_2_REGION, "SUMMARY_REGION_2_REGION", RiaDefines::summaryRegion2Region() );
    addItem( RifAdr::SUMMARY_GROUP, "SUMMARY_WELL_GROUP", RiaDefines::summaryWellGroup() );
    addItem( RifAdr::SUMMARY_WELL, "SUMMARY_WELL", RiaDefines::summaryWell() );
    addItem( RifAdr::SUMMARY_WELL_COMPLETION, "SUMMARY_WELL_COMPLETION", RiaDefines::summaryWellCompletion() );
    addItem( RifAdr::SUMMARY_WELL_CONNECTION, "SUMMARY_WELL_CONNECTION", RiaDefines::summaryWellConnection() );
    addItem( RifAdr::SUMMARY_WELL_CONNECTION_LGR, "SUMMARY_WELL_CONNECTION_LGR", RiaDefines::summaryLgrConnection() );
    addItem( RifAdr::SUMMARY_WELL_LGR, "SUMMARY_WELL_LGR", RiaDefines::summaryLgrWell() );
    addItem( RifAdr::SUMMARY_WELL_SEGMENT, "SUMMARY_SEGMENT", RiaDefines::summarySegment() );
    addItem( RifAdr::SUMMARY_BLOCK, "SUMMARY_BLOCK", RiaDefines::summaryBlock() );
    addItem( RifAdr::SUMMARY_BLOCK_LGR, "SUMMARY_BLOCK_LGR", RiaDefines::summaryLgrBlock() );
    addItem( RifAdr::SUMMARY_IMPORTED, "SUMMARY_IMPORTED", "Imported" );
    setDefault( RifAdr::SUMMARY_FIELD );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryAddress, "SummaryAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::RimSummaryAddress()
{
    CAF_PDM_InitObject( "SummaryAddress", ":/DataVector.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_category, "SummaryVarType", "Type" );
    CAF_PDM_InitFieldNoDefault( &m_statistics, "StatisticsType", "StatisticsType" );
    CAF_PDM_InitFieldNoDefault( &m_vectorName, "SummaryQuantityName", "Vector" );
    CAF_PDM_InitFieldNoDefault( &m_regionNumber, "SummaryRegion", "Region" );
    CAF_PDM_InitFieldNoDefault( &m_regionNumber2, "SummaryRegion2", "Region2" );
    CAF_PDM_InitFieldNoDefault( &m_groupName, "SummaryWellGroup", "Group" );
    CAF_PDM_InitFieldNoDefault( &m_networkName, "SummaryNetworkGroup", "Network" );
    CAF_PDM_InitFieldNoDefault( &m_wellName, "SummaryWell", "Well" );
    CAF_PDM_InitFieldNoDefault( &m_wellSegmentNumber, "SummaryWellSegment", "Well Segment" );
    CAF_PDM_InitFieldNoDefault( &m_lgrName, "SummaryLgr", "Grid" );
    CAF_PDM_InitFieldNoDefault( &m_cellI, "SummaryCellI", "I" );
    CAF_PDM_InitFieldNoDefault( &m_cellJ, "SummaryCellJ", "J" );
    CAF_PDM_InitFieldNoDefault( &m_cellK, "SummaryCellK", "K" );
    CAF_PDM_InitFieldNoDefault( &m_aquiferNumber, "SummaryAquifer", "Aquifer" );
    CAF_PDM_InitFieldNoDefault( &m_wellCompletionNumber, "SummaryWellCompletionNumber", "Well Completion Number" );
    CAF_PDM_InitFieldNoDefault( &m_isErrorResult, "IsErrorResult", "Is Error Result" );
    CAF_PDM_InitFieldNoDefault( &m_calculationId, "CalculationId", "Calculation Id" );

    CAF_PDM_InitField( &m_caseId, "CaseId", -1, "CaseId" );
    CAF_PDM_InitField( &m_ensembleId, "EnsembleId", -1, "EnsembleId" );

    m_category             = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID;
    m_statistics           = RifEclipseSummaryAddressDefines::StatisticsType::NONE;
    m_regionNumber         = -1;
    m_regionNumber2        = -1;
    m_wellSegmentNumber    = -1;
    m_cellI                = -1;
    m_cellJ                = -1;
    m_cellK                = -1;
    m_aquiferNumber        = -1;
    m_wellCompletionNumber = -1;
    m_isErrorResult        = false;
    m_calculationId        = -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::~RimSummaryAddress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddress*
    RimSummaryAddress::wrapFileReaderAddress( const RifEclipseSummaryAddress& addr, int caseId /* = -1 */, int ensembleId /* = -1 */ )
{
    RimSummaryAddress* newAddress = new RimSummaryAddress();
    newAddress->setAddress( addr );
    newAddress->setCaseId( caseId );
    newAddress->setEnsembleId( ensembleId );
    return newAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::setAddress( const RifEclipseSummaryAddress& addr )
{
    m_category             = addr.category();
    m_statistics           = addr.statisticsType();
    m_vectorName           = addr.vectorName().c_str();
    m_regionNumber         = addr.regionNumber();
    m_regionNumber2        = addr.regionNumber2();
    m_groupName            = addr.groupName().c_str();
    m_networkName          = addr.networkName().c_str();
    m_wellName             = addr.wellName().c_str();
    m_wellSegmentNumber    = addr.wellSegmentNumber();
    m_lgrName              = addr.lgrName().c_str();
    m_aquiferNumber        = addr.aquiferNumber();
    m_wellCompletionNumber = addr.wellCompletionNumber();
    m_isErrorResult        = addr.isErrorResult();

    m_cellI         = addr.cellI();
    m_cellJ         = addr.cellJ();
    m_cellK         = addr.cellK();
    m_calculationId = addr.id();

    setUiName( m_vectorName );
    setUiIconFromResourceString( iconResourceText() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddress::address() const
{
    return RifEclipseSummaryAddress( m_category(),
                                     m_statistics(),
                                     m_vectorName().toStdString(),
                                     m_regionNumber(),
                                     m_regionNumber2(),
                                     m_groupName().toStdString(),
                                     m_networkName().toStdString(),
                                     m_wellName().toStdString(),
                                     m_wellSegmentNumber(),
                                     m_lgrName().toStdString(),
                                     m_cellI(),
                                     m_cellJ(),
                                     m_cellK(),
                                     m_aquiferNumber,
                                     m_wellCompletionNumber(),
                                     m_isErrorResult,
                                     m_calculationId );
}

//--------------------------------------------------------------------------------------------------
/// Return phase type if the current result is known to be of a particular
/// fluid phase type. Otherwise the method will return PHASE_NOT_APPLICABLE.
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimSummaryAddress::addressPhaseType() const
{
    if ( QRegularExpression( "^.OP" ).match( m_vectorName ).hasMatch() )
    {
        return RiaDefines::PhaseType::OIL_PHASE;
    }
    else if ( QRegularExpression( "^.GP" ).match( m_vectorName ).hasMatch() )
    {
        return RiaDefines::PhaseType::GAS_PHASE;
    }
    else if ( QRegularExpression( "^.WP" ).match( m_vectorName ).hasMatch() )
    {
        return RiaDefines::PhaseType::WATER_PHASE;
    }
    return RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryAddress::keywordForCategory( RifEclipseSummaryAddressDefines::SummaryCategory category ) const
{
    // Return the keyword text for supported field replacement in plot templates

    if ( category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL ) return m_wellName.keyword();
    if ( category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP ) return m_groupName.keyword();
    if ( category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION ) return m_regionNumber.keyword();
    if ( category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK ) return m_networkName.keyword();

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::setCaseId( int caseId )
{
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddress::caseId() const
{
    return m_caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryAddress::quantityName() const
{
    return m_vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::setEnsembleId( int ensembleId )
{
    m_ensembleId = ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddress::ensembleId() const
{
    return m_ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddress::isEnsemble() const
{
    return m_ensembleId >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryAddress::iconResourceText() const
{
    if ( m_calculationId != -1 ) return ":/DataVectorCalculated.svg";

    return ":/DataVector.svg";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::initAfterRead()
{
    // The changes in this function is related to https://github.com/OPM/ResInsight/issues/12214
    //
    // Lumped connection vectors was previously displayed as well vectors. Move them into well completions and extract the well
    // completion number.
    if ( m_vectorName().contains( "__" ) && m_category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL )
    {
        RiaLogging::info( "Converting lumped vector with '*L__N' syntax into Well Completion." + m_vectorName() );

        m_category             = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION;
        m_wellCompletionNumber = m_vectorName().split( "__" ).last().toInt();
        m_vectorName           = m_vectorName().split( "__" ).first();
    }

    // Lumped connection vectors was previously displayed as well completion vectors. Move them into well completions
    if ( m_vectorName().startsWith( "C" ) && m_category == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION )
    {
        RiaLogging::info( "Converting completion vector starting with 'C' syntax into Well Connection." + m_vectorName() );

        m_category = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION;
    }
}
