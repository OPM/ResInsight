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
        SUMMARY_WELL_GROUP,
        SUMMARY_WELL,
        SUMMARY_WELL_COMPLETION,
        SUMMARY_WELL_LGR,
        SUMMARY_WELL_COMPLETION_LGR,
        SUMMARY_WELL_SEGMENT,
        SUMMARY_BLOCK,
        SUMMARY_BLOCK_LGR,
        SUMMARY_CALCULATED,
        SUMMARY_IMPORTED,
        SUMMARY_ENSEMBLE_STATISTICS
    };

    enum SummaryIdentifierType
    {
        INPUT_REGION_NUMBER,
        INPUT_REGION_2_REGION,
        INPUT_WELL_NAME,
        INPUT_WELL_GROUP_NAME,
        INPUT_CELL_IJK,
        INPUT_LGR_NAME,
        INPUT_SEGMENT_NUMBER,
        INPUT_AQUIFER_NUMBER,
        INPUT_VECTOR_NAME,
        INPUT_ID
    };

public:
    RifEclipseSummaryAddress()
        : m_variableCategory( RifEclipseSummaryAddress::SUMMARY_INVALID )
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
    }

    RifEclipseSummaryAddress( SummaryVarCategory category,
                              const std::string& quantityName,
                              int16_t            regionNumber,
                              int16_t            regionNumber2,
                              const std::string& wellGroupName,
                              const std::string& wellName,
                              int16_t            wellSegmentNumber,
                              const std::string& lgrName,
                              int32_t            cellI,
                              int32_t            cellJ,
                              int32_t            cellK,
                              int16_t            aquiferNumber,
                              bool               isErrorResult,
                              int32_t            id )
        : m_variableCategory( category )
        , m_quantityName( quantityName )
        , m_regionNumber( regionNumber )
        , m_regionNumber2( regionNumber2 )
        , m_wellGroupName( wellGroupName )
        , m_wellName( wellName )
        , m_wellSegmentNumber( wellSegmentNumber )
        , m_lgrName( lgrName )
        , m_cellI( cellI )
        , m_cellJ( cellJ )
        , m_cellK( cellK )
        , m_aquiferNumber( aquiferNumber )
        , m_isErrorResult( isErrorResult )
        , m_id( id )
    {
    }

    RifEclipseSummaryAddress( SummaryVarCategory category, std::map<SummaryIdentifierType, std::string>& identifiers );

    // Static specialized creation methods

    static RifEclipseSummaryAddress fromEclipseTextAddress( const std::string& textAddress );
    static SummaryVarCategory       identifyCategory( const std::string& quantityName );

    static RifEclipseSummaryAddress fieldAddress( const std::string& quantityName );
    static RifEclipseSummaryAddress aquiferAddress( const std::string& quantityName, int aquiferNumber );
    static RifEclipseSummaryAddress networkAddress( const std::string& quantityName );
    static RifEclipseSummaryAddress miscAddress( const std::string& quantityName );
    static RifEclipseSummaryAddress regionAddress( const std::string& quantityName, int regionNumber );
    static RifEclipseSummaryAddress
                                    regionToRegionAddress( const std::string& quantityName, int regionNumber, int region2Number );
    static RifEclipseSummaryAddress wellGroupAddress( const std::string& quantityName, const std::string& wellGroupName );
    static RifEclipseSummaryAddress wellAddress( const std::string& quantityName, const std::string& wellName );
    static RifEclipseSummaryAddress
        wellCompletionAddress( const std::string& quantityName, const std::string& wellName, int i, int j, int k );
    static RifEclipseSummaryAddress
                                    wellLgrAddress( const std::string& quantityName, const std::string& lgrName, const std::string& wellName );
    static RifEclipseSummaryAddress wellCompletionLgrAddress( const std::string& quantityName,
                                                              const std::string& lgrName,
                                                              const std::string& wellName,
                                                              int                i,
                                                              int                j,
                                                              int                k );
    static RifEclipseSummaryAddress
                                    wellSegmentAddress( const std::string& quantityName, const std::string& wellName, int segmentNumber );
    static RifEclipseSummaryAddress blockAddress( const std::string& quantityName, int i, int j, int k );
    static RifEclipseSummaryAddress
                                    blockLgrAddress( const std::string& quantityName, const std::string& lgrName, int i, int j, int k );
    static RifEclipseSummaryAddress calculatedAddress( const std::string& quantityName, int id );
    static RifEclipseSummaryAddress importedAddress( const std::string& quantityName );
    static RifEclipseSummaryAddress ensembleStatisticsAddress( const std::string& quantityName,
                                                               const std::string& dataQuantityName );

    static bool isDependentOnWellName( SummaryVarCategory category );

    // Access methods

    SummaryVarCategory category() const
    {
        return m_variableCategory;
    }
    const std::string& quantityName() const
    {
        return m_quantityName;
    }
    bool isHistoryQuantity() const;

    int regionNumber() const
    {
        return m_regionNumber;
    }
    int regionNumber2() const
    {
        return m_regionNumber2;
    }

    const std::string& wellGroupName() const
    {
        return m_wellGroupName;
    }
    const std::string& wellName() const
    {
        return m_wellName;
    }
    int wellSegmentNumber() const
    {
        return m_wellSegmentNumber;
    }
    const std::string& lgrName() const
    {
        return m_lgrName;
    }
    int cellI() const
    {
        return m_cellI;
    }
    int cellJ() const
    {
        return m_cellJ;
    }
    int cellK() const
    {
        return m_cellK;
    }
    int aquiferNumber() const
    {
        return m_aquiferNumber;
    }
    int id() const
    {
        return m_id;
    }
    std::string blockAsString() const;

    const std::string ensembleStatisticsQuantityName() const;

    // Derived properties

    std::string uiText() const;
    std::string uiText( RifEclipseSummaryAddress::SummaryIdentifierType itemTypeInput ) const;
    bool        isUiTextMatchingFilterText( const QString& filterString ) const;

    bool isValid() const;
    void setQuantityName( const std::string& quantity )
    {
        m_quantityName = quantity;
    }
    void setWellName( const std::string& wellName )
    {
        m_wellName = wellName;
    }
    void setWellGroupName( const std::string& wellGroupName )
    {
        m_wellGroupName = wellGroupName;
    }
    void setRegion( int region )
    {
        m_regionNumber = (int16_t)region;
    }
    void setAquiferNumber( int aquiferNumber )
    {
        m_aquiferNumber = (int16_t)aquiferNumber;
    }
    void setCellIjk( const std::string& uiText );
    void setWellSegmentNumber( int segment )
    {
        m_wellSegmentNumber = (int16_t)segment;
    }

    void setAsErrorResult()
    {
        m_isErrorResult = true;
    }
    bool isErrorResult() const
    {
        return m_isErrorResult;
    }

    void setId( int id )
    {
        m_id = id;
    }

    bool hasAccumulatedData() const;

private:
    bool                                         isValidEclipseCategory() const;
    static QString                               baseQuantityName( const QString& quantityName );
    static std::tuple<int32_t, int32_t, int32_t> ijkTupleFromUiText( const std::string& s );
    std::string                                  formatUiTextRegionToRegion() const;
    std::pair<int16_t, int16_t>                  regionToRegionPairFromUiText( const std::string& s );

    std::string        m_quantityName;
    std::string        m_wellGroupName;
    std::string        m_wellName;
    std::string        m_lgrName;
    int32_t            m_cellI;
    int32_t            m_cellJ;
    int32_t            m_cellK;
    int16_t            m_regionNumber;
    int16_t            m_regionNumber2;
    int16_t            m_wellSegmentNumber;
    int16_t            m_aquiferNumber;
    SummaryVarCategory m_variableCategory;
    bool               m_isErrorResult;
    int32_t            m_id;
};

bool operator==( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second );
bool operator!=( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second );

bool operator<( const RifEclipseSummaryAddress& first, const RifEclipseSummaryAddress& second );

QTextStream& operator<<( QTextStream& str, const RifEclipseSummaryAddress& sobj );

QTextStream& operator>>( QTextStream& str, RifEclipseSummaryAddress& sobj );
