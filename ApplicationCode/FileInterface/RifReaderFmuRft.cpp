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

#include "cafAssert.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Observation::Observation()
    : utmx( -std::numeric_limits<double>::infinity() )
    , utmy( -std::numeric_limits<double>::infinity() )
    , mdrkb( -std::numeric_limits<double>::infinity() )
    , tvdmsl( -std::numeric_limits<double>::infinity() )
    , pressure( -std::numeric_limits<double>::infinity() )
    , pressureError( -std::numeric_limits<double>::infinity() )
    , formation()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::Observation::valid() const
{
    return utmx != std::numeric_limits<double>::infinity() && utmy != std::numeric_limits<double>::infinity() &&
           mdrkb != std::numeric_limits<double>::infinity() && tvdmsl != std::numeric_limits<double>::infinity() &&
           pressure != std::numeric_limits<double>::infinity() &&
           pressureError != std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::WellObservationSet::WellObservationSet( const QDateTime& dateTime, int measurementIndex )
    : dateTime( dateTime )
    , measurementIndex( measurementIndex )
{
}

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
    for ( QString subDir : subDirs )
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
    obsFiles << "*.obs"
             << "*.txt";
    QFileInfoList fileInfos = dir.entryInfoList( obsFiles, QDir::Files, QDir::Name );

    std::map<QString, int> fileStemCounts;
    for ( QFileInfo fileInfo : fileInfos )
    {
        // TODO:
        // Uses completeBaseName() to support wells with a dot in the name.
        // Not sure if this is necessary or desired
        fileStemCounts[fileInfo.completeBaseName()]++;
        if ( fileStemCounts[fileInfo.completeBaseName()] == 2 )
        {
            // At least one matching obs and txt file.
            return true;
        }
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

    if ( m_allWellObservations.empty() )
    {
        load();
    }

    auto it = m_allWellObservations.find( rftAddress.wellName() );
    if ( it != m_allWellObservations.end() )
    {
        const std::vector<Observation>& observations = it->second.observations;
        for ( const Observation& observation : observations )
        {
            formationLabels.push_back( QString( "%1 - Pressure: %2 +/- %3" )
                                           .arg( observation.formation )
                                           .arg( observation.pressure )
                                           .arg( observation.pressureError ) );
        }
    }
    return formationLabels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderFmuRft::eclipseRftAddresses()
{
    if ( m_allWellObservations.empty() )
    {
        load();
    }

    std::set<RifEclipseRftAddress> allAddresses;
    for ( const WellObservationPair& wellObservationPair : m_allWellObservations )
    {
        const QString&                  wellName     = wellObservationPair.first;
        const QDateTime&                dateTime     = wellObservationPair.second.dateTime;
        const std::vector<Observation>& observations = wellObservationPair.second.observations;

        for ( const Observation& observation : observations )
        {
            if ( observation.valid() )
            {
                RifEclipseRftAddress tvdAddress( wellName, dateTime, RifEclipseRftAddress::TVD );
                RifEclipseRftAddress mdAddress( wellName, dateTime, RifEclipseRftAddress::MD );
                RifEclipseRftAddress pressureAddress( wellName, dateTime, RifEclipseRftAddress::PRESSURE );
                RifEclipseRftAddress pressureErrorAddress( wellName, dateTime, RifEclipseRftAddress::PRESSURE_ERROR );
                allAddresses.insert( tvdAddress );
                allAddresses.insert( mdAddress );
                allAddresses.insert( pressureAddress );
                allAddresses.insert( pressureErrorAddress );
            }
        }
    }
    return allAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderFmuRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    CAF_ASSERT( values );

    if ( m_allWellObservations.empty() )
    {
        load();
    }

    auto it = m_allWellObservations.find( rftAddress.wellName() );
    if ( it != m_allWellObservations.end() )
    {
        const std::vector<Observation>& observations = it->second.observations;
        values->clear();
        values->reserve( observations.size() );
        for ( const Observation& observation : observations )
        {
            switch ( rftAddress.wellLogChannel() )
            {
                case RifEclipseRftAddress::TVD:
                    values->push_back( observation.tvdmsl );
                    break;
                case RifEclipseRftAddress::MD:
                    values->push_back( observation.mdrkb );
                    break;
                case RifEclipseRftAddress::PRESSURE:
                    values->push_back( observation.pressure );
                    break;
                case RifEclipseRftAddress::PRESSURE_ERROR:
                    values->push_back( observation.pressureError );
                    break;
                default:
                    CAF_ASSERT( false && "Wrong channel type sent to Fmu RFT reader" );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderFmuRft::load()
{
    QString errorMsg;

    QFileInfo fileInfo( m_filePath );
    if ( !( fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable() ) )
    {
        errorMsg = QString( "Directory '%1' does not exist or isn't readable" ).arg( m_filePath );
        RiaLogging::error( errorMsg );
        return;
    }

    QDir dir( m_filePath );

    WellObservationMap wellObservations = loadWellDates( dir, &errorMsg );
    WellObservationMap validObservations;
    if ( wellObservations.empty() )
    {
        if ( errorMsg.isEmpty() )
        {
            errorMsg = QString( "'%1' contains no valid FMU RFT data" ).arg( m_filePath );
        }
        RiaLogging::error( errorMsg );
        return;
    }

    for ( auto it = wellObservations.begin(); it != wellObservations.end(); ++it )
    {
        const QString&      wellName           = it->first;
        WellObservationSet& wellObservationSet = it->second;
        QString             txtFile            = QString( "%1.txt" ).arg( wellName );
        QString             obsFile            = QString( "%1.obs" ).arg( wellName );

        if ( !readTxtFile( dir.absoluteFilePath( txtFile ), &errorMsg, &wellObservationSet ) )
        {
            RiaLogging::warning( errorMsg );
            continue;
        }

        if ( !readObsFile( dir.absoluteFilePath( obsFile ), &errorMsg, &wellObservationSet ) )
        {
            RiaLogging::warning( errorMsg );
            continue;
        }
        validObservations.insert( *it );
    }

    m_allWellObservations.swap( validObservations );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime>
    RifReaderFmuRft::availableTimeSteps( const QString&                                     wellName,
                                         const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    if ( wellLogChannelName == RifEclipseRftAddress::TVD || wellLogChannelName == RifEclipseRftAddress::MD ||
         wellLogChannelName == RifEclipseRftAddress::PRESSURE )
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
    if ( m_allWellObservations.empty() )
    {
        load();
    }

    auto it = m_allWellObservations.find( wellName );
    if ( it != m_allWellObservations.end() )
    {
        return { it->second.dateTime };
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime>
    RifReaderFmuRft::availableTimeSteps( const QString&                                               wellName,
                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    if ( relevantChannels.count( RifEclipseRftAddress::TVD ) || relevantChannels.count( RifEclipseRftAddress::MD ) ||
         relevantChannels.count( RifEclipseRftAddress::PRESSURE ) )
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
    if ( m_allWellObservations.empty() )
    {
        load();
    }

    if ( !m_allWellObservations.empty() )
    {
        return { RifEclipseRftAddress::TVD, RifEclipseRftAddress::MD, RifEclipseRftAddress::PRESSURE };
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderFmuRft::wellNames()
{
    if ( m_allWellObservations.empty() )
    {
        load();
    }

    std::set<QString> wellNames;
    for ( auto it = m_allWellObservations.begin(); it != m_allWellObservations.end(); ++it )
    {
        wellNames.insert( it->first );
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::WellObservationMap RifReaderFmuRft::loadWellDates( QDir& dir, QString* errorMsg )
{
    CAF_ASSERT( errorMsg );

    WellObservationMap validObservations;

    QFileInfo wellDateFileInfo( dir.absoluteFilePath( RifReaderFmuRft::wellPathFileName() ) );
    if ( !( wellDateFileInfo.exists() && wellDateFileInfo.isFile() && wellDateFileInfo.isReadable() ) )
    {
        *errorMsg = QString( "%1 cannot be found at '%s'" ).arg( RifReaderFmuRft::wellPathFileName() ).arg( m_filePath );
        return WellObservationMap();
    }

    {
        QFile wellDateFile( wellDateFileInfo.absoluteFilePath() );
        if ( !wellDateFile.open( QIODevice::Text | QIODevice::ReadOnly ) )
        {
            *errorMsg = QString( "Could not read '%1'" ).arg( wellDateFileInfo.absoluteFilePath() );
            return WellObservationMap();
        }
        QTextStream fileStream( &wellDateFile );
        while ( true )
        {
            QString line = fileStream.readLine();
            if ( line.isNull() )
            {
                break;
            }
            else
            {
                QTextStream lineStream( &line );

                QString wellName;
                int     day, month, year, measurementIndex;

                lineStream >> wellName >> day >> month >> year >> measurementIndex;
                if ( lineStream.status() != QTextStream::Ok )
                {
                    *errorMsg = QString( "Failed to parse '%1'" ).arg( wellDateFileInfo.absoluteFilePath() );
                    return WellObservationMap();
                }

                QDateTime dateTime( QDate( year, month, day ) );
                dateTime.setTimeSpec( Qt::UTC );
                WellObservationSet observationSet( dateTime, measurementIndex );
                validObservations.insert( std::make_pair( wellName, observationSet ) );
            }
        }
    }

    return validObservations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::readTxtFile( const QString& fileName, QString* errorMsg, WellObservationSet* wellObservationSet )
{
    CAF_ASSERT( wellObservationSet );

    QFile file( fileName );
    if ( !( file.open( QIODevice::Text | QIODevice::ReadOnly ) ) )
    {
        *errorMsg = QString( "Could not open '%1'" ).arg( fileName );
        return false;
    }

    QTextStream stream( &file );
    while ( true )
    {
        QString line = stream.readLine().trimmed();
        if ( line.isNull() || line.isEmpty() )
        {
            break;
        }
        else
        {
            QTextStream lineStream( &line );

            double  utmx, utmy, mdrkb, tvdmsl;
            QString formationName;

            lineStream >> utmx >> utmy >> mdrkb >> tvdmsl >> formationName;

            if ( lineStream.status() != QTextStream::Ok )
            {
                *errorMsg = QString( "Failed to parse '%1'" ).arg( fileName );
                return false;
            }

            Observation observation;
            observation.utmx      = utmx;
            observation.utmy      = utmy;
            observation.mdrkb     = mdrkb;
            observation.tvdmsl    = tvdmsl;
            observation.formation = formationName;
            wellObservationSet->observations.push_back( observation );
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::readObsFile( const QString& fileName, QString* errorMsg, WellObservationSet* wellObservationSet )
{
    QFile file( fileName );
    if ( !( file.open( QIODevice::Text | QIODevice::ReadOnly ) ) )
    {
        *errorMsg = QString( "Could not open '%1'" ).arg( fileName );
        return false;
    }

    size_t lineNumber = 0u;

    QTextStream stream( &file );
    while ( true )
    {
        QString line = stream.readLine().trimmed();
        if ( line.isNull() || line.isEmpty() )
        {
            break;
        }
        else if ( lineNumber >= wellObservationSet->observations.size() )
        {
            *errorMsg = QString( "'%1' has more lines than corresponding txt file" ).arg( fileName );
            return false;
        }
        else
        {
            QTextStream lineStream( &line );

            double pressure, pressureError;

            lineStream >> pressure >> pressureError;

            if ( lineStream.status() != QTextStream::Ok )
            {
                *errorMsg = QString( "Failed to parse line %1 of '%2'" ).arg( lineNumber + 1 ).arg( fileName );
                return false;
            }

            Observation& observation  = wellObservationSet->observations[lineNumber];
            observation.pressure      = pressure;
            observation.pressureError = pressureError;
        }
        lineNumber++;
    }

    if ( lineNumber != wellObservationSet->observations.size() )
    {
        *errorMsg = QString( "'%1' has less lines than corresponding txt file" ).arg( fileName );
        return false;
    }
    return true;
}
