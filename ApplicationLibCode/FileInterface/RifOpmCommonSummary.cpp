/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RifOpmCommonSummary.h"

#include "RiaLogging.h"

#include "opm/io/eclipse/ESmry.hpp"

#ifdef USE_OPENMP
#include <omp.h>
#endif

size_t RifOpmCommonEclipseSummary::sm_createdLodFileCount = 0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonEclipseSummary::RifOpmCommonEclipseSummary()
    : m_useLodsmryFiles( false )
    , m_createLodsmryFiles( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonEclipseSummary::~RifOpmCommonEclipseSummary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::useLodsmaryFiles( bool enable )
{
    m_useLodsmryFiles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::createLodsmaryFiles( bool enable )
{
    m_createLodsmryFiles = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::resetLodCount()
{
    sm_createdLodFileCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOpmCommonEclipseSummary::numberOfLodFilesCreated()
{
    return sm_createdLodFileCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::open( const QString& headerFileName, bool includeRestartFiles )
{
    try
    {
        m_eSmry =
            std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles, m_useLodsmryFiles );
    }
    catch ( std::exception& e )
    {
        QString txt = QString( "Optimized Summary Reader error : %1" ).arg( e.what() );
        RiaLogging::error( txt );

        return false;
    }

    if ( m_createLodsmryFiles && !includeRestartFiles )
    {
        // Create the lodsmry file, no-op if already present.
        bool hasFileBeenCreated = m_eSmry->make_lodsmry_file();

        if ( hasFileBeenCreated )
        {
            RifOpmCommonEclipseSummary::increaseLodFileCount();
        }
    }

    if ( !m_eSmry ) return false;

    buildMetaData();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifOpmCommonEclipseSummary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    if ( m_eSmry )
    {
        auto it = m_adrToSummaryNodeIndex.find( resultAddress );
        if ( it != m_adrToSummaryNodeIndex.end() )
        {
            auto index      = it->second;
            auto node       = m_eSmry->summaryNodeList()[index];
            auto fileValues = m_eSmry->get( node );
            values->insert( values->begin(), fileValues.begin(), fileValues.end() );
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmCommonEclipseSummary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_eSmry )
    {
        auto it = m_adrToSummaryNodeIndex.find( resultAddress );
        if ( it != m_adrToSummaryNodeIndex.end() )
        {
            auto index = it->second;
            auto node  = m_eSmry->summaryNodeList()[index];
            return m_eSmry->get_unit( node );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifOpmCommonEclipseSummary::unitSystem() const
{
    // TODO: Not implemented
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::buildMetaData()
{
    if ( m_eSmry )
    {
        auto dates = m_eSmry->dates();
        for ( const auto& d : dates )
        {
            auto timeAsTimeT = std::chrono::system_clock::to_time_t( d );
            m_timeSteps.push_back( timeAsTimeT );
        }

        auto nodes = m_eSmry->summaryNodeList();
        for ( size_t i = 0; i < nodes.size(); i++ )
        {
            auto summaryNode = nodes[i];
            auto eclAdr      = createAddressFromSummaryNode( summaryNode, m_eSmry.get() );

            if ( eclAdr.isValid() )
            {
                m_allResultAddresses.insert( eclAdr );
                m_adrToSummaryNodeIndex[eclAdr] = i;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmCommonEclipseSummary::increaseLodFileCount()
{
    // This function can be called from a parallel loop, make it thread safe
#pragma omp critical
    sm_createdLodFileCount++;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifOpmCommonEclipseSummary::createAddressFromSummaryNode( const Opm::EclIO::SummaryNode& node,
                                                                                   Opm::EclIO::ESmry* summaryFile )
{
    int i = -1;
    int j = -1;
    int k = -1;

    switch ( node.category )
    {
        case Opm::EclIO::SummaryNode::Category::Aquifer:
            return RifEclipseSummaryAddress::aquiferAddress( node.keyword, node.number );
            break;
        case Opm::EclIO::SummaryNode::Category::Well:
            return RifEclipseSummaryAddress::wellAddress( node.keyword, node.wgname );
            break;
        case Opm::EclIO::SummaryNode::Category::Group:
            return RifEclipseSummaryAddress::wellGroupAddress( node.keyword, node.wgname );
            break;
        case Opm::EclIO::SummaryNode::Category::Field:
            return RifEclipseSummaryAddress::fieldAddress( node.keyword );
            break;
        case Opm::EclIO::SummaryNode::Category::Region:
            return RifEclipseSummaryAddress::regionAddress( node.keyword, node.number );
            break;
        case Opm::EclIO::SummaryNode::Category::Block:
            summaryFile->ijk_from_global_index( node.number, i, j, k );
            return RifEclipseSummaryAddress::blockAddress( node.keyword, i, j, k );
            break;
        case Opm::EclIO::SummaryNode::Category::Connection:
            summaryFile->ijk_from_global_index( node.number, i, j, k );
            return RifEclipseSummaryAddress::wellCompletionAddress( node.keyword, node.wgname, i, j, k );
            break;
        case Opm::EclIO::SummaryNode::Category::Segment:
            return RifEclipseSummaryAddress::wellSegmentAddress( node.keyword, node.wgname, node.number );
            break;
        case Opm::EclIO::SummaryNode::Category::Miscellaneous:
            return RifEclipseSummaryAddress::miscAddress( node.keyword );
            break;
        default:
            break;
    }

    return RifEclipseSummaryAddress();
}
