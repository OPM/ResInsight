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
bool RifOpmCommonEclipseSummary::open( const QString&       headerFileName,
                                       bool                 includeRestartFiles,
                                       RiaThreadSafeLogger* threadSafeLogger )
{
    if ( !openESmryFile( headerFileName, includeRestartFiles, threadSafeLogger ) ) return false;

    if ( m_createLodsmryFiles && !includeRestartFiles )
    {
        // Create the lodsmry file, no-op if already present.
        bool hasFileBeenCreated = m_eSmry->make_lodsmry_file();

        if ( hasFileBeenCreated )
        {
            RifOpmCommonEclipseSummary::increaseLodFileCount();

            // If a LODSMRY file has been created, all data for all vectors has now been loaded into the summary file
            // object. Close the file object to make sure allocated data is released, and create a new file object
            // that will import only the meta data and no curve data. This is a relatively fast operation.

            if ( !openESmryFile( headerFileName, includeRestartFiles, threadSafeLogger ) ) return false;
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

        auto [addresses, addressMap] = RifOpmCommonSummaryTools::buildMetaData( m_eSmry.get() );

        m_allResultAddresses    = addresses;
        m_adrToSummaryNodeIndex = addressMap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmCommonEclipseSummary::openESmryFile( const QString&       headerFileName,
                                                bool                 includeRestartFiles,
                                                RiaThreadSafeLogger* threadSafeLogger )
{
    try
    {
        m_eSmry =
            std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles, m_useLodsmryFiles );
    }
    catch ( std::exception& e )
    {
        QString txt = QString( "Optimized Summary Reader error : %1" ).arg( e.what() );

        if ( threadSafeLogger ) threadSafeLogger->error( txt );

        return false;
    }

    return true;
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
RifEclipseSummaryAddress RifOpmCommonSummaryTools::createAddressFromSummaryNode( const Opm::EclIO::SummaryNode& summaryNode,
                                                                                 const Opm::EclIO::ESmry* summaryFile )
{
    int i = -1;
    int j = -1;
    int k = -1;

    switch ( summaryNode.category )
    {
        case Opm::EclIO::SummaryNode::Category::Aquifer:
            return RifEclipseSummaryAddress::aquiferAddress( summaryNode.keyword, summaryNode.number );
            break;
        case Opm::EclIO::SummaryNode::Category::Well:
            return RifEclipseSummaryAddress::wellAddress( summaryNode.keyword, summaryNode.wgname );
            break;
        case Opm::EclIO::SummaryNode::Category::Group:
            return RifEclipseSummaryAddress::wellGroupAddress( summaryNode.keyword, summaryNode.wgname );
            break;
        case Opm::EclIO::SummaryNode::Category::Field:
            return RifEclipseSummaryAddress::fieldAddress( summaryNode.keyword );
            break;
        case Opm::EclIO::SummaryNode::Category::Region:
        {
            if ( summaryNode.isRegionToRegion() )
            {
                auto [r1, r2] = summaryNode.regionToRegionNumbers();
                return RifEclipseSummaryAddress::regionToRegionAddress( summaryNode.keyword, r1, r2 );
            }

            return RifEclipseSummaryAddress::regionAddress( summaryNode.keyword, summaryNode.number );
        }
        break;
        case Opm::EclIO::SummaryNode::Category::Block:
            summaryFile->ijk_from_global_index( summaryNode.number, i, j, k );
            return RifEclipseSummaryAddress::blockAddress( summaryNode.keyword, i, j, k );
            break;
        case Opm::EclIO::SummaryNode::Category::Connection:
            summaryFile->ijk_from_global_index( summaryNode.number, i, j, k );
            return RifEclipseSummaryAddress::wellCompletionAddress( summaryNode.keyword, summaryNode.wgname, i, j, k );
            break;
        case Opm::EclIO::SummaryNode::Category::Segment:
            return RifEclipseSummaryAddress::wellSegmentAddress( summaryNode.keyword, summaryNode.wgname, summaryNode.number );
            break;
        case Opm::EclIO::SummaryNode::Category::Miscellaneous:
            return RifEclipseSummaryAddress::miscAddress( summaryNode.keyword );
            break;
        default:
            break;
        case Opm::EclIO::SummaryNode::Category::Node:
            break;
        case Opm::EclIO::SummaryNode::Category::Network:
            return RifEclipseSummaryAddress::networkAddress(summaryNode.keyword);
            break;
        case Opm::EclIO::SummaryNode::Category::Well_Lgr:
            return RifEclipseSummaryAddress::wellLgrAddress(summaryNode.keyword, summaryNode.lgrname, summaryNode.wgname );
            break;
        case Opm::EclIO::SummaryNode::Category::Block_Lgr:
            return RifEclipseSummaryAddress::blockLgrAddress(summaryNode.keyword, summaryNode.lgrname, summaryNode.lgri, summaryNode.lgrj, summaryNode.lgrk);
            break;
        case Opm::EclIO::SummaryNode::Category::Connection_Lgr:
            return RifEclipseSummaryAddress::wellCompletionLgrAddress(summaryNode.keyword, summaryNode.lgrname, summaryNode.wgname, summaryNode.lgri, summaryNode.lgrj, summaryNode.lgrk);
            break;
    }

    return RifEclipseSummaryAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, size_t>>
    RifOpmCommonSummaryTools::buildMetaData( const Opm::EclIO::ESmry* summaryFile )
{
    std::set<RifEclipseSummaryAddress>         addresses;
    std::map<RifEclipseSummaryAddress, size_t> addressToNodeIndexMap;

    if ( summaryFile )
    {
        auto nodes = summaryFile->summaryNodeList();
        for ( size_t i = 0; i < nodes.size(); i++ )
        {
            auto summaryNode = nodes[i];
            auto eclAdr      = createAddressFromSummaryNode( summaryNode, summaryFile );

            if ( eclAdr.isValid() )
            {
                addresses.insert( eclAdr );
                addressToNodeIndexMap[eclAdr] = i;
            }
        }
    }

    return { addresses, addressToNodeIndexMap };
}
