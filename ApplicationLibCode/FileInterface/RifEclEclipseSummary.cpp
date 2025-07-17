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

#include "RifEclEclipseSummary.h"

#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaStringEncodingTools.h"

#include "RifEclipseSummaryTools.h"
#include "RifOpmCommonSummary.h"
#include "RifReaderEclipseOutput.h"

#include <cassert>
#include <regex>
#include <string>

#include <QDateTime>
#include <QDir>
#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclEclipseSummary::RifEclEclipseSummary()
    : m_ecl_sum( nullptr )
    , m_ecl_SmSpec( nullptr )
    , m_unitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclEclipseSummary::~RifEclEclipseSummary()
{
    if ( m_ecl_sum )
    {
        RifEclipseSummaryTools::closeEclSum( m_ecl_sum );

        m_ecl_sum = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclEclipseSummary::open( const QString& headerFileName, RiaThreadSafeLogger* threadSafeLogger )
{
    assert( m_ecl_sum == nullptr );

    m_ecl_sum = RifEclipseSummaryTools::openEclSum( headerFileName, false );
    if ( m_ecl_sum )
    {
        m_timeSteps.clear();
        m_ecl_SmSpec = ecl_sum_get_smspec( m_ecl_sum );
        m_timeSteps  = RifEclipseSummaryTools::getTimeSteps( m_ecl_sum );
        m_unitSystem = RifEclipseSummaryTools::readUnitSystem( m_ecl_sum );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string stringFromPointer( const char* pointerToChar )
{
    std::string myString;

    // NB! Assigning a null pointer to a std::string causes runtime crash
    if ( pointerToChar )
    {
        myString = pointerToChar;

        replace( myString.begin(), myString.end(), '\t', ' ' );
    }

    return myString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress addressFromErtSmSpecNode( const ecl::smspec_node& ertSumVarNode )
{
    if ( ertSumVarNode.get_var_type() == ECL_SMSPEC_INVALID_VAR )
    {
        return RifEclipseSummaryAddress();
    }

    RifEclipseSummaryAddressDefines::SummaryCategory sumCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID );
    std::string                                      quantityName;
    int                                              regionNumber( -1 );
    int                                              regionNumber2( -1 );
    std::string                                      groupName;
    std::string                                      networkName;
    std::string                                      wellName;
    int                                              wellSegmentNumber( -1 );
    std::string                                      lgrName;
    int                                              cellI( -1 );
    int                                              cellJ( -1 );
    int                                              cellK( -1 );
    int                                              aquiferNumber( -1 );
    int                                              wellCompletionNumber( -1 );
    bool                                             isErrorResult( false );
    int                                              id( -1 );

    quantityName = stringFromPointer( ertSumVarNode.get_keyword() );

    switch ( ertSumVarNode.get_var_type() )
    {
        case ECL_SMSPEC_AQUIFER_VAR:
        {
            sumCategory   = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER;
            aquiferNumber = ertSumVarNode.get_num();
        }
        break;
        case ECL_SMSPEC_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL;
            wellName    = stringFromPointer( ertSumVarNode.get_wgname() );
        }
        break;
        case ECL_SMSPEC_REGION_VAR:
        {
            sumCategory  = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION;
            regionNumber = ertSumVarNode.get_num();
        }
        break;
        case ECL_SMSPEC_FIELD_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD;
        }
        break;
        case ECL_SMSPEC_GROUP_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP;
            groupName   = stringFromPointer( ertSumVarNode.get_wgname() );
        }
        break;
        case ECL_SMSPEC_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK;

            auto ijk = ertSumVarNode.get_ijk();
            cellI    = ijk[0];
            cellJ    = ijk[1];
            cellK    = ijk[2];
        }
        break;
        case ECL_SMSPEC_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION;
            wellName    = stringFromPointer( ertSumVarNode.get_wgname() );

            auto ijk = ertSumVarNode.get_ijk();
            cellI    = ijk[0];
            cellJ    = ijk[1];
            cellK    = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK_LGR;
            lgrName     = stringFromPointer( ertSumVarNode.get_lgr_name() );

            auto ijk = ertSumVarNode.get_lgr_ijk();
            cellI    = ijk[0];
            cellJ    = ijk[1];
            cellK    = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION_LGR;
            wellName    = stringFromPointer( ertSumVarNode.get_wgname() );
            lgrName     = stringFromPointer( ertSumVarNode.get_lgr_name() );

            auto ijk = ertSumVarNode.get_lgr_ijk();
            cellI    = ijk[0];
            cellJ    = ijk[1];
            cellK    = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_LGR;
            wellName    = stringFromPointer( ertSumVarNode.get_wgname() );
            lgrName     = stringFromPointer( ertSumVarNode.get_lgr_name() );
        }
        break;
        case ECL_SMSPEC_NETWORK_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK;
        }
        break;
        case ECL_SMSPEC_REGION_2_REGION_VAR:
        {
            sumCategory   = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION;
            regionNumber  = ertSumVarNode.get_R1();
            regionNumber2 = ertSumVarNode.get_R2();
        }
        break;
        case ECL_SMSPEC_SEGMENT_VAR:
        {
            sumCategory       = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT;
            wellName          = stringFromPointer( ertSumVarNode.get_wgname() );
            wellSegmentNumber = ertSumVarNode.get_num();
        }
        break;
        case ECL_SMSPEC_MISC_VAR:
        {
            sumCategory = RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_MISC;
        }
        break;
        default:
            CVF_ASSERT( false );
            break;
    }

    return RifEclipseSummaryAddress( sumCategory,
                                     RifEclipseSummaryAddressDefines::StatisticsType::NONE,
                                     quantityName,
                                     regionNumber,
                                     regionNumber2,
                                     groupName,
                                     networkName,
                                     wellName,
                                     wellSegmentNumber,
                                     lgrName,
                                     cellI,
                                     cellJ,
                                     cellK,
                                     aquiferNumber,
                                     wellCompletionNumber,
                                     isErrorResult,
                                     id );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifEclEclipseSummary::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_timeSteps.empty() ) return { true, {} };

    std::vector<double> values;
    values.reserve( m_timeSteps.size() );

    if ( m_ecl_SmSpec )
    {
        int variableIndex = indexFromAddress( resultAddress );
        if ( variableIndex < 0 ) return { false, {} };

        const ecl::smspec_node& ertSumVarNode = ecl_smspec_iget_node_w_node_index( m_ecl_SmSpec, variableIndex );
        int                     paramsIndex   = ertSumVarNode.get_params_index();

        double_vector_type* dataValues = ecl_sum_alloc_data_vector( m_ecl_sum, paramsIndex, false );

        if ( dataValues )
        {
            int           dataSize = double_vector_size( dataValues );
            const double* dataPtr  = double_vector_get_const_ptr( dataValues );
            values.insert( values.end(), dataPtr, dataPtr + dataSize );
            double_vector_free( dataValues );
        }
    }

    return { true, values };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifEclEclipseSummary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclEclipseSummary::indexFromAddress( const RifEclipseSummaryAddress& resultAddress ) const
{
    auto it = m_resultAddressToErtNodeIdx.find( resultAddress );
    if ( it != m_resultAddressToErtNodeIdx.end() )
    {
        return it->second;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclEclipseSummary::createAndSetAddresses()
{
    m_allResultAddresses.clear();
    m_resultAddressToErtNodeIdx.clear();

    if ( m_ecl_SmSpec )
    {
        int varCount = ecl_smspec_num_nodes( m_ecl_SmSpec );
        for ( int i = 0; i < varCount; i++ )
        {
            const ecl::smspec_node& ertSumVarNode = ecl_smspec_iget_node_w_node_index( m_ecl_SmSpec, i );

            RifEclipseSummaryAddress addr = addressFromErtSmSpecNode( ertSumVarNode );

            if ( addr.category() == SummaryCategory::SUMMARY_WELL && ertSumVarNode.get_gen_key1() )
            {
                auto nativeAddress     = std::string( ertSumVarNode.get_gen_key1() );
                auto normalizedAddress = normalizeCompletionAddress( nativeAddress );

                auto adrFromTextParsing = RifEclipseSummaryAddress::fromEclipseTextAddress( normalizedAddress );

                bool debugOutput = false;
                if ( debugOutput )
                {
                    if ( addr != adrFromTextParsing )
                    {
                        auto ertAdrText  = addr.toEclipseTextAddress();
                        auto adrFromText = adrFromTextParsing.toEclipseTextAddress();

                        QString detectedInconsiteny =
                            QString( "Full text from ERT: %1, Address from ERT: %2, Address from text parsing: %3 " )
                                .arg( QString::fromStdString( nativeAddress ) )
                                .arg( QString::fromStdString( ertAdrText ) )
                                .arg( QString::fromStdString( adrFromText ) );

                        RiaLogging::debug( detectedInconsiteny );
                    }
                }

                if ( adrFromTextParsing.isValid() )
                {
                    addr = adrFromTextParsing;
                }
            }

            m_allResultAddresses.insert( addr );
            m_resultAddressToErtNodeIdx[addr] = i;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifEclEclipseSummary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !m_ecl_SmSpec ) return "";

    int variableIndex = indexFromAddress( resultAddress );

    if ( variableIndex < 0 ) return "";

    const ecl::smspec_node& ertSumVarNode = ecl_smspec_iget_node_w_node_index( m_ecl_SmSpec, variableIndex );
    return ertSumVarNode.get_unit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifEclEclipseSummary::unitSystem() const
{
    return m_unitSystem;
}

//--------------------------------------------------------------------------------------------------
// Convert "WGPRL__2:MY-WELL1-A5" to "WGPRL:MY-WELL1-A5:2"
// Matches text followed by _ or __, number, colon, alphanumeric text
// If the completion number is larger than 9, a single underscore is used due to maximum keyword length of 8 characters
//--------------------------------------------------------------------------------------------------
std::string RifEclEclipseSummary::normalizeCompletionAddress( const std::string& address )
{
    std::regex pattern( R"((\w+?)(?:_{1,2})(\d+):(\w[\w-]*))" );
    if ( std::regex_match( address, pattern ) )
    {
        return std::regex_replace( address, pattern, "$1:$3:$2" );
    }

    return address; // Return original string if pattern doesn't match
}
