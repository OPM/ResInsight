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
#include "RifOpmCommonSummary.h"

#include "opm/io/eclipse/ESmry.hpp"

#include <QFileInfo>

#ifdef USE_OPENMP
#include <omp.h>
#endif

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
bool RifOpmHdf5Summary::open( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
{
    if ( !openESmryFile( headerFileName, includeRestartFiles, threadSafeLogger ) )
    {
        QString errorTxt = "Failed to open " + headerFileName;

        if ( threadSafeLogger ) threadSafeLogger->error( errorTxt );

        return false;
    }

    QFileInfo fi( headerFileName );
    QString   hdfFileName = fi.absolutePath() + "/" + fi.baseName() + ".h5";

    if ( !QFile::exists( hdfFileName ) )
    {
        QString errorTxt = "Failed to open H5 file " + hdfFileName;
        if ( threadSafeLogger ) threadSafeLogger->error( errorTxt );

        return false;
    }

    m_hdf5Reader = std::make_unique<RifHdf5SummaryReader>( hdfFileName );

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
    if ( m_eSmry && m_hdf5Reader )
    {
        auto it = m_adrToSummaryNodeIndex.find( resultAddress );
        if ( it != m_adrToSummaryNodeIndex.end() )
        {
            size_t index = it->second;
            auto   node  = m_eSmry->summaryNodeList()[index];

            int         smspecIndex = static_cast<int>( node.smspecKeywordIndex );
            const auto& vectorName  = resultAddress.quantityName();

            *values = m_hdf5Reader->values( vectorName, smspecIndex );

            return true;
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

            if ( m_eSmry->hasKey( node.keyword ) )
            {
                return m_eSmry->get_unit( node );
            }
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
        // Get time step data from HDF
        if ( m_hdf5Reader )
        {
            m_timeSteps = m_hdf5Reader->timeSteps();
        }
        else
        {
            // Fallback to using opm-reader for time step data
            auto timePoints = m_eSmry->dates();

            for ( const auto& d : timePoints )
            {
                auto timeAsTimeT = std::chrono::system_clock::to_time_t( d );
                m_timeSteps.push_back( timeAsTimeT );
            }
        }

        auto [addresses, addressMap] = RifOpmCommonSummaryTools::buildMetaData( m_eSmry.get() );

        m_allResultAddresses    = addresses;
        m_adrToSummaryNodeIndex = addressMap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmHdf5Summary::openESmryFile( const QString&       headerFileName,
                                       bool                 includeRestartFiles,
                                       RiaThreadSafeLogger* threadSafeLogger )
{
    try
    {
        m_eSmry = std::make_unique<Opm::EclIO::ESmry>( headerFileName.toStdString(), includeRestartFiles );
    }
    catch ( std::exception& e )
    {
        QString txt = QString( "Optimized Summary Reader error : %1" ).arg( e.what() );

        if ( threadSafeLogger ) threadSafeLogger->error( txt );

        return false;
    }

    return true;
}
