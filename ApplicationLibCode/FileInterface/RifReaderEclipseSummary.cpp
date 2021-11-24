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

#include "RifReaderEclipseSummary.h"

#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"
#include "RiaStdStringTools.h"

#include "RifEclEclipseSummary.h"
#include "RifOpmCommonSummary.h"

#ifdef USE_HDF5
#include "RifHdf5SummaryExporter.h"
#include "RifOpmHdf5Summary.h"
#endif

#include <cassert>
#include <string>

#include <QDateTime>
#include <QDir>
#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::RifReaderEclipseSummary()
{
    m_valuesCache = std::make_unique<ValuesCache>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::~RifReaderEclipseSummary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::open( const QString& headerFileName, RiaThreadSafeLogger* threadSafeLogger )
{
    bool isValid = false;

    // Create reader as specified by the user using the following fallback strategy
    //
    // ESMRY
    // - if h5 file is present on disk
    //   - use h5 reader
    // - else
    //   - create ESMRY file if defined in preference
    //   - use ESMRY reader
    // - if no reader has been created, fallback to libecl
    //
    // H5
    // - if h5 file is present on disk
    //   - use h5 reader
    // - else
    //   - create h5 file if defined in preference
    //   - use h5 reader
    // - if no reader has been created, fallback to libecl
    //
    // For all import modes, use libecl to read data if no data is imported with ESMRY or h5

    RiaPreferencesSummary* prefSummary = RiaPreferencesSummary::current();

    if ( prefSummary->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON ||
         prefSummary->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON )
    {
        QFileInfo fi( headerFileName );
        QString   basenameNoExtension = fi.absolutePath() + "/" + fi.baseName();
        QString   h5FileName          = basenameNoExtension + ".h5";

        bool h5FileFound = QFile::exists( h5FileName );

        if ( h5FileFound ||
             ( prefSummary->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON ) )
        {
#ifdef USE_HDF5
            if ( prefSummary->createH5SummaryDataFiles() )
            {
                size_t createdH5FileCount = 0;
                RifHdf5SummaryExporter::ensureHdf5FileIsCreated( headerFileName.toStdString(),
                                                                 h5FileName.toStdString(),
                                                                 createdH5FileCount );

                if ( createdH5FileCount > 0 )
                {
                    QString txt = QString( "Created %1 " ).arg( h5FileName );
                    if ( threadSafeLogger ) threadSafeLogger->info( txt );
                }
                h5FileFound = QFile::exists( h5FileName );
            }

            if ( h5FileFound )
            {
                auto hdfReader = std::make_unique<RifOpmHdf5Summary>();

                isValid = hdfReader->open( headerFileName, false, threadSafeLogger );
                if ( isValid )
                {
                    m_summaryReader = std::move( hdfReader );
                }
            }
#endif
        }

        if ( !isValid && prefSummary->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON )
        {
            auto opmCommonReader = std::make_unique<RifOpmCommonEclipseSummary>();

            opmCommonReader->useEnhancedSummaryFiles( prefSummary->useEnhancedSummaryDataFiles() );
            opmCommonReader->createEnhancedSummaryFiles( prefSummary->createEnhancedSummaryDataFiles() );
            isValid = opmCommonReader->open( headerFileName, false, threadSafeLogger );

            if ( isValid )
            {
                m_summaryReader = std::move( opmCommonReader );
            }
        }
    }

    // If no summary reader has been created, always try to read data using libecl
    if ( !isValid )
    {
        auto libeclReader = std::make_unique<RifEclEclipseSummary>();

        isValid = libeclReader->open( headerFileName, threadSafeLogger );
        if ( isValid )
        {
            m_summaryReader = std::move( libeclReader );
        }
    }

    if ( isValid )
    {
        buildMetaData();
    }

    return isValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    CVF_ASSERT( values );

    if ( timeSteps( resultAddress ).empty() ) return false;

    values->clear();
    values->reserve( timeSteps( resultAddress ).size() );

    const std::vector<double>& cachedValues = m_valuesCache->getValues( resultAddress );
    if ( !cachedValues.empty() )
    {
        values->insert( values->begin(), cachedValues.begin(), cachedValues.end() );

        return true;
    }

    if ( m_differenceAddresses.count( resultAddress ) )
    {
        const std::string& quantityName = resultAddress.quantityName();
        auto historyQuantity = quantityName.substr( 0, quantityName.size() - differenceIdentifier().size() ) +
                               historyIdentifier();

        RifEclipseSummaryAddress nativeAdrNoHistory = resultAddress;
        nativeAdrNoHistory.setQuantityName( historyQuantity );
        auto quantityNoHistory = quantityName.substr( 0, historyQuantity.size() - 1 );

        RifEclipseSummaryAddress nativeAdrHistory = resultAddress;
        nativeAdrHistory.setQuantityName( quantityNoHistory );

        std::vector<double> nativeValues;
        std::vector<double> historyValues;

        if ( !this->values( nativeAdrHistory, &nativeValues ) ) return false;
        if ( !this->values( nativeAdrNoHistory, &historyValues ) ) return false;

        if ( nativeValues.size() != historyValues.size() ) return false;

        for ( size_t i = 0; i < nativeValues.size(); i++ )
        {
            double diff = nativeValues[i] - historyValues[i];
            values->push_back( diff );
            m_valuesCache->insertValues( resultAddress, *values );
        }

        return true;
    }

    auto reader = currentSummaryReader();
    if ( reader )
    {
        auto status = reader->values( resultAddress, values );

        if ( status ) m_valuesCache->insertValues( resultAddress, *values );

        return status;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifReaderEclipseSummary::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    auto reader = currentSummaryReader();
    if ( reader ) return reader->timeSteps( resultAddress );

    static std::vector<time_t> emptyVector;
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::buildMetaData()
{
    m_allResultAddresses.clear();
    m_allErrorAddresses.clear();

    auto reader = currentSummaryReader();

    if ( reader )
    {
        m_allResultAddresses = reader->allResultAddresses();
        m_allErrorAddresses  = reader->allErrorAddresses();
    }

    bool addDifferenceVectors = true;
    if ( addDifferenceVectors )
    {
        for ( const auto& adr : m_allResultAddresses )
        {
            RifEclipseSummaryAddress adrWithHistory;
            RifEclipseSummaryAddress adrWithoutHistory;

            {
                const std::string& s = adr.quantityName();
                if ( !RiaStdStringTools::endsWith( s, historyIdentifier() ) )
                {
                    RifEclipseSummaryAddress candidate = adr;
                    candidate.setQuantityName( s + historyIdentifier() );
                    if ( m_allResultAddresses.count( candidate ) )
                    {
                        adrWithHistory    = candidate;
                        adrWithoutHistory = adr;
                    }
                }
            }

            if ( adrWithoutHistory.isValid() && adrWithHistory.isValid() )
            {
                RifEclipseSummaryAddress candidate = adr;

                std::string s = candidate.quantityName() + differenceIdentifier();
                candidate.setQuantityName( s );

                m_allResultAddresses.insert( candidate );
                m_differenceAddresses.insert( candidate );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RifReaderEclipseSummary::currentSummaryReader() const
{
    if ( m_summaryReader ) return m_summaryReader.get();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifReaderEclipseSummary::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    auto reader = currentSummaryReader();

    if ( reader ) return reader->unitName( resultAddress );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifReaderEclipseSummary::unitSystem() const
{
    auto reader = currentSummaryReader();

    if ( reader ) return reader->unitSystem();

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double> RifReaderEclipseSummary::ValuesCache::EMPTY_VECTOR;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::ValuesCache::ValuesCache()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::ValuesCache::~ValuesCache()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::ValuesCache::insertValues( const RifEclipseSummaryAddress& address,
                                                         const std::vector<double>&      values )
{
    m_cachedValues[address] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifReaderEclipseSummary::ValuesCache::getValues( const RifEclipseSummaryAddress& address ) const
{
    if ( m_cachedValues.find( address ) != m_cachedValues.end() )
    {
        return m_cachedValues.at( address );
    }
    return EMPTY_VECTOR;
}
