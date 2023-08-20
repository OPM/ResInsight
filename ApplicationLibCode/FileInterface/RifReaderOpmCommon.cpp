/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RifReaderOpmCommon.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaLogging.h"
#include "RifEclipseOutputFileTools.h"
#include "RifOpmGridTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/EclipseState/Runspec.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/io/eclipse/EInit.hpp"
#include "opm/io/eclipse/ERst.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::RifReaderOpmCommon()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::~RifReaderOpmCommon()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    QStringList fileSet;
    {
        // auto task = progress.task( "Get set of files" );

        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;
    }

    try
    {
        m_gridFileName = fileName.toStdString();

        if ( !RifOpmGridTools::importGrid( m_gridFileName, eclipseCase->mainGrid(), eclipseCase ) )
        {
            RiaLogging::error( "Failed to open grid file " + fileName );

            return false;
        }

        buildMetaData( eclipseCase );

        return true;
    }
    catch ( std::exception& e )
    {
        auto description = e.what();
        RiaLogging::error( description );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    if ( m_initFile )
    {
        auto resultName = result.toStdString();

        auto resultEntries = m_initFile->getList();
        for ( const auto& entry : resultEntries )
        {
            const auto& [keyword, kwType, size] = entry;
            if ( keyword == resultName )
            {
                if ( kwType == Opm::EclIO::eclArrType::DOUB || kwType == Opm::EclIO::eclArrType::REAL )
                {
                    auto fileValues = m_initFile->getInitData<double>( resultName );
                    values->insert( values->end(), fileValues.begin(), fileValues.end() );
                }
                else if ( kwType == Opm::EclIO::eclArrType::INTE )
                {
                    auto fileValues = m_initFile->getInitData<int>( resultName );
                    values->insert( values->end(), fileValues.begin(), fileValues.end() );
                }
            }
        }
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::dynamicResult( const QString&                result,
                                        RiaDefines::PorosityModelType matrixOrFracture,
                                        size_t                        stepIndex,
                                        std::vector<double>*          values )
{
    if ( m_restartFile )
    {
        auto resultName = result.toStdString();

        auto stepNumbers = m_restartFile->listOfReportStepNumbers();
        auto stepNumber  = stepNumbers[stepIndex];

        auto resultEntries = m_restartFile->getList();
        for ( const auto& entry : resultEntries )
        {
            const auto& [keyword, kwType, size] = entry;
            if ( keyword == resultName )
            {
                if ( kwType == Opm::EclIO::eclArrType::DOUB || kwType == Opm::EclIO::eclArrType::REAL )
                {
                    auto fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber );
                    values->insert( values->end(), fileValues.begin(), fileValues.end() );
                }
                else if ( kwType == Opm::EclIO::eclArrType::INTE )
                {
                    auto fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber );
                    values->insert( values->end(), fileValues.begin(), fileValues.end() );
                }
            }
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifKeywordValueCount> createKeywordInfo( std::vector<Opm::EclIO::EclFile::EclEntry> entries )
{
    std::vector<RifKeywordValueCount> fileKeywordInfo;

    for ( const auto& entry : entries )
    {
        const auto& [keyword, kwType, size] = entry;

        RifKeywordValueCount::KeywordDataType dataType = RifKeywordValueCount::KeywordDataType::UNKNOWN;

        if ( kwType == Opm::EclIO::eclArrType::INTE )
            dataType = RifKeywordValueCount::KeywordDataType::INTEGER;
        else if ( kwType == Opm::EclIO::eclArrType::REAL )
            dataType = RifKeywordValueCount::KeywordDataType::FLOAT;
        else if ( kwType == Opm::EclIO::eclArrType::DOUB )
            dataType = RifKeywordValueCount::KeywordDataType::DOUBLE;

        if ( dataType != RifKeywordValueCount::KeywordDataType::UNKNOWN )
        {
            fileKeywordInfo.emplace_back( RifKeywordValueCount( keyword, static_cast<size_t>( size ), dataType ) );
        }
    }

    return fileKeywordInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::buildMetaData( RigEclipseCaseData* eclipseCase )
{
    Opm::Deck deck;

    // It is required to create a deck as the input parameter to runspec. The = default() initialization of the runspec keyword does not
    // initialize the object as expected.
    Opm::Runspec runspec( deck );
    Opm::Parser  parser( false );

    // Get set of files
    QStringList fileSet;
    RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( QString::fromStdString( m_gridFileName ), &fileSet );

    std::string initFileName;
    std::string restartFileName;

    const QString initExt =
        caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_INIT );

    const QString restartExt =
        caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNRST );

    for ( const auto& s : fileSet )
    {
        if ( s.endsWith( initExt, Qt::CaseInsensitive ) ) initFileName = s.toStdString();
        if ( s.endsWith( restartExt, Qt::CaseInsensitive ) ) restartFileName = s.toStdString();
    }

    if ( !initFileName.empty() )
    {
        m_initFile = std::make_unique<Opm::EclIO::EInit>( initFileName );

        auto                              entries     = m_initFile->list_arrays();
        std::vector<RifKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        std::vector<RigEclipseTimeStepInfo> timeStepInfo;
        QDateTime                           dateTime;
        double                              daysSinceSimStart = 0.0;
        timeStepInfo.push_back( { dateTime, 0, 0.0 } );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo, timeStepInfo, eclipseCase );
    }

    if ( !restartFileName.empty() )
    {
        m_restartFile = std::make_unique<Opm::EclIO::ERst>( restartFileName );

        std::vector<Opm::EclIO::EclFile::EclEntry> entries;
        for ( auto reportNumber : m_restartFile->listOfReportStepNumbers() )
        {
            auto reportEntries = m_restartFile->listOfRstArrays( reportNumber );
            entries.insert( entries.end(), reportEntries.begin(), reportEntries.end() );
        }

        std::vector<RigEclipseTimeStepInfo> timeStepInfo;
        for ( auto reportNumber : m_restartFile->listOfReportStepNumbers() )
        {
            QDateTime dateTime;
            double    daysSinceSimStart = 0.0;
            dateTime                    = dateTime.addDays( reportNumber );
            timeStepInfo.push_back( { dateTime, reportNumber, 0.0 } );
        }

        auto last = std::unique( entries.begin(), entries.end() );
        entries.erase( last, entries.end() );

        std::vector<RifKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo, timeStepInfo, eclipseCase );
    }
}
