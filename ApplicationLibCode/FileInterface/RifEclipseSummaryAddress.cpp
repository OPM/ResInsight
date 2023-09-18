/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RiaStdStringTools.h"

#include "RifEclEclipseSummary.h"
#include "RiuSummaryQuantityNameInfoProvider.h"

#include <QStringList>
#include <QTextStream>

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress( SummaryCategory category, std::map<SummaryIdentifierType, std::string>& identifiers )
    : m_category( category )
    , m_number0( -1 )
    , m_number1( -1 )
    , m_number2( -1 )
    , m_isErrorResult( false )
    , m_id( -1 )
{
    std::pair<int, int> reg2regPair;
    switch ( category )
    {
        case SummaryCategory::SUMMARY_REGION:
            m_number0 = RiaStdStringTools::toInt16( identifiers[SummaryIdentifierType::INPUT_REGION_NUMBER] );
            break;
        case SummaryCategory::SUMMARY_REGION_2_REGION:
            reg2regPair = regionToRegionPairFromUiText( identifiers[SummaryIdentifierType::INPUT_REGION_2_REGION] );
            m_number0   = reg2regPair.first;
            m_number1   = reg2regPair.second;
            break;
        case SummaryCategory::SUMMARY_GROUP:
            m_name = identifiers[SummaryIdentifierType::INPUT_GROUP_NAME];
            break;
        case SummaryCategory::SUMMARY_NETWORK:
            m_name = identifiers[SummaryIdentifierType::INPUT_NETWORK_NAME];
            break;
        case SummaryCategory::SUMMARY_WELL:
            m_name = identifiers[SummaryIdentifierType::INPUT_WELL_NAME];
            break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION:
            m_name = identifiers[SummaryIdentifierType::INPUT_WELL_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[SummaryIdentifierType::INPUT_CELL_IJK] ) );
            break;
        case SummaryCategory::SUMMARY_WELL_LGR:
            m_lgrName = identifiers[SummaryIdentifierType::INPUT_LGR_NAME];
            m_name    = identifiers[SummaryIdentifierType::INPUT_WELL_NAME];
            break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
            m_lgrName = identifiers[SummaryIdentifierType::INPUT_LGR_NAME];
            m_name    = identifiers[SummaryIdentifierType::INPUT_WELL_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[SummaryIdentifierType::INPUT_CELL_IJK] ) );
            break;
        case SummaryCategory::SUMMARY_WELL_SEGMENT:
            m_name    = identifiers[SummaryIdentifierType::INPUT_WELL_NAME];
            m_number0 = RiaStdStringTools::toInt( identifiers[SummaryIdentifierType::INPUT_SEGMENT_NUMBER] );
            break;
        case SummaryCategory::SUMMARY_BLOCK:
            setCellIjk( ijkTupleFromUiText( identifiers[SummaryIdentifierType::INPUT_CELL_IJK] ) );
            break;
        case SummaryCategory::SUMMARY_BLOCK_LGR:
            m_lgrName = identifiers[SummaryIdentifierType::INPUT_LGR_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[SummaryIdentifierType::INPUT_CELL_IJK] ) );
            break;
        case SummaryCategory::SUMMARY_AQUIFER:
            m_number0 = RiaStdStringTools::toInt( identifiers[SummaryIdentifierType::INPUT_AQUIFER_NUMBER] );
            break;
    }

    m_vectorName = identifiers[SummaryIdentifierType::INPUT_VECTOR_NAME];
    m_id         = RiaStdStringTools::toInt( identifiers[SummaryIdentifierType::INPUT_ID] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress( SummaryCategory    category,
                                                    const std::string& vectorName,
                                                    int                regionNumber,
                                                    int                regionNumber2,
                                                    const std::string& groupName,
                                                    const std::string& networkName,
                                                    const std::string& wellName,
                                                    int                wellSegmentNumber,
                                                    const std::string& lgrName,
                                                    int                cellI,
                                                    int                cellJ,
                                                    int                cellK,
                                                    int                aquiferNumber,
                                                    bool               isErrorResult,
                                                    int                id )
    : m_category( category )
    , m_vectorName( vectorName )
    , m_lgrName( lgrName )
    , m_number0( -1 )
    , m_number1( -1 )
    , m_number2( -1 )
    , m_isErrorResult( isErrorResult )
    , m_id( id )
{
    switch ( category )
    {
        case SummaryCategory::SUMMARY_REGION:
            m_number0 = regionNumber;
            break;
        case SummaryCategory::SUMMARY_REGION_2_REGION:
            m_number0 = regionNumber;
            m_number1 = regionNumber2;
            break;
        case SummaryCategory::SUMMARY_GROUP:
            m_name = groupName;
            break;
        case SummaryCategory::SUMMARY_NETWORK:
            m_name = networkName;
            break;
        case SummaryCategory::SUMMARY_WELL:
            m_name = wellName;
            break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION:
            m_name = wellName;
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SummaryCategory::SUMMARY_WELL_LGR:
            m_name = wellName;
            break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
            m_name = wellName;
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SummaryCategory::SUMMARY_WELL_SEGMENT:
            m_name    = wellName;
            m_number0 = wellSegmentNumber;
            break;
        case SummaryCategory::SUMMARY_BLOCK:
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SummaryCategory::SUMMARY_BLOCK_LGR:
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SummaryCategory::SUMMARY_AQUIFER:
            m_number0 = aquiferNumber;
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress()
    : m_category( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
    , m_number0( -1 )
    , m_number1( -1 )
    , m_number2( -1 )
    , m_isErrorResult( false )
    , m_id( -1 )
{
}

//--------------------------------------------------------------------------------------------------
/// Column header text format:   [<ER|ERR|ERROR>:]<VECTOR>:<CATEGORY_PARAM_NAME1>[:<CATEGORY_PARAM_NAME2>][....]
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( const std::string& textAddress )
{
    auto tokens = RiaStdStringTools::splitString( textAddress, ':' );

    bool isErrorResult = false;

    if ( tokens.size() > 1 )
    {
        auto firstToken = RiaStdStringTools::trimString( tokens[0] );
        firstToken      = RiaStdStringTools::toUpper( firstToken );

        if ( ( firstToken == "ER" ) || ( firstToken == "ERR" ) || ( firstToken == "ERROR" ) )
        {
            isErrorResult = true;
            tokens.erase( tokens.begin() );
        }
    }

    auto address = fromTokens( tokens );

    if ( address.category() == SummaryCategory::SUMMARY_INVALID || address.category() == SummaryCategory::SUMMARY_IMPORTED )
    {
        // Address category not recognized, use incoming text string without error prefix as vector name
        auto text = RiaStdStringTools::joinStrings( tokens, ':' );
        address   = importedAddress( text );
    }

    if ( isErrorResult ) address.setAsErrorResult();
    return address;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fromEclipseTextAddress( const std::string& textAddress )
{
    auto tokens = RiaStdStringTools::splitString( textAddress, ':' );

    return fromTokens( tokens );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fieldAddress( const std::string& vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_FIELD;
    addr.m_vectorName = vectorName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::aquiferAddress( const std::string& vectorName, int aquiferNumber, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_AQUIFER;
    addr.m_vectorName = vectorName;
    addr.m_number0    = aquiferNumber;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::networkAddress( const std::string& vectorName, const std::string& networkName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_NETWORK;
    addr.m_vectorName = vectorName;
    addr.m_name       = networkName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::miscAddress( const std::string& vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_MISC;
    addr.m_vectorName = vectorName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::regionAddress( const std::string& vectorName, int regionNumber, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_REGION;
    addr.m_vectorName = vectorName;
    addr.m_number0    = regionNumber;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::regionToRegionAddress( const std::string& vectorName, int regionNumber, int region2Number, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_REGION_2_REGION;
    addr.m_vectorName = vectorName;
    addr.m_number0    = regionNumber;
    addr.m_number1    = region2Number;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::groupAddress( const std::string& vectorName, const std::string& groupName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_GROUP;
    addr.m_vectorName = vectorName;
    addr.m_name       = groupName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellAddress( const std::string& vectorName, const std::string& wellName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_WELL;
    addr.m_vectorName = vectorName;
    addr.m_name       = wellName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::wellCompletionAddress( const std::string& vectorName, const std::string& wellName, int i, int j, int k, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_WELL_COMPLETION;
    addr.m_vectorName = vectorName;
    addr.m_name       = wellName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellLgrAddress( const std::string& vectorName,
                                                                   const std::string& lgrName,
                                                                   const std::string& wellName,
                                                                   int                calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_WELL_LGR;
    addr.m_vectorName = vectorName;
    addr.m_lgrName    = lgrName;
    addr.m_name       = wellName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionLgrAddress( const std::string& vectorName,
                                                                             const std::string& lgrName,
                                                                             const std::string& wellName,
                                                                             int                i,
                                                                             int                j,
                                                                             int                k,
                                                                             int                calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_WELL_COMPLETION_LGR;
    addr.m_vectorName = vectorName;
    addr.m_lgrName    = lgrName;
    addr.m_name       = wellName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::wellSegmentAddress( const std::string& vectorName, const std::string& wellName, int segmentNumber, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_WELL_SEGMENT;
    addr.m_vectorName = vectorName;
    addr.m_name       = wellName;
    addr.m_number0    = segmentNumber;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::blockAddress( const std::string& vectorName, int i, int j, int k, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_BLOCK;
    addr.m_vectorName = vectorName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::blockLgrAddress( const std::string& vectorName, const std::string& lgrName, int i, int j, int k, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_BLOCK_LGR;
    addr.m_vectorName = vectorName;
    addr.m_lgrName    = lgrName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::importedAddress( const std::string& vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_IMPORTED;
    addr.m_vectorName = vectorName;
    addr.m_id         = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::ensembleStatisticsAddress( const std::string& vectorName, const std::string& dataQuantityName )
{
    RifEclipseSummaryAddress addr;
    addr.m_category   = SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS;
    addr.m_vectorName = vectorName + ":" + dataQuantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::timeAddress()
{
    RifEclipseSummaryAddress addr;
    addr.m_category = SummaryCategory::SUMMARY_TIME;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::generateStringFromAddresses( const std::vector<RifEclipseSummaryAddress>& addressVector,
                                                                   const std::string                            jointString )
{
    std::string addrString;
    for ( const RifEclipseSummaryAddress& address : addressVector )
    {
        if ( addrString.length() > 0 )
        {
            addrString += jointString;
        }
        addrString += address.uiText();
        if ( addrString.length() > 50 )
        {
            addrString = addrString.substr( 0, 50 ) + "...";
            break;
        }
    }
    return addrString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isDependentOnWellName( SummaryCategory category )
{
    return ( category == SummaryCategory::SUMMARY_WELL || category == SummaryCategory::SUMMARY_WELL_COMPLETION ||
             category == SummaryCategory::SUMMARY_WELL_COMPLETION_LGR || category == SummaryCategory::SUMMARY_WELL_LGR ||
             category == SummaryCategory::SUMMARY_WELL_SEGMENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RifEclipseSummaryAddress::category() const
{
    return m_category;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::vectorName() const
{
    return m_vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isHistoryVector() const
{
    const std::string historyIdentifier = "H";

    return RiaStdStringTools::endsWith( m_vectorName, historyIdentifier );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::regionNumber() const
{
    return m_number0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::regionNumber2() const
{
    return m_number1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::groupName() const
{
    return ( m_category == SummaryCategory::SUMMARY_GROUP ) ? m_name : std::string();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::networkName() const
{
    return ( m_category == SummaryCategory::SUMMARY_NETWORK ) ? m_name : std::string();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::wellName() const
{
    return isDependentOnWellName( m_category ) ? m_name : std::string();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::wellSegmentNumber() const
{
    return m_number0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::lgrName() const
{
    return m_lgrName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::cellI() const
{
    // IJK is stored in the order KJI to be able to sort the addresses by K first
    return m_number2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::cellJ() const
{
    // IJK is stored in the order KJI to be able to sort the addresses by K first
    return m_number1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::cellK() const
{
    // IJK is stored in the order KJI to be able to sort the addresses by K first
    return m_number0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::aquiferNumber() const
{
    return m_number0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseSummaryAddress::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::ensembleStatisticsVectorName() const
{
    QString qName = QString::fromStdString( m_vectorName );
    return qName.split( ":" )[0].toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::uiText() const
{
    std::string text;

    if ( m_isErrorResult ) text += "ERR:";

    text += m_vectorName;

    std::string itemText = itemUiText();
    if ( !itemText.empty() )
    {
        text += ":" + itemText;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::itemUiText() const
{
    std::string text;

    switch ( category() )
    {
        case SummaryCategory::SUMMARY_REGION:
        {
            text += std::to_string( regionNumber() );
        }
        break;
        case SummaryCategory::SUMMARY_REGION_2_REGION:
        {
            text += formatUiTextRegionToRegion();
        }
        break;
        case SummaryCategory::SUMMARY_GROUP:
        {
            text += groupName();
        }
        break;
        case SummaryCategory::SUMMARY_NETWORK:
        {
            text += networkName();
        }
        break;
        case SummaryCategory::SUMMARY_WELL:
        {
            text += wellName();
        }
        break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION:
        {
            text += wellName();
            text += ":" + blockAsString();
        }
        break;
        case SummaryCategory::SUMMARY_WELL_LGR:
        {
            text += lgrName();
            text += ":";
            text += wellName();
        }
        break;
        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
        {
            text += lgrName();
            text += ":";
            text += wellName();
            text += ":";
            text += blockAsString();
        }
        break;
        case SummaryCategory::SUMMARY_WELL_SEGMENT:
        {
            text += wellName();
            text += ":";
            text += std::to_string( wellSegmentNumber() );
        }
        break;
        case SummaryCategory::SUMMARY_BLOCK:
        {
            text += blockAsString();
        }
        break;
        case SummaryCategory::SUMMARY_BLOCK_LGR:
        {
            text += lgrName();
            text += ":";
            text += blockAsString();
        }
        break;
        case SummaryCategory::SUMMARY_AQUIFER:
        {
            text += std::to_string( aquiferNumber() );
        }
        break;
        case SummaryCategory::SUMMARY_IMPORTED:
        {
            text += vectorName();
        }
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// Returns the stringified address component requested
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::addressComponentUiText( RifEclipseSummaryAddressDefines::SummaryIdentifierType identifierType ) const
{
    switch ( identifierType )
    {
        case SummaryIdentifierType::INPUT_REGION_NUMBER:
            return std::to_string( regionNumber() );
        case SummaryIdentifierType::INPUT_REGION_2_REGION:
            return formatUiTextRegionToRegion();
        case SummaryIdentifierType::INPUT_WELL_NAME:
            return m_name;
        case SummaryIdentifierType::INPUT_GROUP_NAME:
            return m_name;
        case SummaryIdentifierType::INPUT_NETWORK_NAME:
            return m_name;
        case SummaryIdentifierType::INPUT_CELL_IJK:
            return blockAsString();
        case SummaryIdentifierType::INPUT_LGR_NAME:
            return m_lgrName;
        case SummaryIdentifierType::INPUT_SEGMENT_NUMBER:
            return std::to_string( wellSegmentNumber() );
        case SummaryIdentifierType::INPUT_AQUIFER_NUMBER:
            return std::to_string( aquiferNumber() );
        case SummaryIdentifierType::INPUT_VECTOR_NAME:
            return m_vectorName;
        case SummaryIdentifierType::INPUT_ID:
            return std::to_string( id() );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isUiTextMatchingFilterText( const QString& filterString ) const
{
    std::string value = uiText();
    if ( filterString.isEmpty() ) return true;
    if ( filterString.trimmed() == "*" ) return !value.empty();

    QRegExp searcher( filterString, Qt::CaseInsensitive, QRegExp::WildcardUnix );
    QString qstrValue = QString::fromStdString( value );
    return searcher.exactMatch( qstrValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isValid() const
{
    if ( m_vectorName.empty() ) return false;

    switch ( category() )
    {
        case SummaryCategory::SUMMARY_INVALID:
            return false;

        case SummaryCategory::SUMMARY_REGION:
            if ( m_number0 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_REGION_2_REGION:
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_GROUP:
            if ( m_name.empty() ) return false;
            return true;

        case SummaryCategory::SUMMARY_WELL:
            if ( m_name.empty() ) return false;
            return true;

        case SummaryCategory::SUMMARY_WELL_COMPLETION:
            if ( m_name.empty() ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_WELL_LGR:
            if ( m_lgrName.empty() ) return false;
            if ( m_name.empty() ) return false;
            return true;

        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
            if ( m_lgrName.empty() ) return false;
            if ( m_name.empty() ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_WELL_SEGMENT:
            if ( m_name.empty() ) return false;
            if ( m_number0 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_BLOCK:
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_BLOCK_LGR:
            if ( m_lgrName.empty() ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SummaryCategory::SUMMARY_AQUIFER:
            if ( m_number0 == -1 ) return false;
            return true;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setVectorName( const std::string& vectorName )
{
    m_vectorName = vectorName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setWellName( const std::string& wellName )
{
    m_name = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setGroupName( const std::string& groupName )
{
    m_name = groupName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setNetworkName( const std::string& networkName )
{
    m_name = networkName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setRegion( int region )
{
    m_number0 = region;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setRegion2( int region2 )
{
    m_number1 = region2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setAquiferNumber( int aquiferNumber )
{
    m_number0 = aquiferNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setCellIjk( const std::string& uiText )
{
    auto vec = RifEclipseSummaryAddress::ijkTupleFromUiText( uiText );

    setCellIjk( std::get<0>( vec ), std::get<1>( vec ), std::get<2>( vec ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setCellIjk( int i, int j, int k )
{
    // NB! Order is reversed to be able to sort on K first
    m_number0 = k;
    m_number1 = j;
    m_number2 = i;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setCellIjk( std::tuple<int, int, int> ijk )
{
    setCellIjk( std::get<0>( ijk ), std::get<1>( ijk ), std::get<2>( ijk ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setWellSegmentNumber( int segment )
{
    m_number0 = segment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setAsErrorResult()
{
    m_isErrorResult = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isErrorResult() const
{
    return m_isErrorResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setId( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::hasAccumulatedData() const
{
    if ( !isValidEclipseCategory() ) return false;

    QString quantityForInspection = QString::fromStdString( vectorName() );
    if ( category() == SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS )
    {
        // Remove statistics text prefix
        quantityForInspection = quantityForInspection.mid( quantityForInspection.indexOf( ":" ) + 1 );
    }

    QString qBaseName = QString::fromStdString( baseVectorName( quantityForInspection.toStdString() ) );

    if ( qBaseName.endsWith( "WCT" ) || qBaseName.endsWith( "WCTH" ) )
    {
        // https://github.com/OPM/ResInsight/issues/5808
        return false;
    }

    return qBaseName.endsWith( "T" ) || qBaseName.endsWith( "TH" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fromTokens( const std::vector<std::string>& tokens )
{
    if ( tokens.empty() ) return {};

    std::string vectorName;
    std::string token1;
    std::string token2;

    int intValue0 = 0;
    int intValue1 = 0;
    int intValue2 = 0;

    vectorName = tokens[0];

    if ( tokens.size() > 1 ) token1 = tokens[1];
    if ( tokens.size() > 2 ) token2 = tokens[2];

    SummaryCategory category = RiuSummaryQuantityNameInfoProvider::instance()->identifyCategory( vectorName );

    switch ( category )
    {
        case SummaryCategory::SUMMARY_FIELD:
            return fieldAddress( vectorName );

        case SummaryCategory::SUMMARY_AQUIFER:
            if ( !token1.empty() )
            {
                RiaStdStringTools::toInt( token1, intValue0 );
                return aquiferAddress( vectorName, intValue0 );
            }
            break;

        case SummaryCategory::SUMMARY_NETWORK:
            return networkAddress( vectorName, token1 );
            break;

        case SummaryCategory::SUMMARY_MISC:
            return miscAddress( vectorName );
            break;

        case SummaryCategory::SUMMARY_REGION:
            if ( !token1.empty() )
            {
                RiaStdStringTools::toInt( token1, intValue0 );
                return regionAddress( vectorName, intValue0 );
            }
            break;

        case SummaryCategory::SUMMARY_REGION_2_REGION:
            if ( !token1.empty() )
            {
                auto regions = RiaStdStringTools::splitString( token1, '-' );
                if ( regions.size() == 2 )
                {
                    RiaStdStringTools::toInt( regions[0], intValue0 );
                    RiaStdStringTools::toInt( regions[1], intValue1 );

                    return regionToRegionAddress( vectorName, intValue0, intValue1 );
                }
            }
            break;

        case SummaryCategory::SUMMARY_GROUP:
            if ( !token1.empty() ) return groupAddress( vectorName, token1 );
            break;

        case SummaryCategory::SUMMARY_WELL:
            if ( !token1.empty() ) return wellAddress( vectorName, token1 );
            break;

        case SummaryCategory::SUMMARY_WELL_COMPLETION:
            if ( !token2.empty() )
            {
                auto ijk = RiaStdStringTools::splitString( token2, ',' );
                if ( ijk.size() == 3 )
                {
                    RiaStdStringTools::toInt( ijk[0], intValue0 );
                    RiaStdStringTools::toInt( ijk[1], intValue1 );
                    RiaStdStringTools::toInt( ijk[2], intValue2 );

                    return wellCompletionAddress( vectorName, token1, intValue0, intValue1, intValue2 );
                }
            }
            break;

        case SummaryCategory::SUMMARY_WELL_LGR:
            if ( !token1.empty() && !token2.empty() ) return wellLgrAddress( vectorName, token1, token2 );
            break;

        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
            if ( tokens.size() > 2 )
            {
                const auto& token3 = tokens[3];
                const auto  ijk    = RiaStdStringTools::splitString( token3, ',' );
                if ( ijk.size() == 3 )
                {
                    RiaStdStringTools::toInt( ijk[0], intValue0 );
                    RiaStdStringTools::toInt( ijk[1], intValue1 );
                    RiaStdStringTools::toInt( ijk[2], intValue2 );

                    return wellCompletionLgrAddress( vectorName, token1, token2, intValue0, intValue1, intValue2 );
                }
            }
            break;

        case SummaryCategory::SUMMARY_WELL_SEGMENT:

            if ( !token2.empty() )
            {
                RiaStdStringTools::toInt( token2, intValue0 );

                return wellSegmentAddress( vectorName, token1, intValue0 );
            }
            break;

        case SummaryCategory::SUMMARY_BLOCK:
            if ( !token1.empty() )
            {
                auto ijk = RiaStdStringTools::splitString( token1, ',' );
                if ( ijk.size() == 3 )
                {
                    RiaStdStringTools::toInt( ijk[0], intValue0 );
                    RiaStdStringTools::toInt( ijk[1], intValue1 );
                    RiaStdStringTools::toInt( ijk[2], intValue2 );

                    return blockAddress( vectorName, intValue0, intValue1, intValue2 );
                }
            }
            break;

        case SummaryCategory::SUMMARY_BLOCK_LGR:
            if ( !token2.empty() )
            {
                auto ijk = RiaStdStringTools::splitString( token2, ',' );
                if ( ijk.size() == 3 )
                {
                    RiaStdStringTools::toInt( ijk[0], intValue0 );
                    RiaStdStringTools::toInt( ijk[1], intValue1 );
                    RiaStdStringTools::toInt( ijk[2], intValue2 );

                    return blockLgrAddress( vectorName, token1, intValue0, intValue1, intValue2 );
                }
            }
            break;

        case SummaryCategory::SUMMARY_IMPORTED:
        case SummaryCategory::SUMMARY_INVALID:
        default:
            break;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isValidEclipseCategory() const
{
    switch ( category() )
    {
        case SummaryCategory::SUMMARY_FIELD:
        case SummaryCategory::SUMMARY_AQUIFER:
        case SummaryCategory::SUMMARY_NETWORK:
        case SummaryCategory::SUMMARY_MISC:
        case SummaryCategory::SUMMARY_REGION:
        case SummaryCategory::SUMMARY_REGION_2_REGION:
        case SummaryCategory::SUMMARY_GROUP:
        case SummaryCategory::SUMMARY_WELL:
        case SummaryCategory::SUMMARY_WELL_COMPLETION:
        case SummaryCategory::SUMMARY_WELL_LGR:
        case SummaryCategory::SUMMARY_WELL_COMPLETION_LGR:
        case SummaryCategory::SUMMARY_WELL_SEGMENT:
        case SummaryCategory::SUMMARY_BLOCK:
        case SummaryCategory::SUMMARY_BLOCK_LGR:
        case SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS:
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::baseVectorName( const std::string& vectorName )
{
    auto tmpString = vectorName;

    if ( tmpString.size() == 8 ) tmpString = tmpString.substr( 0, 5 );

    auto indexToUnderscore = tmpString.find_first_of( '_' );
    if ( indexToUnderscore > 0 )
    {
        tmpString = tmpString.substr( 0, indexToUnderscore );
    }

    return tmpString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::blockAsString() const
{
    // Avoid space in address text https://github.com/OPM/ResInsight/issues/9707

    return std::to_string( cellI() ) + "," + std::to_string( cellJ() ) + "," + std::to_string( cellK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> RifEclipseSummaryAddress::ijkTupleFromUiText( const std::string& s )
{
    QStringList ijk = QString().fromStdString( s ).trimmed().split( QRegExp( "[,]" ) );

    if ( ijk.size() != 3 ) return std::make_tuple( -1, -1, -1 );

    return std::make_tuple( RiaStdStringTools::toInt( ijk[0].trimmed().toStdString() ),
                            RiaStdStringTools::toInt( ijk[1].trimmed().toStdString() ),
                            RiaStdStringTools::toInt( ijk[2].trimmed().toStdString() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::formatUiTextRegionToRegion() const
{
    return std::to_string( regionNumber() ) + " - " + std::to_string( regionNumber2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RifEclipseSummaryAddress::regionToRegionPairFromUiText( const std::string& s )
{
    QStringList r2r = QString().fromStdString( s ).trimmed().split( QRegExp( "[-]" ) );

    if ( r2r.size() != 2 ) return std::make_pair( -1, -1 );

    return std::make_pair( RiaStdStringTools::toInt16( r2r[0].trimmed().toStdString() ),
                           RiaStdStringTools::toInt16( r2r[1].trimmed().toStdString() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isCalculated() const
{
    return m_id != -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseSummaryAddress::isTime() const
{
    return m_category == SummaryCategory::SUMMARY_TIME;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<( QTextStream& str, const RifEclipseSummaryAddress& sobj )
{
    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator>>( QTextStream& str, RifEclipseSummaryAddress& sobj )
{
    return str;
}
