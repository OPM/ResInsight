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

#include "RiuSummaryQuantityNameInfoProvider.h"

#include <QStringList>
#include <QTextStream>

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress::RifEclipseSummaryAddress( SummaryVarCategory                            category,
                                                    std::map<SummaryIdentifierType, std::string>& identifiers )
    : m_variableCategory( category )
    , m_regionNumber( -1 )
    , m_regionNumber2( -1 )
    , m_wellSegmentNumber( -1 )
    , m_cellI( -1 )
    , m_cellJ( -1 )
    , m_cellK( -1 )
    , m_aquiferNumber( -1 )
    , m_isErrorResult( false )
    , m_id( -1 )
{
    std::tuple<int32_t, int32_t, int32_t> ijkTuple;
    std::pair<int16_t, int16_t>           reg2regPair;
    switch ( category )
    {
        case SUMMARY_REGION:
            m_regionNumber = RiaStdStringTools::toInt16( identifiers[INPUT_REGION_NUMBER] );
            break;
        case SUMMARY_REGION_2_REGION:
            reg2regPair     = regionToRegionPairFromUiText( identifiers[INPUT_REGION_2_REGION] );
            m_regionNumber  = reg2regPair.first;
            m_regionNumber2 = reg2regPair.second;
            break;
        case SUMMARY_GROUP:
            m_groupName = identifiers[INPUT_GROUP_NAME];
            break;
        case SUMMARY_WELL:
            m_wellName = identifiers[INPUT_WELL_NAME];
            break;
        case SUMMARY_WELL_COMPLETION:
            m_wellName = identifiers[INPUT_WELL_NAME];
            ijkTuple   = ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] );
            m_cellI    = std::get<0>( ijkTuple );
            m_cellJ    = std::get<1>( ijkTuple );
            m_cellK    = std::get<2>( ijkTuple );
            break;
        case SUMMARY_WELL_LGR:
            m_lgrName  = identifiers[INPUT_LGR_NAME];
            m_wellName = identifiers[INPUT_WELL_NAME];
            break;
        case SUMMARY_WELL_COMPLETION_LGR:
            m_lgrName  = identifiers[INPUT_LGR_NAME];
            m_wellName = identifiers[INPUT_WELL_NAME];
            ijkTuple   = ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] );
            m_cellI    = std::get<0>( ijkTuple );
            m_cellJ    = std::get<1>( ijkTuple );
            m_cellK    = std::get<2>( ijkTuple );
            break;
        case SUMMARY_WELL_SEGMENT:
            m_wellName          = identifiers[INPUT_WELL_NAME];
            m_wellSegmentNumber = RiaStdStringTools::toInt( identifiers[INPUT_SEGMENT_NUMBER] );
            break;
        case SUMMARY_BLOCK:
            ijkTuple = ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] );
            m_cellI  = std::get<0>( ijkTuple );
            m_cellJ  = std::get<1>( ijkTuple );
            m_cellK  = std::get<2>( ijkTuple );
            break;
        case SUMMARY_BLOCK_LGR:
            m_lgrName = identifiers[INPUT_LGR_NAME];
            ijkTuple  = ijkTupleFromUiText( identifiers[INPUT_CELL_IJK] );
            m_cellI   = std::get<0>( ijkTuple );
            m_cellJ   = std::get<1>( ijkTuple );
            m_cellK   = std::get<2>( ijkTuple );
            break;
        case SUMMARY_AQUIFER:
            m_aquiferNumber = RiaStdStringTools::toInt( identifiers[INPUT_AQUIFER_NUMBER] );
            break;
        case SUMMARY_CALCULATED:
            m_id = RiaStdStringTools::toInt( identifiers[INPUT_ID] );
            break;
    }

    m_vectorName = identifiers[INPUT_VECTOR_NAME];
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
RifEclipseSummaryAddress::SummaryVarCategory RifEclipseSummaryAddress::identifyCategory( const std::string& vectorName )
{
    // Try to an exact match on the vector name first in the vector table.
    bool exactMatch    = true;
    auto exactCategory = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromVectorName( vectorName, exactMatch );
    if ( exactCategory != SUMMARY_INVALID ) return exactCategory;

    if ( vectorName.size() < 3 || vectorName.size() > 8 ) return SUMMARY_INVALID;

    // Try to match the base vector name with more heuristics
    auto strippedQuantityName = baseVectorName( vectorName );

    // First, try to lookup vector in vector table
    auto category = RiuSummaryQuantityNameInfoProvider::instance()->categoryFromVectorName( strippedQuantityName );
    if ( category != SUMMARY_INVALID ) return category;

    if ( strippedQuantityName[0] == 'N' ) return SUMMARY_NETWORK;
    if ( strippedQuantityName[0] == 'W' ) return SUMMARY_WELL;

    if ( strippedQuantityName[0] == 'R' )
    {
        // Taken from
        // \ThirdParty\custom-opm-common\opm-common\src\opm\parser\eclipse\EclipseState\SummaryConfig\SummaryConfig.cpp

        auto is_region_to_region = []( const std::string& keyword ) {
            using sz_t = std::string::size_type;
            if ( ( keyword.size() == sz_t{ 3 } ) && keyword[2] == 'F' ) return true;
            if ( ( keyword == "RNLF" ) || ( keyword == "RORFR" ) ) return true;
            if ( ( keyword.size() >= sz_t{ 4 } ) &&
                 ( ( keyword[2] == 'F' ) && ( ( keyword[3] == 'T' ) || ( keyword[3] == 'R' ) ) ) )
                return true;
            if ( ( keyword.size() >= sz_t{ 5 } ) &&
                 ( ( keyword[3] == 'F' ) && ( ( keyword[4] == 'T' ) || ( keyword[4] == 'R' ) ) ) )
                return true;

            return false;
        };

        if ( is_region_to_region( strippedQuantityName ) ) return SUMMARY_REGION_2_REGION;

        return SUMMARY_REGION;
    }

    // Then check LGR categories
    std::string firstTwoLetters = strippedQuantityName.substr( 0, 2 );

    if ( firstTwoLetters == "LB" ) return SUMMARY_BLOCK_LGR;
    if ( firstTwoLetters == "LC" ) return SUMMARY_WELL_COMPLETION_LGR;
    if ( firstTwoLetters == "LW" ) return SUMMARY_WELL_LGR;

    return SUMMARY_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::fieldAddress( const std::string& vectorName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_FIELD;
    addr.m_vectorName       = vectorName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::aquiferAddress( const std::string& vectorName, int aquiferNumber )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_AQUIFER;
    addr.m_vectorName       = vectorName;
    addr.m_aquiferNumber    = aquiferNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::networkAddress( const std::string& vectorName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_NETWORK;
    addr.m_vectorName       = vectorName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::miscAddress( const std::string& vectorName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_MISC;
    addr.m_vectorName       = vectorName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::regionAddress( const std::string& vectorName, int regionNumber )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION;
    addr.m_vectorName       = vectorName;
    addr.m_regionNumber     = regionNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::regionToRegionAddress( const std::string& vectorName, int regionNumber, int region2Number )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_REGION_2_REGION;
    addr.m_vectorName       = vectorName;
    addr.m_regionNumber     = regionNumber;
    addr.m_regionNumber2    = region2Number;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::groupAddress( const std::string& vectorName, const std::string& groupName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_GROUP;
    addr.m_vectorName       = vectorName;
    addr.m_groupName        = groupName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellAddress( const std::string& vectorName, const std::string& wellName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL;
    addr.m_vectorName       = vectorName;
    addr.m_wellName         = wellName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellCompletionAddress( const std::string& vectorName,
                                                                          const std::string& wellName,
                                                                          int                i,
                                                                          int                j,
                                                                          int                k )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION;
    addr.m_vectorName       = vectorName;
    addr.m_wellName         = wellName;
    addr.m_cellI            = i;
    addr.m_cellJ            = j;
    addr.m_cellK            = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellLgrAddress( const std::string& vectorName,
                                                                   const std::string& lgrName,
                                                                   const std::string& wellName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.m_wellName         = wellName;
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
                                                                             int                k )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_WELL_COMPLETION_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.m_wellName         = wellName;
    addr.m_cellI            = i;
    addr.m_cellJ            = j;
    addr.m_cellK            = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::wellSegmentAddress( const std::string& vectorName,
                                                                       const std::string& wellName,
                                                                       int                segmentNumber )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory  = SUMMARY_WELL_SEGMENT;
    addr.m_vectorName        = vectorName;
    addr.m_wellName          = wellName;
    addr.m_wellSegmentNumber = segmentNumber;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::blockAddress( const std::string& vectorName, int i, int j, int k )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK;
    addr.m_vectorName       = vectorName;
    addr.m_cellI            = i;
    addr.m_cellJ            = j;
    addr.m_cellK            = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RifEclipseSummaryAddress::blockLgrAddress( const std::string& vectorName, const std::string& lgrName, int i, int j, int k )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_BLOCK_LGR;
    addr.m_vectorName       = vectorName;
    addr.m_lgrName          = lgrName;
    addr.m_cellI            = i;
    addr.m_cellJ            = j;
    addr.m_cellK            = k;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::calculatedAddress( const std::string& vectorName, int id )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_CALCULATED;
    addr.m_vectorName       = vectorName;
    addr.m_id               = id;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::importedAddress( const std::string& vectorName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_IMPORTED;
    addr.m_vectorName       = vectorName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifEclipseSummaryAddress::ensembleStatisticsAddress( const std::string& vectorName,
                                                                              const std::string& dataQuantityName )
{
    RifEclipseSummaryAddress addr;
    addr.m_variableCategory = SUMMARY_ENSEMBLE_STATISTICS;
    addr.m_vectorName       = vectorName + ":" + dataQuantityName;
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclipseSummaryAddress::generateStringFromAddresses( const std::vector<RifEclipseSummaryAddress>& addressVector,
                                                                   const std::string jointString )
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
    // clang-format off
    if (category == SUMMARY_WELL ||
        category == SUMMARY_WELL_COMPLETION ||
        category == SUMMARY_WELL_COMPLETION_LGR ||
        category == SUMMARY_WELL_LGR ||
        category == SUMMARY_WELL_SEGMENT)
    {
        return true;
    }

    // clang-format on

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

    switch ( this->category() )
    {
        case SUMMARY_REGION:
        {
            text += std::to_string( this->regionNumber() );
        }
        break;
        case SUMMARY_REGION_2_REGION:
        {
            text += formatUiTextRegionToRegion();
        }
        break;
        case SUMMARY_GROUP:
        {
            text += this->groupName();
        }
        break;
        case SUMMARY_WELL:
        {
            text += this->wellName();
        }
        break;
        case SUMMARY_WELL_COMPLETION:
        {
            text += this->wellName();
            text += ":" + blockAsString();
        }
        break;
        case SUMMARY_WELL_LGR:
        {
            text += this->lgrName();
            text += ":" + this->wellName();
        }
        break;
        case SUMMARY_WELL_COMPLETION_LGR:
        {
            text += this->lgrName();
            text += ":" + this->wellName();
            text += ":" + blockAsString();
        }
        break;
        case SUMMARY_WELL_SEGMENT:
        {
            text += this->wellName();
            text += ":" + std::to_string( this->wellSegmentNumber() );
        }
        break;
        case SUMMARY_BLOCK:
        {
            text += blockAsString();
        }
        break;
        case SUMMARY_BLOCK_LGR:
        {
            text += this->lgrName();
            text += ":" + blockAsString();
        }
        break;
        case SUMMARY_AQUIFER:
        {
            text += std::to_string( this->aquiferNumber() );
        }
        break;
        case SUMMARY_CALCULATED:
        {
            text += std::to_string( this->id() );
        }
        break;
        case SUMMARY_IMPORTED:
        {
            text += this->vectorName();
        }
        break;
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// Returns the stringified address component requested
//--------------------------------------------------------------------------------------------------
std::string
    RifEclipseSummaryAddress::addressComponentUiText( RifEclipseSummaryAddress::SummaryIdentifierType identifierType ) const
{
    switch ( identifierType )
    {
        case INPUT_REGION_NUMBER:
            return std::to_string( regionNumber() );
        case INPUT_REGION_2_REGION:
            return formatUiTextRegionToRegion();
        case INPUT_WELL_NAME:
            return wellName();
        case INPUT_GROUP_NAME:
            return groupName();
        case INPUT_CELL_IJK:
            return blockAsString();
        case INPUT_LGR_NAME:
            return lgrName();
        case INPUT_SEGMENT_NUMBER:
            return std::to_string( wellSegmentNumber() );
        case INPUT_AQUIFER_NUMBER:
            return std::to_string( aquiferNumber() );
        case INPUT_VECTOR_NAME:
            return vectorName();
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
            if ( m_regionNumber == -1 ) return false;
            return true;

        case SUMMARY_REGION_2_REGION:
            if ( m_regionNumber == -1 ) return false;
            if ( m_regionNumber2 == -1 ) return false;
            return true;

        case SUMMARY_GROUP:
            if ( m_groupName.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL:
            if ( m_wellName.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL_COMPLETION:
            if ( m_wellName.size() == 0 ) return false;
            if ( m_cellI == -1 ) return false;
            if ( m_cellJ == -1 ) return false;
            if ( m_cellK == -1 ) return false;
            return true;

        case SUMMARY_WELL_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_wellName.size() == 0 ) return false;
            return true;

        case SUMMARY_WELL_COMPLETION_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_wellName.size() == 0 ) return false;
            if ( m_cellI == -1 ) return false;
            if ( m_cellJ == -1 ) return false;
            if ( m_cellK == -1 ) return false;
            return true;

        case SUMMARY_WELL_SEGMENT:
            if ( m_wellName.size() == 0 ) return false;
            if ( m_wellSegmentNumber == -1 ) return false;
            return true;

        case SUMMARY_BLOCK:
            if ( m_cellI == -1 ) return false;
            if ( m_cellJ == -1 ) return false;
            if ( m_cellK == -1 ) return false;
            return true;

        case SUMMARY_BLOCK_LGR:
            if ( m_lgrName.size() == 0 ) return false;
            if ( m_cellI == -1 ) return false;
            if ( m_cellJ == -1 ) return false;
            if ( m_cellK == -1 ) return false;
            return true;

        case SUMMARY_AQUIFER:
            if ( m_aquiferNumber == -1 ) return false;
            return true;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryAddress::setCellIjk( const std::string& uiText )
{
    auto vec = RifEclipseSummaryAddress::ijkTupleFromUiText( uiText );

    m_cellI = std::get<0>( vec );
    m_cellJ = std::get<1>( vec );
    m_cellK = std::get<2>( vec );
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
    if ( tokens.empty() )
    {
        return RifEclipseSummaryAddress();
    }

    std::string vectorName;
    std::string token1;
    std::string token2;

    vectorName = tokens[0];

    if ( tokens.size() > 1 ) token1 = tokens[1];
    if ( tokens.size() > 2 ) token2 = tokens[2];

    SummaryVarCategory category = identifyCategory( vectorName );

    switch ( category )
    {
        case SUMMARY_FIELD:
            return fieldAddress( vectorName );

        case SUMMARY_AQUIFER:
            if ( !token1.empty() ) return aquiferAddress( vectorName, RiaStdStringTools::toInt( token1 ) );
            break;

        case SUMMARY_NETWORK:
            return networkAddress( vectorName );
            break;

        case SUMMARY_MISC:
            return miscAddress( vectorName );
            break;

        case SUMMARY_REGION:
            if ( !token1.empty() ) return regionAddress( vectorName, RiaStdStringTools::toInt( token1 ) );
            break;

        case SUMMARY_REGION_2_REGION:
            if ( !token1.empty() )
            {
                auto regions = RiaStdStringTools::splitString( token1, '-' );
                if ( regions.size() == 2 )
                {
                    return regionToRegionAddress( vectorName,
                                                  RiaStdStringTools::toInt( regions[0] ),
                                                  RiaStdStringTools::toInt( regions[1] ) );
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
                    return wellCompletionAddress( vectorName,
                                                  token1,
                                                  RiaStdStringTools::toInt( ijk[0] ),
                                                  RiaStdStringTools::toInt( ijk[1] ),
                                                  RiaStdStringTools::toInt( ijk[2] ) );
                }
            }
            break;

        case SUMMARY_WELL_LGR:
            if ( !token1.empty() && !token2.empty() ) return wellLgrAddress( vectorName, token1, token2 );
            break;

        case SUMMARY_WELL_COMPLETION_LGR:
            if ( tokens.size() > 2 )
            {
                auto token3 = tokens[3];
                auto ijk    = RiaStdStringTools::splitString( token3, ',' );
                if ( ijk.size() == 3 )
                {
                    return wellCompletionLgrAddress( vectorName,
                                                     token1,
                                                     token2,
                                                     RiaStdStringTools::toInt( ijk[0] ),
                                                     RiaStdStringTools::toInt( ijk[1] ),
                                                     RiaStdStringTools::toInt( ijk[2] ) );
                }
            }
            break;

        case SUMMARY_WELL_SEGMENT:
            if ( !token2.empty() ) return wellSegmentAddress( vectorName, token1, RiaStdStringTools::toInt( token2 ) );
            break;

        case SUMMARY_BLOCK:
            if ( !token1.empty() )
            {
                auto ijk = RiaStdStringTools::splitString( token1, ',' );
                if ( ijk.size() == 3 )
                {
                    return blockAddress( vectorName,
                                         RiaStdStringTools::toInt( ijk[0] ),
                                         RiaStdStringTools::toInt( ijk[1] ),
                                         RiaStdStringTools::toInt( ijk[2] ) );
                }
            }
            break;

        case SUMMARY_BLOCK_LGR:
            if ( !token2.empty() )
            {
                auto ijk = RiaStdStringTools::splitString( token2, ',' );
                if ( ijk.size() == 3 )
                {
                    return blockLgrAddress( vectorName,
                                            token1,
                                            RiaStdStringTools::toInt( ijk[0] ),
                                            RiaStdStringTools::toInt( ijk[1] ),
                                            RiaStdStringTools::toInt( ijk[2] ) );
                }
            }
            break;

        case SUMMARY_IMPORTED:
        case SUMMARY_INVALID:
        default:
            break;
    }

    return RifEclipseSummaryAddress();
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
    return std::to_string( this->cellI() ) + ", " + std::to_string( this->cellJ() ) + ", " +
           std::to_string( this->cellK() );
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
    return std::to_string( this->regionNumber() ) + " - " + std::to_string( this->regionNumber2() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int16_t, int16_t> RifEclipseSummaryAddress::regionToRegionPairFromUiText( const std::string& s )
{
    QStringList r2r = QString().fromStdString( s ).trimmed().split( QRegExp( "[-]" ) );

    if ( r2r.size() != 2 ) return std::make_pair( (int16_t)-1, (int16_t)-1 );

    return std::make_pair( RiaStdStringTools::toInt16( r2r[0].trimmed().toStdString() ),
                           RiaStdStringTools::toInt16( r2r[1].trimmed().toStdString() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator==( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second )
{
    if ( first.category() != second.category() ) return false;
    if ( first.vectorName() != second.vectorName() ) return false;
    switch ( first.category() )
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if ( first.regionNumber() != second.regionNumber() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if ( first.regionNumber() != second.regionNumber() ) return false;
            if ( first.regionNumber2() != second.regionNumber2() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_GROUP:
        {
            if ( first.groupName() != second.groupName() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            if ( first.wellName() != second.wellName() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            if ( first.wellName() != second.wellName() ) return false;
            if ( first.cellI() != second.cellI() ) return false;
            if ( first.cellJ() != second.cellJ() ) return false;
            if ( first.cellK() != second.cellK() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            if ( first.wellName() != second.wellName() ) return false;
            if ( first.lgrName() != second.lgrName() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            if ( first.wellName() != second.wellName() ) return false;
            if ( first.lgrName() != second.lgrName() ) return false;
            if ( first.cellI() != second.cellI() ) return false;
            if ( first.cellJ() != second.cellJ() ) return false;
            if ( first.cellK() != second.cellK() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            if ( first.wellName() != second.wellName() ) return false;
            if ( first.wellSegmentNumber() != second.wellSegmentNumber() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if ( first.cellI() != second.cellI() ) return false;
            if ( first.cellJ() != second.cellJ() ) return false;
            if ( first.cellK() != second.cellK() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            if ( first.lgrName() != second.lgrName() ) return false;
            if ( first.cellI() != second.cellI() ) return false;
            if ( first.cellJ() != second.cellJ() ) return false;
            if ( first.cellK() != second.cellK() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            if ( first.aquiferNumber() != second.aquiferNumber() ) return false;
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_CALCULATED:
        {
            if ( first.id() != second.id() ) return false;
        }
        break;
    }
    if ( first.isErrorResult() != second.isErrorResult() ) return false;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator!=( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second )
{
    return !operator==( first, second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator<( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second )
{
    if ( first.vectorName() != second.vectorName() ) return first.vectorName() < second.vectorName();

    switch ( first.category() )
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            if ( first.regionNumber() != second.regionNumber() ) return first.regionNumber() < second.regionNumber();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            if ( first.regionNumber() != second.regionNumber() ) return first.regionNumber() < second.regionNumber();
            if ( first.regionNumber2() != second.regionNumber2() )
                return first.regionNumber2() < second.regionNumber2();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_GROUP:
        {
            if ( first.groupName() != second.groupName() ) return first.groupName() < second.groupName();
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
            if ( first.cellI() != second.cellI() ) return ( first.cellI() < second.cellI() );
            if ( first.cellJ() != second.cellJ() ) return ( first.cellJ() < second.cellJ() );
            if ( first.cellK() != second.cellK() ) return ( first.cellK() < second.cellK() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
            if ( first.lgrName() != second.lgrName() ) return ( first.lgrName() < second.lgrName() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
            if ( first.lgrName() != second.lgrName() ) return ( first.lgrName() < second.lgrName() );
            if ( first.cellI() != second.cellI() ) return ( first.cellI() < second.cellI() );
            if ( first.cellJ() != second.cellJ() ) return ( first.cellJ() < second.cellJ() );
            if ( first.cellK() != second.cellK() ) return ( first.cellK() < second.cellK() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
            if ( first.wellSegmentNumber() != second.wellSegmentNumber() )
                return ( first.wellSegmentNumber() < second.wellSegmentNumber() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            if ( first.cellI() != second.cellI() ) return ( first.cellI() < second.cellI() );
            if ( first.cellJ() != second.cellJ() ) return ( first.cellJ() < second.cellJ() );
            if ( first.cellK() != second.cellK() ) return ( first.cellK() < second.cellK() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            if ( first.lgrName() != second.lgrName() ) return ( first.lgrName() < second.lgrName() );
            if ( first.cellI() != second.cellI() ) return ( first.cellI() < second.cellI() );
            if ( first.cellJ() != second.cellJ() ) return ( first.cellJ() < second.cellJ() );
            if ( first.cellK() != second.cellK() ) return ( first.cellK() < second.cellK() );
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        {
            if ( first.aquiferNumber() != second.aquiferNumber() )
                return first.aquiferNumber() < second.aquiferNumber();
        }
        break;
    }
    if ( first.isErrorResult() != second.isErrorResult() ) return first.isErrorResult() < second.isErrorResult();
    return false;
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
