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

#include "RifOpmHdf5Summary.h"

#include "RiaLogging.h"

#include "RifHdf5SummaryReader.h"

#include "opm/io/eclipse/ESmry.hpp"

#ifdef USE_OPENMP
#include <omp.h>
#endif
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmHdf5Summary::RifOpmHdf5Summary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmHdf5Summary::~RifOpmHdf5Summary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmHdf5Summary::open( const QString& headerFileName, bool includeRestartFiles )
{
    if ( !openESmryFile( headerFileName, includeRestartFiles ) ) return false;

    QFileInfo fi( headerFileName );
    QString   hdfFileName = fi.absolutePath() + "/" + fi.baseName() + ".H5";

    if ( QFile::exists( hdfFileName ) )
    {
        m_hdf5Reader = std::make_unique<RifHdf5SummaryReader>( hdfFileName );
    }

    if ( !m_eSmry || !m_hdf5Reader ) return false;

    buildMetaData();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifOpmHdf5Summary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmHdf5Summary::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    if ( m_eSmry )
    {
        auto it = m_adrToSummaryNodeIndex.find( resultAddress );
        if ( it != m_adrToSummaryNodeIndex.end() )
        {
            auto index = it->second;

            bool getDataFromHdf = true;
            if ( getDataFromHdf )
            {
                auto vectorName = resultAddress.quantityName();

                *values = m_hdf5Reader->values( vectorName, static_cast<int>( index ) );

                return true;
            }
            else
            {
                auto node       = m_eSmry->summaryNodeList()[index];
                auto fileValues = m_eSmry->get( node );
                values->insert( values->begin(), fileValues.begin(), fileValues.end() );

                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmHdf5Summary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
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
RiaDefines::EclipseUnitSystem RifOpmHdf5Summary::unitSystem() const
{
    // TODO: Not implemented
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpmHdf5Summary::buildMetaData()
{
    if ( m_eSmry )
    {
        std::vector<std::chrono::system_clock::time_point> timePoints;

        if ( m_hdf5Reader )
        {
            double time_unit = 24 * 3600;

            using namespace std::chrono;
            using TP      = time_point<system_clock>;
            using DoubSec = duration<double, seconds::period>;

            auto timeDeltasInDays = m_hdf5Reader->values( "TIME" );

            std::vector<std::chrono::system_clock::time_point> d;

            TP startDat;

            d.reserve( timeDeltasInDays.size() );
            for ( const auto& t : timeDeltasInDays )
            {
                d.push_back( startDat + duration_cast<TP::duration>( DoubSec( t * time_unit ) ) );
            }

            timePoints = d;
        }
        else
            timePoints = m_eSmry->dates();

        for ( const auto& d : timePoints )
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
bool RifOpmHdf5Summary::openESmryFile( const QString& headerFileName, bool includeRestartFiles )
{
    try
    {
        m_eSmry = std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles, false );
    }
    catch ( std::exception& e )
    {
        QString txt = QString( "Optimized Summary Reader error : %1" ).arg( e.what() );
        RiaLogging::error( txt );

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifOpmHdf5Summary::createAddressFromSummaryNode( const Opm::EclIO::SummaryNode& node,
                                                                          Opm::EclIO::ESmry*             summaryFile )
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
