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
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "RifEclipseSummaryAddressDefines.h"
using namespace RifEclipseSummaryAddressDefines;

class QTextStream;
class QString;

//==================================================================================================
//
//
//==================================================================================================
class RifEclipseSummaryAddress
{
public:
    RifEclipseSummaryAddress();

    RifEclipseSummaryAddress( SummaryCategory    category,
                              StatisticsType     statisticsType,
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
                              int                completionNumber,
                              bool               isErrorResult,
                              int                id );

    RifEclipseSummaryAddress( SummaryCategory category, std::map<SummaryIdentifierType, std::string>& identifiers );

    // Static specialized creation methods

    static RifEclipseSummaryAddress fromEclipseTextAddress( const std::string& textAddress );
    static RifEclipseSummaryAddress fromEclipseTextAddressParseErrorTokens( const std::string& textAddress );

    static RifEclipseSummaryAddress fieldAddress( const std::string& vectorName, int calculationId = -1 );
    static RifEclipseSummaryAddress aquiferAddress( const std::string& vectorName, int aquiferNumber, int calculationId = -1 );
    static RifEclipseSummaryAddress networkAddress( const std::string& vectorName, const std::string& networkName, int calculationId = -1 );
    static RifEclipseSummaryAddress miscAddress( const std::string& vectorName, int calculationId = -1 );
    static RifEclipseSummaryAddress regionAddress( const std::string& vectorName, int regionNumber, int calculationId = -1 );
    static RifEclipseSummaryAddress
        regionToRegionAddress( const std::string& vectorName, int regionNumber, int region2Number, int calculationId = -1 );
    static RifEclipseSummaryAddress groupAddress( const std::string& vectorName, const std::string& groupName, int calculationId = -1 );
    static RifEclipseSummaryAddress wellAddress( const std::string& vectorName, const std::string& wellName, int calculationId = -1 );
    static RifEclipseSummaryAddress
        wellCompletionAddress( const std::string& vectorName, const std::string& wellName, int completionNumber, int calculationId = -1 );

    static RifEclipseSummaryAddress
        wellConnectionAddress( const std::string& vectorName, const std::string& wellName, int i, int j, int k, int calculationId = -1 );
    static RifEclipseSummaryAddress
        wellLgrAddress( const std::string& vectorName, const std::string& lgrName, const std::string& wellName, int calculationId = -1 );
    static RifEclipseSummaryAddress wellCompletionLgrAddress( const std::string& vectorName,
                                                              const std::string& lgrName,
                                                              const std::string& wellName,
                                                              int                i,
                                                              int                j,
                                                              int                k,
                                                              int                calculationId = -1 );
    static RifEclipseSummaryAddress
        wellSegmentAddress( const std::string& vectorName, const std::string& wellName, int segmentNumber, int calculationId = -1 );
    static RifEclipseSummaryAddress blockAddress( const std::string& vectorName, int i, int j, int k, int calculationId = -1 );
    static RifEclipseSummaryAddress
        blockLgrAddress( const std::string& vectorName, const std::string& lgrName, int i, int j, int k, int calculationId = -1 );
    static RifEclipseSummaryAddress importedAddress( const std::string& vectorName, int calculationId = -1 );

    // Special address when time is used as x-axis
    static RifEclipseSummaryAddress timeAddress();

    static std::string generateStringFromAddresses( const std::vector<RifEclipseSummaryAddress>& addressVector,
                                                    const std::string                            jointString = "; " );

    static bool isDependentOnWellName( SummaryCategory category );

    // Access methods

    SummaryCategory category() const;
    std::string     vectorName() const;
    bool            isHistoryVector() const;

    bool           isStatistics() const;
    StatisticsType statisticsType() const;
    void           setStatisticsType( StatisticsType type );

    int regionNumber() const;
    int regionNumber2() const;

    std::string groupName() const;
    std::string networkName() const;
    std::string wellName() const;
    int         wellCompletionNumber() const;
    int         wellSegmentNumber() const;
    std::string lgrName() const;
    int         cellI() const;
    int         cellJ() const;
    int         cellK() const;
    int         aquiferNumber() const;
    int         id() const;
    std::string blockAsString() const;
    std::string connectionAsString() const;

    std::string toEclipseTextAddress() const;

    // Derived properties

    std::string uiText() const;
    std::string itemUiText() const;
    std::string addressComponentUiText( SummaryIdentifierType itemTypeInput ) const;
    bool        isUiTextMatchingFilterText( const QString& filterString ) const;

    bool isValid() const;
    void setVectorName( const std::string& vectorName );
    void setWellName( const std::string& wellName );
    void setGroupName( const std::string& groupName );
    void setNetworkName( const std::string& networkName );
    void setRegion( int region );
    void setRegion2( int region2 );
    void setAquiferNumber( int aquiferNumber );
    void setCellIjk( const std::string& uiText );
    void setWellSegmentNumber( int segment );
    void setWellCompletionNumber( int completionNumber );

    void setAsErrorResult();
    bool isErrorResult() const;

    void setId( int id );

    bool hasAccumulatedData() const;

    static std::string baseVectorName( const std::string& vectorName );

    auto operator<=>( const RifEclipseSummaryAddress& rhs ) const = default;

    bool isCalculated() const;
    bool isTime() const;

    std::string                formatUiTextRegionToRegion() const;
    static std::pair<int, int> regionToRegionPairFromUiText( const std::string& s );

private:
    static RifEclipseSummaryAddress fromTokens( const std::vector<std::string>& tokens );

    bool                             isValidEclipseCategory() const;
    static std::tuple<int, int, int> ijkTupleFromUiText( const std::string& s );
    void                             setCellIjk( std::tuple<int, int, int> ijk );
    void                             setCellIjk( int i, int j, int k );

private:
    // The ordering the variables are defined in defines how the objects get sorted. Members defined first will be
    // evaluated first. This concept is used by <=> operator.

    SummaryCategory m_category;
    StatisticsType  m_statisticsType;
    std::string     m_vectorName;
    std::string     m_name;
    std::string     m_lgrName;
    int             m_number0;
    int             m_number1;
    int             m_number2;
    bool            m_isErrorResult;
    int             m_id;
};

QTextStream& operator<<( QTextStream& str, const RifEclipseSummaryAddress& sobj );
QTextStream& operator>>( QTextStream& str, RifEclipseSummaryAddress& sobj );
