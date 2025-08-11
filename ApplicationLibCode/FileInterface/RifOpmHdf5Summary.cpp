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
#include "RiaStdStringTools.h"

#include "RifHdf5SummaryReader.h"
#include "RifOpmSummaryTools.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/ESmry.hpp"

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

    m_timeSteps.clear();
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
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifOpmHdf5Summary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RifOpmHdf5Summary::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_eSmry && m_hdf5Reader )
    {
        auto it = m_adrToSmspecIndices.find( resultAddress );
        if ( it != m_adrToSmspecIndices.end() )
        {
            const auto& vectorName  = resultAddress.vectorName();
            size_t      smspecIndex = it->second;

            if ( smspecIndex != std::numeric_limits<size_t>::max() )
            {
                std::vector<double> values = m_hdf5Reader->values( vectorName, static_cast<int>( smspecIndex ) );
                return { true, values };
            }
        }
    }

    return { false, {} };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmHdf5Summary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_eSmry )
    {
        auto it = m_summaryAddressToKeywordMap.find( resultAddress );
        if ( it != m_summaryAddressToKeywordMap.end() )
        {
            auto keyword              = it->second;
            auto stringFromFileReader = m_eSmry->get_unit( keyword );

            return std::string( RiaStdStringTools::trimString( stringFromFileReader ) );
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
void RifOpmHdf5Summary::createAndSetAddresses()
{
    if ( m_eSmry )
    {
        auto [addresses, smspecIndices, addressToKeywordMap] = RifOpmSummaryTools::buildAddressesSmspecAndKeywordMap( m_eSmry.get() );
        m_allResultAddresses                                 = addresses;
        m_adrToSmspecIndices                                 = smspecIndices;
        m_summaryAddressToKeywordMap                         = addressToKeywordMap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmHdf5Summary::openESmryFile( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOpmHdf5Summary::keywordCount() const
{
    if ( m_eSmry )
    {
        return m_eSmry->keywordList().size();
    }
    return 0;
}
