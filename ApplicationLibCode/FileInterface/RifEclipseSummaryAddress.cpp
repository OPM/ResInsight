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
RifEclipseSummaryAddress::RifEclipseSummaryAddress( SummaryVarCategory category, std::map<SummaryIdentifierType, std::string>& identifiers )
    : m_variableCategory( category )
    , m_number0( -1 )
    , m_number1( -1 )
    , m_number2( -1 )
    , m_isErrorResult( false )
    , m_id( -1 )
{
    std::pair<int, int> reg2regPair;
    switch ( category )
    {
        case SUMMARY_REGION:
            m_number0 = RiaStdStringTools::toInt16( identifiers[INPUT_REGION_NUMBER] );
            break;
        case SUMMARY_REGION_2_REGION:
            reg2regPair = regionToRegionPairFromUiText( identifiers[INPUT_REGION_2_REGION] );
            m_number0   = reg2regPair.first;
            m_number1   = reg2regPair.second;
            break;
        case SUMMARY_GROUP:
            m_name = identifiers[INPUT_GROUP_NAME];
            break;
        case SUMMARY_NETWORK:
            m_name = identifiers[INPUT_NETWORK_NAME];
            break;
        case SUMMARY_WELL:
            m_name = identifiers[INPUT_WELL_NAME];
            break;
        case SUMMARY_WELL_COMPLETION:
            m_name = identifiers[INPUT_WELL_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] ) );
            break;
        case SUMMARY_WELL_LGR:
            m_lgrName = identifiers[INPUT_LGR_NAME];
            m_name    = identifiers[INPUT_WELL_NAME];
            break;
        case SUMMARY_WELL_COMPLETION_LGR:
            m_lgrName = identifiers[INPUT_LGR_NAME];
            m_name    = identifiers[INPUT_WELL_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] ) );
            break;
        case SUMMARY_WELL_SEGMENT:
            m_name    = identifiers[INPUT_WELL_NAME];
            m_number0 = RiaStdStringTools::toInt( identifiers[INPUT_SEGMENT_NUMBER] );
            break;
        case SUMMARY_BLOCK:
            setCellIjk( ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] ) );
            break;
        case SUMMARY_BLOCK_LGR:
            m_lgrName = identifiers[INPUT_LGR_NAME];
            setCellIjk( ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] ) );
            break;
        case SUMMARY_AQUIFER:
            m_number0 = RiaStdStringTools::toInt( identifiers[INPUT_AQUIFER_NUMBER] );
            break;
    }

    m_vectorName = identifiers[INPUT_VECTOR_NAME];
    m_id         = RiaStdStringTools::toInt( identifiers[INPUT_ID] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress( SummaryVarCategory     category,
                                                    const std::string_view vectorName,
                                                    int                    regionNumber,
                                                    int                    regionNumber2,
                                                    const std::string_view groupName,
                                                    const std::string_view networkName,
                                                    const std::string_view wellName,
                                                    int                    wellSegmentNumber,
                                                    const std::string_view lgrName,
                                                    int                    cellI,
                                                    int                    cellJ,
                                                    int                    cellK,
                                                    int                    aquiferNumber,
                                                    bool                   isErrorResult,
                                                    int                    id )
    : m_variableCategory( category )
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
        case SUMMARY_REGION:
            m_number0 = regionNumber;
            break;
        case SUMMARY_REGION_2_REGION:
            m_number0 = regionNumber;
            m_number1 = regionNumber2;
            break;
        case SUMMARY_GROUP:
            m_name = groupName;
            break;
        case SUMMARY_NETWORK:
            m_name = networkName;
            break;
        case SUMMARY_WELL:
            m_name = wellName;
            break;
        case SUMMARY_WELL_COMPLETION:
            m_name = wellName;
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SUMMARY_WELL_LGR:
            m_name = wellName;
            break;
        case SUMMARY_WELL_COMPLETION_LGR:
            m_name = wellName;
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SUMMARY_WELL_SEGMENT:
            m_name    = wellName;
            m_number0 = wellSegmentNumber;
            break;
        case SUMMARY_BLOCK:
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SUMMARY_BLOCK_LGR:
            setCellIjk( cellI, cellJ, cellK );
            break;
        case SUMMARY_AQUIFER:
            m_number0 = aquiferNumber;
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress()
    : m_variableCategory( RifEclipseSummaryAddress::SUMMARY_INVALID )
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

    if ( address.category() == SUMMARY_INVALID || address.category() == SUMMARY_IMPORTED )
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
RifEclipseSummaryAddress RifEclipseSummaryAddress::fieldAddress( const std::string_view vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_FIELD;
    addr.m_vectorName       = vectorName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::aquiferAddress( const std::string_view vectorName, int aquiferNumber, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_AQUIFER;
    addr.m_vectorName       = vectorName;
    addr.m_number0          = aquiferNumber;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::networkAddress( const std::string_view vectorName, const std::string_view networkName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_NETWORK;
    addr.m_vectorName       = vectorName;
    addr.m_name             = networkName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::miscAddress( const std::string_view vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_MISC;
    addr.m_vectorName       = vectorName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::regionAddress( const std::string_view vectorName, int regionNumber, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION;
    addr.m_vectorName       = vectorName;
    addr.m_number0          = regionNumber;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::regionToRegionAddress( const std::string_view vectorName, int regionNumber, int region2Number, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION_2_REGION;
    addr.m_vectorName       = vectorName;
    addr.m_number0          = regionNumber;
    addr.m_number1          = region2Number;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::groupAddress( const std::string_view vectorName, const std::string_view groupName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_GROUP;
    addr.m_vectorName       = vectorName;
    addr.m_name             = groupName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::wellAddress( const std::string_view vectorName, const std::string_view wellName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL;
    addr.m_vectorName       = vectorName;
    addr.m_name             = wellName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionAddress( const std::string_view vectorName,
                                                                          const std::string_view wellName,
                                                                          int                    i,
                                                                          int                    j,
                                                                          int                    k,
                                                                          int                    calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION;
    addr.m_vectorName       = vectorName;
    addr.m_name             = wellName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellLgrAddress( const std::string_view vectorName,
                                                                   const std::string_view lgrName,
                                                                   const std::string_view wellName,
                                                                   int                    calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.m_name             = wellName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionLgrAddress( const std::string_view vectorName,
                                                                             const std::string_view lgrName,
                                                                             const std::string_view wellName,
                                                                             int                    i,
                                                                             int                    j,
                                                                             int                    k,
                                                                             int                    calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.m_name             = wellName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellSegmentAddress( const std::string_view vectorName,
                                                                       const std::string_view wellName,
                                                                       int                    segmentNumber,
                                                                       int                    calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_SEGMENT;
    addr.m_vectorName       = vectorName;
    addr.m_name             = wellName;
    addr.m_number0          = segmentNumber;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::blockAddress( const std::string_view vectorName, int i, int j, int k, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK;
    addr.m_vectorName       = vectorName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::blockLgrAddress( const std::string_view vectorName, const std::string_view lgrName, int i, int j, int k, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.setCellIjk( i, j, k );
    addr.m_id = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::importedAddress( const std::string_view vectorName, int calculationId )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_IMPORTED;
    addr.m_vectorName       = vectorName;
    addr.m_id               = calculationId;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::ensembleStatisticsAddress( const std::string_view vectorName,
                                                                              const std::string_view dataQuantityName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_ENSEMBLE_STATISTICS;
    addr.m_vectorName += vectorName;
    addr.m_vectorName += ":";
    addr.m_vectorName += dataQuantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::generateStringFromAddresses( const std::vector<RifEclipseSummaryAddress>& addressVector,
                                                                   const std::string                            jointString )
{
    std::string addrString;
    for ( RifEclipseSummaryAddress address : addressVector )
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
bool RifEclipseSummaryAddress::isDependentOnWellName( SummaryVarCategory category )
{
    if ( category == SUMMARY_WELL || category == SUMMARY_WELL_COMPLETION || category == SUMMARY_WELL_COMPLETION_LGR ||
         category == SUMMARY_WELL_LGR || category == SUMMARY_WELL_SEGMENT )
    {
        return true;
    }

    return false;
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
const std::string RifEclipseSummaryAddress::ensembleStatisticsVectorName() const
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
        case SUMMARY_REGION:
        {
            text += std::to_string( regionNumber() );
        }
        break;
        case SUMMARY_REGION_2_REGION:
        {
            text += formatUiTextRegionToRegion();
        }
        break;
        case SUMMARY_GROUP:
        {
            text += groupName();
        }
        break;
        case SUMMARY_NETWORK:
        {
            text += networkName();
        }
        break;
        case SUMMARY_WELL:
        {
            text += wellName();
        }
        break;
        case SUMMARY_WELL_COMPLETION:
        {
            text += wellName();
            text += ":" + blockAsString();
        }
        break;
        case SUMMARY_WELL_LGR:
        {
            text += lgrName();
            text += ":";
            text += wellName();
        }
        break;
        case SUMMARY_WELL_COMPLETION_LGR:
        {
            text += lgrName();
            text += ":";
            text += wellName();
            text += ":";
            text += blockAsString();
        }
        break;
        case SUMMARY_WELL_SEGMENT:
        {
            text += wellName();
            text += ":";
            text += std::to_string( wellSegmentNumber() );
        }
        break;
        case SUMMARY_BLOCK:
        {
            text += blockAsString();
        }
        break;
        case SUMMARY_BLOCK_LGR:
        {
            text += lgrName();
            text += ":";
            text += blockAsString();
        }
        break;
        case SUMMARY_AQUIFER:
        {
            text += std::to_string( aquiferNumber() );
        }
        break;
        case SUMMARY_IMPORTED:
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
std::string RifEclipseSummaryAddress::addressComponentUiText( RifEclipseSummaryAddress::SummaryIdentifierType identifierType ) const
{
    switch ( identifierType )
    {
        case INPUT_REGION_NUMBER:
            return std::to_string( regionNumber() );
        case INPUT_REGION_2_REGION:
            return formatUiTextRegionToRegion();
        case INPUT_WELL_NAME:
            return m_name;
        case INPUT_GROUP_NAME:
            return m_name;
        case INPUT_NETWORK_NAME:
            return m_name;
        case INPUT_CELL_IJK:
            return blockAsString();
        case INPUT_LGR_NAME:
            return m_lgrName;
        case INPUT_SEGMENT_NUMBER:
            return std::to_string( wellSegmentNumber() );
        case INPUT_AQUIFER_NUMBER:
            return std::to_string( aquiferNumber() );
        case INPUT_VECTOR_NAME:
            return m_vectorName;
        case INPUT_ID:
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
    if ( filterString.trimmed() == "*" )
    {
        if ( !value.empty() )
            return true;
        else
            return false;
    }

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
        case SUMMARY_INVALID:
            return false;

        case SUMMARY_REGION:
            if ( m_number0 == -1 ) return false;
            return true;

        case SUMMARY_REGION_2_REGION:
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            return true;

        case SUMMARY_GROUP:
            if ( m_name.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL:
            if ( m_name.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL_COMPLETION:
            if ( m_name.size() == 0 ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SUMMARY_WELL_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_name.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL_COMPLETION_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_name.size() == 0 ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SUMMARY_WELL_SEGMENT:
            if ( m_name.size() == 0 ) return false;
            if ( m_number0 == -1 ) return false;
            return true;

        case SUMMARY_BLOCK:
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SUMMARY_BLOCK_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_number0 == -1 ) return false;
            if ( m_number1 == -1 ) return false;
            if ( m_number2 == -1 ) return false;
            return true;

        case SUMMARY_AQUIFER:
            if ( m_number0 == -1 ) return false;
            return true;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setCellIjk( const std::string_view uiText )
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
bool RifEclipseSummaryAddress::hasAccumulatedData() const
{
    if ( !isValidEclipseCategory() ) return false;

    QString quantityForInspection = QString::fromStdString( vectorName() );
    if ( category() == SUMMARY_ENSEMBLE_STATISTICS )
    {
        // Remove statistics text prefix
        quantityForInspection = quantityForInspection.mid( quantityForInspection.indexOf( ":" ) + 1 );
    }

    QString qBaseName = QString::fromStdString( baseVectorName( quantityForInspection.toStdString() ).data() );

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
    if ( tokens.empty() )
    {
        return RifEclipseSummaryAddress();
    }

    std::string vectorName;
    std::string token1;
    std::string token2;

    int intValue0 = 0;
    int intValue1 = 0;
    int intValue2 = 0;

    vectorName = tokens[0];

    if ( tokens.size() > 1 ) token1 = tokens[1];
    if ( tokens.size() > 2 ) token2 = tokens[2];

    SummaryVarCategory category = RiuSummaryQuantityNameInfoProvider::instance()->identifyCategory( vectorName );

    switch ( category )
    {
        case SUMMARY_FIELD:
            return fieldAddress( vectorName );

        case SUMMARY_AQUIFER:
            if ( !token1.empty() )
            {
                RiaStdStringTools::toInt( token1, intValue0 );
                return aquiferAddress( vectorName, intValue0 );
            }
            break;

        case SUMMARY_NETWORK:
            return networkAddress( vectorName, token1 );
            break;

        case SUMMARY_MISC:
            return miscAddress( vectorName );
            break;

        case SUMMARY_REGION:
            if ( !token1.empty() )
            {
                RiaStdStringTools::toInt( token1, intValue0 );
                return regionAddress( vectorName, intValue0 );
            }
            break;

        case SUMMARY_REGION_2_REGION:
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

        case SUMMARY_GROUP:
            if ( !token1.empty() ) return groupAddress( vectorName, token1 );
            break;

        case SUMMARY_WELL:
            if ( !token1.empty() ) return wellAddress( vectorName, token1 );
            break;

        case SUMMARY_WELL_COMPLETION:
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

        case SUMMARY_WELL_LGR:
            if ( !token1.empty() && !token2.empty() ) return wellLgrAddress( vectorName, token1, token2 );
            break;

        case SUMMARY_WELL_COMPLETION_LGR:
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

        case SUMMARY_WELL_SEGMENT:

            if ( !token2.empty() )
            {
                RiaStdStringTools::toInt( token2, intValue0 );

                return wellSegmentAddress( vectorName, token1, intValue0 );
            }
            break;

        case SUMMARY_BLOCK:
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

        case SUMMARY_BLOCK_LGR:
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

        case SUMMARY_IMPORTED:
        case SUMMARY_INVALID:
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
        case SUMMARY_FIELD:
        case SUMMARY_AQUIFER:
        case SUMMARY_NETWORK:
        case SUMMARY_MISC:
        case SUMMARY_REGION:
        case SUMMARY_REGION_2_REGION:
        case SUMMARY_GROUP:
        case SUMMARY_WELL:
        case SUMMARY_WELL_COMPLETION:
        case SUMMARY_WELL_LGR:
        case SUMMARY_WELL_COMPLETION_LGR:
        case SUMMARY_WELL_SEGMENT:
        case SUMMARY_BLOCK:
        case SUMMARY_BLOCK_LGR:
        case SUMMARY_ENSEMBLE_STATISTICS:
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::baseVectorName( const std::string_view vectorName )
{
    auto tmpString = vectorName;

    if ( tmpString.size() == 8 ) tmpString = tmpString.substr( 0, 5 );

    auto indexToUnderscore = tmpString.find_first_of( '_' );
    if ( indexToUnderscore > 0 )
    {
        tmpString = tmpString.substr( 0, indexToUnderscore );
    }

    return std::string( tmpString );
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
std::tuple<int, int, int> RifEclipseSummaryAddress::ijkTupleFromUiText( const std::string_view s )
{
    QStringList ijk = QString().fromStdString( s.data() ).trimmed().split( QRegExp( "[,]" ) );

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
std::pair<int, int> RifEclipseSummaryAddress::regionToRegionPairFromUiText( const std::string_view s )
{
    QStringList r2r = QString().fromStdString( s.data() ).trimmed().split( QRegExp( "[-]" ) );

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
