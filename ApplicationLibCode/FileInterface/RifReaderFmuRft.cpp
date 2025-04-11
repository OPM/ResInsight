/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-  Equinor ASA
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

#include "RifReaderFmuRft.h"

#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "RiaTextStringTools.h"

#include "cafAssert.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::RifReaderFmuRft( const QString& filePath )
    : m_filePath( filePath )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifReaderFmuRft::findSubDirectoriesWithFmuRftData( const QString& filePath )
{
    QStringList subDirsContainingFmuRftData;

    QFileInfo fileInfo( filePath );
    if ( !( fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable() ) )
    {
        return subDirsContainingFmuRftData;
    }

    if ( directoryContainsFmuRftData( filePath ) )
    {
        subDirsContainingFmuRftData.push_back( filePath );
    }

    QDir dir( filePath );

    QStringList subDirs = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name );
    for ( const QString& subDir : subDirs )
    {
        QString absDir = dir.absoluteFilePath( subDir );
        subDirsContainingFmuRftData.append( findSubDirectoriesWithFmuRftData( absDir ) );
    }

    return subDirsContainingFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::directoryContainsFmuRftData( const QString& filePath )
{
    QFileInfo baseFileInfo( filePath );
    if ( !( baseFileInfo.exists() && baseFileInfo.isDir() && baseFileInfo.isReadable() ) )
    {
        return false;
    }

    QDir dir( filePath );
    if ( !dir.exists( RifReaderFmuRft::wellPathFileName() ) )
    {
        return false;
    }

    QStringList obsFiles;
    obsFiles << "*.obs" << "*.txt";
    QFileInfoList fileInfos = dir.entryInfoList( obsFiles, QDir::Files, QDir::Name );

    bool foundObsFile = false;
    bool foundTxtFile = false;
    for ( const QFileInfo& fileInfo : fileInfos )
    {
        if ( fileInfo.fileName().endsWith( "obs" ) ) foundObsFile = true;
        if ( fileInfo.fileName().endsWith( "txt" ) ) foundTxtFile = true;

        // At least one matching obs and txt file.
        if ( foundObsFile && foundTxtFile ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReaderFmuRft::wellPathFileName()
{
    return "well_date_rft.txt";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RifReaderFmuRft::labels( const RifEclipseRftAddress& rftAddress )
{
    std::vector<QString> formationLabels;

    for ( const auto& observation : m_observations )
    {
        if ( observation.wellDate.wellName == rftAddress.wellName() && observation.wellDate.dateTime == rftAddress.timeStep() )
        {
            formationLabels.push_back(
                QString( "%1 - Pressure: %2 +/- %3" ).arg( observation.location.formation ).arg( observation.pressure ).arg( observation.pressureError ) );
        }
    }

    return formationLabels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderFmuRft::eclipseRftAddresses()
{
    if ( m_observations.empty() )
    {
        importData();
    }

    std::set<std::pair<QString, QDateTime>> wellDateTimePairs;
    for ( const auto& observation : m_observations )
    {
        wellDateTimePairs.insert( { observation.wellDate.wellName, observation.wellDate.dateTime } );
    }

    std::set<RifEclipseRftAddress> allAddresses;

    for ( const auto& [wellName, dateTime] : wellDateTimePairs )
    {
        RifEclipseRftAddress tvdAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::TVD );
        RifEclipseRftAddress mdAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::MD );
        RifEclipseRftAddress pressureAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        RifEclipseRftAddress pressureErrorAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR );
        allAddresses.insert( tvdAddress );
        allAddresses.insert( mdAddress );
        allAddresses.insert( pressureAddress );
        allAddresses.insert( pressureErrorAddress );
    }

    return allAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderFmuRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    CAF_ASSERT( values );

    if ( m_observations.empty() )
    {
        importData();
    }

    for ( const auto& observation : m_observations )
    {
        if ( observation.wellDate.wellName == rftAddress.wellName() && observation.wellDate.dateTime == rftAddress.timeStep() )
        {
            switch ( rftAddress.wellLogChannel() )
            {
                case RifEclipseRftAddress::RftWellLogChannelType::TVD:
                    values->push_back( observation.location.tvdmsl );
                    break;
                case RifEclipseRftAddress::RftWellLogChannelType::MD:
                    values->push_back( observation.location.mdrkb );
                    break;
                case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE:
                    values->push_back( observation.pressure );
                    break;
                case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR:
                    values->push_back( observation.pressureError );
                    break;
                default:
                    CAF_ASSERT( false );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderFmuRft::importData()
{
    QFileInfo fileInfo( m_filePath );
    if ( !( fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable() ) )
    {
        auto errorMsg = QString( "Directory '%1' does not exist or isn't readable" ).arg( m_filePath );
        RiaLogging::error( errorMsg );
        return;
    }

    QDir dir( m_filePath );

    auto wellDates = importWellDates( dir.absoluteFilePath( RifReaderFmuRft::wellPathFileName() ) );
    if ( wellDates.empty() )
    {
        RiaLogging::error( QString( "'%1' contains no valid FMU RFT data" ).arg( m_filePath ) );
        return;
    }

    std::map<QString, int> nameAndMeasurementCount;

    // Find the number of well measurements for each well
    for ( const auto& wellDate : wellDates )
    {
        nameAndMeasurementCount[wellDate.wellName]++;
    }

    for ( const auto& [wellName, measurementCount] : nameAndMeasurementCount )
    {
        for ( int i = 0; i < measurementCount; i++ )
        {
            int measurementId = i + 1;

            auto findFileName = []( const QString& wellName, const QString& extension, int measurementId, const QDir& dir ) -> QString
            {
                QString candidate = dir.absoluteFilePath( QString( "%1_%2.%3" ).arg( wellName ).arg( measurementId ).arg( extension ) );
                if ( QFile::exists( candidate ) )
                {
                    return candidate;
                }

                QString candidateOldFormat = dir.absoluteFilePath( QString( "%1.%2" ).arg( wellName ).arg( extension ) );
                if ( QFile::exists( candidateOldFormat ) )
                {
                    return candidateOldFormat;
                }

                return {};
            };

            // The text file name can be either <wellName>_<measurementId>.txt or <wellName>.txt
            QString txtFile   = findFileName( wellName, "txt", measurementId, dir );
            auto    locations = importLocations( dir.absoluteFilePath( txtFile ) );
            if ( locations.empty() ) continue;

            // The observation file name can be either <wellName>_<measurementId>.obs or <wellName>.obs
            QString observationFileName = findFileName( wellName, "obs", measurementId, dir );
            if ( observationFileName.isEmpty() ) continue;

            for ( const auto& wellDate : wellDates )
            {
                if ( wellDate.wellName == wellName && wellDate.measurementId == measurementId )
                {
                    auto observations = importObservations( dir.absoluteFilePath( observationFileName ), locations, wellDate );
                    m_observations.insert( m_observations.end(), observations.begin(), observations.end() );

                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderFmuRft::availableTimeSteps( const QString&                                     wellName,
                                                         const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    if ( wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::TVD ||
         wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::MD ||
         wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE )
    {
        return availableTimeSteps( wellName );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderFmuRft::availableTimeSteps( const QString& wellName )
{
    if ( m_observations.empty() )
    {
        importData();
    }

    std::set<QDateTime> dateTimes;
    for ( const auto& observation : m_observations )
    {
        if ( observation.wellDate.wellName != wellName ) continue;
        dateTimes.insert( observation.wellDate.dateTime );
    }
    return dateTimes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderFmuRft::availableTimeSteps( const QString&                                               wellName,
                                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    if ( relevantChannels.count( RifEclipseRftAddress::RftWellLogChannelType::TVD ) ||
         relevantChannels.count( RifEclipseRftAddress::RftWellLogChannelType::MD ) ||
         relevantChannels.count( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE ) )
    {
        return availableTimeSteps( wellName );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifReaderFmuRft::availableWellLogChannels( const QString& wellName )
{
    if ( m_observations.empty() )
    {
        importData();
    }

    if ( !m_observations.empty() )
    {
        return { RifEclipseRftAddress::RftWellLogChannelType::TVD,
                 RifEclipseRftAddress::RftWellLogChannelType::MD,
                 RifEclipseRftAddress::RftWellLogChannelType::PRESSURE };
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderFmuRft::wellNames()
{
    if ( m_observations.empty() )
    {
        importData();
    }

    std::set<QString> names;

    for ( const auto& observation : m_observations )
    {
        names.insert( observation.wellDate.wellName );
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderFmuRft::WellDate> RifReaderFmuRft::importWellDates( const QString& fileName )
{
    if ( !( QFile::exists( fileName ) ) )
    {
        RiaLogging::error( QString( "%1 cannot be found at '%s'" ).arg( RifReaderFmuRft::wellPathFileName() ).arg( fileName ) );
        return {};
    }

    QFile wellDateFile( fileName );
    if ( !wellDateFile.open( QIODevice::Text | QIODevice::ReadOnly ) )
    {
        RiaLogging::error( QString( "Could not read '%1'" ).arg( fileName ) );
        return {};
    }

    std::vector<RifReaderFmuRft::WellDate> wellDates;

    QTextStream fileStream( &wellDateFile );
    while ( !fileStream.atEnd() )
    {
        QString line = fileStream.readLine();

        line = line.simplified();
        if ( line.isNull() || line.isEmpty() )
        {
            continue;
        }

        QString wellName;
        int     day, month, year, measurementIndex;

        auto words = RiaTextStringTools::splitSkipEmptyParts( line );
        if ( words.size() == 5 )
        {
            wellName         = words[0];
            day              = words[1].toInt();
            month            = words[2].toInt();
            year             = words[3].toInt();
            measurementIndex = words[4].toInt();
        }
        else if ( words.size() == 3 )
        {
            wellName = words[0];

            QStringList dateWords = words[1].split( "-" );
            if ( dateWords.size() != 3 )
            {
                RiaLogging::error( QString( "Failed to parse '%1'" ).arg( fileName ) );
                return {};
            }

            year  = dateWords[0].toInt();
            month = dateWords[1].toInt();
            day   = dateWords[2].toInt();

            measurementIndex = words[2].toInt();
        }
        else
        {
            RiaLogging::error( QString( "Failed to parse '%1'" ).arg( fileName ) );
            return {};
        }

        QDateTime dateTime = RiaQDateTimeTools::createDateTime( QDate( year, month, day ) );
        dateTime.setTimeSpec( Qt::UTC );

        wellDates.push_back( { wellName, dateTime, measurementIndex } );
    }

    return wellDates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderFmuRft::Location> RifReaderFmuRft::importLocations( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::Text | QIODevice::ReadOnly ) )
    {
        RiaLogging::error( QString( "Could not open '%1'" ).arg( fileName ) );
        return {};
    }

    std::vector<RifReaderFmuRft::Location> locations;

    QTextStream stream( &file );
    while ( true )
    {
        QString line = stream.readLine().trimmed();
        if ( line.isNull() || line.isEmpty() )
        {
            break;
        }

        QTextStream lineStream( &line );

        double  utmx, utmy, mdrkb, tvdmsl;
        QString formationName;

        lineStream >> utmx >> utmy >> mdrkb >> tvdmsl >> formationName;

        if ( lineStream.status() != QTextStream::Ok )
        {
            RiaLogging::error( QString( "Failed to parse '%1'" ).arg( fileName ) );
            return {};
        }

        locations.push_back( { utmx, utmy, mdrkb, tvdmsl, formationName } );
    }

    return locations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderFmuRft::Observation>
    RifReaderFmuRft::importObservations( const QString& fileName, const std::vector<Location>& locations, const WellDate& wellDate )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::Text | QIODevice::ReadOnly ) )
    {
        RiaLogging::error( QString( "Could not open '%1'" ).arg( fileName ) );
        return {};
    }

    std::vector<RifReaderFmuRft::Observation> observations;

    QTextStream stream( &file );
    size_t      lineNumber = 0u;
    while ( true )
    {
        QString line = stream.readLine().trimmed();
        if ( line.isNull() || line.isEmpty() )
        {
            break;
        }

        if ( lineNumber >= locations.size() )
        {
            RiaLogging::error( QString( "'%1' has more lines than corresponding txt file" ).arg( fileName ) );
            return {};
        }

        QTextStream lineStream( &line );

        double pressure, pressureError;

        lineStream >> pressure >> pressureError;

        if ( lineStream.status() != QTextStream::Ok )
        {
            RiaLogging::error( QString( "Failed to parse line %1 of '%2'" ).arg( lineNumber + 1 ).arg( fileName ) );
            return {};
        }

        // -1.0 is used to indicate missing data
        if ( pressure != -1.0 )
        {
            observations.push_back(
                { .wellDate = wellDate, .location = locations[lineNumber], .pressure = pressure, .pressureError = pressureError } );
        }

        lineNumber++;
    }

    return observations;
}
