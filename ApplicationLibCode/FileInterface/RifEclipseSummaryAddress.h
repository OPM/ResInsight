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

class QTextStream;
class QString;

#define ENSEMBLE_STAT_P10_QUANTITY_NAME "P10"
#define ENSEMBLE_STAT_P50_QUANTITY_NAME "P50"
#define ENSEMBLE_STAT_P90_QUANTITY_NAME "P90"
#define ENSEMBLE_STAT_MEAN_QUANTITY_NAME "MEAN"

//==================================================================================================
//
//
//==================================================================================================
class RifEclipseSummaryAddress
{
public:
    // Based on list in ecl_smspec.c and list of types taken from Eclipse Reference Manual ecl_rm_2011.1.pdf
    enum SummaryVarCategory : int8_t
    {
        SUMMARY_INVALID,
        SUMMARY_FIELD,
        SUMMARY_AQUIFER,
        SUMMARY_NETWORK,
        SUMMARY_MISC,
        SUMMARY_REGION,
        SUMMARY_REGION_2_REGION,
        SUMMARY_GROUP,
        SUMMARY_WELL,
        SUMMARY_WELL_COMPLETION,
        SUMMARY_WELL_LGR,
        SUMMARY_WELL_COMPLETION_LGR,
        SUMMARY_WELL_SEGMENT,
        SUMMARY_BLOCK,
        SUMMARY_BLOCK_LGR,
        SUMMARY_IMPORTED,
        SUMMARY_ENSEMBLE_STATISTICS
    };

    enum SummaryIdentifierType
    {
        INPUT_REGION_NUMBER,
        INPUT_REGION_2_REGION,
        INPUT_WELL_NAME,
        INPUT_GROUP_NAME,
        INPUT_NETWORK_NAME,
        INPUT_CELL_IJK,
        INPUT_LGR_NAME,
        INPUT_SEGMENT_NUMBER,
        INPUT_AQUIFER_NUMBER,
        INPUT_VECTOR_NAME,
        INPUT_ID
    };

public:
    RifEclipseSummaryAddress();

    RifEclipseSummaryAddress( SummaryVarCategory category,
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
                              int                id );

    RifEclipseSummaryAddress( SummaryVarCategory category, std::map<SummaryIdentifierType, std::string>& identifiers );

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
        wellCompletionAddress( const std::string& vectorName, const std::string& wellName, int i, int j, int k, int calculationId = -1 );
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
    static RifEclipseSummaryAddress ensembleStatisticsAddress( const std::string& vectorName, const std::string& datavectorName );

    static std::string generateStringFromAddresses( const std::vector<RifEclipseSummaryAddress>& addressVector,
                                                    const std::string                            jointString = "; " );

    static bool isDependentOnWellName( SummaryVarCategory category );

    // Access methods

    SummaryVarCategory category() const { return m_variableCategory; }
    const std::string  vectorName() const { return m_vectorName; }
    bool               isHistoryVector() const;

    int regionNumber() const { return m_number0; }
    int regionNumber2() const { return m_number1; }

    const std::string groupName() const { return ( m_variableCategory == SUMMARY_GROUP ) ? m_name : std::string(); }
    const std::string networkName() const { return ( m_variableCategory == SUMMARY_NETWORK ) ? m_name : std::string(); }
    const std::string wellName() const { return isDependentOnWellName( m_variableCategory ) ? m_name : std::string(); }
    int                    wellSegmentNumber() const { return m_number0; }
    const std::string lgrName() const { return m_lgrName; }
    int                    cellI() const { return m_number2; }
    int                    cellJ() const { return m_number1; }
    int                    cellK() const { return m_number0; }
    int                    aquiferNumber() const { return m_number0; }
    int                    id() const { return m_id; }
    std::string            blockAsString() const;

    const std::string ensembleStatisticsVectorName() const;

    // Derived properties

    std::string uiText() const;
    std::string itemUiText() const;
    std::string addressComponentUiText( RifEclipseSummaryAddress::SummaryIdentifierType itemTypeInput ) const;
    bool        isUiTextMatchingFilterText( const QString& filterString ) const;

    bool isValid() const;
    void setVectorName( const std::string& vectorName ) { m_vectorName = vectorName; }
    void setWellName( const std::string& wellName ) { m_name = wellName; }
    void setGroupName( const std::string& groupName ) { m_name = groupName; }
    void setNetworkName( const std::string& networkName ) { m_name = networkName; }
    void setRegion( int region ) { m_number0 = region; }
    void setRegion2( int region2 ) { m_number1 = region2; }
    void setAquiferNumber( int aquiferNumber ) { m_number0 = aquiferNumber; }
    void setCellIjk( const std::string& uiText );
    void setWellSegmentNumber( int segment ) { m_number0 = segment; }

    void setAsErrorResult() { m_isErrorResult = true; }
    bool isErrorResult() const { return m_isErrorResult; }

    void setId( int id ) { m_id = id; }

    bool hasAccumulatedData() const;

    static std::string baseVectorName( const std::string& vectorName );

    auto operator<=>( const RifEclipseSummaryAddress& rhs ) const = default;

    bool isCalculated() const;

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

    SummaryVarCategory m_variableCategory;
    std::string        m_vectorName;
    std::string        m_name;
    std::string        m_lgrName;
    int                m_number0;
    int                m_number1;
    int                m_number2;
    bool               m_isErrorResult;
    int                m_id;
};

QTextStream& operator<<( QTextStream& str, const RifEclipseSummaryAddress& sobj );

QTextStream& operator>>( QTextStream& str, RifEclipseSummaryAddress& sobj );
