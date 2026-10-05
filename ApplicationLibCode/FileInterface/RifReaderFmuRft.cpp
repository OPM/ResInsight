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
#include "RiaQStringFormatter.h"
#include "RiaTextStringTools.h"

#include "cafAssert.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>
#include <limits>

namespace
{
const QString ESTIMATED_FORMATION_NAME_SUFFIX = " (Estimate)";
} // namespace

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
std::set<QString> RifReaderFmuRft::formationNames( const QString& wellName, const QDateTime& timeStep )
{
    if ( m_observations.empty() )
    {
        importData();
    }

    std::set<QString> formations;
    for ( const auto& observation : m_observations )
    {
        if ( observation.wellDate.wellName != wellName || observation.wellDate.dateTime != timeStep ) continue;
        if ( !observation.location.formation.isEmpty() )
            formations.insert( observation.location.formation + ESTIMATED_FORMATION_NAME_SUFFIX );
    }

    return formations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReaderFmuRft::stripEstimatedFormationNameSuffix( const QString& formationName )
{
    if ( formationName.endsWith( ESTIMATED_FORMATION_NAME_SUFFIX ) )
    {
        return formationName.chopped( ESTIMATED_FORMATION_NAME_SUFFIX.length() );
    }

    return formationName;
}

//--------------------------------------------------------------------------------------------------
/// Returns all observations for the given well/time step, sorted by MD (RKB) ascending.
//--------------------------------------------------------------------------------------------------
std::vector<const RifReaderFmuRft::Observation*> RifReaderFmuRft::sortedObservationsForWellDate( const QString&   wellName,
                                                                                                 const QDateTime& timeStep )
{
    if ( m_observations.empty() )
    {
        importData();
    }

    std::vector<const Observation*> observationsForWellDate;
    for ( const auto& observation : m_observations )
    {
        if ( observation.wellDate.wellName != wellName || observation.wellDate.dateTime != timeStep ) continue;
        observationsForWellDate.push_back( &observation );
    }

    std::sort( observationsForWellDate.begin(),
               observationsForWellDate.end(),
               []( const Observation* a, const Observation* b ) { return a->location.mdrkb < b->location.mdrkb; } );

    return observationsForWellDate;
}

//--------------------------------------------------------------------------------------------------
/// FMU RFT observation files typically contain a single observation point per formation per
/// well/time step (one measured pressure point per zone), so the min/max MD among points tagged
/// with the formation is usually degenerate (minMd == maxMd). To produce a usable depth filter,
/// the formation's depth interval is instead extended halfway towards its neighboring observation
/// points (sorted by MD) for the same well/time step, giving contiguous, non-overlapping intervals
/// along the well. The outermost formation(s) extend to the first/last observation point MD.
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>>
    RifReaderFmuRft::formationDepthRange( const QString& wellName, const QDateTime& timeStep, const QString& formationName )
{
    std::vector<const Observation*> observationsForWellDate = sortedObservationsForWellDate( wellName, timeStep );
    if ( observationsForWellDate.empty() ) return std::nullopt;

    const QString baseFormationName = stripEstimatedFormationNameSuffix( formationName );

    std::vector<size_t> indicesForFormation;
    for ( size_t i = 0; i < observationsForWellDate.size(); i++ )
    {
        if ( observationsForWellDate[i]->location.formation == baseFormationName ) indicesForFormation.push_back( i );
    }

    if ( indicesForFormation.empty() ) return std::nullopt;

    const size_t firstIdx = indicesForFormation.front();
    const size_t lastIdx  = indicesForFormation.back();

    const double firstMd = observationsForWellDate[firstIdx]->location.mdrkb;
    const double lastMd  = observationsForWellDate[lastIdx]->location.mdrkb;

    double minMd = ( firstIdx == 0 ) ? firstMd : 0.5 * ( observationsForWellDate[firstIdx - 1]->location.mdrkb + firstMd );
    double maxMd =
        ( lastIdx == observationsForWellDate.size() - 1 ) ? lastMd : 0.5 * ( lastMd + observationsForWellDate[lastIdx + 1]->location.mdrkb );

    // Fall back to a small fixed padding if the formation is the only observation point for this
    // well/time step, so the resulting range is not degenerate (minMd == maxMd).
    if ( minMd == maxMd )
    {
        const double padding = 1.0;
        minMd -= padding;
        maxMd += padding;
    }

    return std::make_pair( minMd, maxMd );
}

//--------------------------------------------------------------------------------------------------
/// Linearly interpolates TVD (MSL) at a given MD (RKB) using the well/time step's observed
/// MD/TVD point pairs, sorted by MD. Clamps to the first/last observation point when the
/// requested MD falls outside the observed range.
//--------------------------------------------------------------------------------------------------
double RifReaderFmuRft::interpolateTvdFromMd( const std::vector<const Observation*>& sortedObservations, double md )
{
    if ( md <= sortedObservations.front()->location.mdrkb ) return sortedObservations.front()->location.tvdmsl;
    if ( md >= sortedObservations.back()->location.mdrkb ) return sortedObservations.back()->location.tvdmsl;

    for ( size_t i = 0; i + 1 < sortedObservations.size(); ++i )
    {
        const double md0 = sortedObservations[i]->location.mdrkb;
        const double md1 = sortedObservations[i + 1]->location.mdrkb;
        if ( md >= md0 && md <= md1 )
        {
            if ( md1 == md0 ) return sortedObservations[i]->location.tvdmsl;

            const double t = ( md - md0 ) / ( md1 - md0 );
            return sortedObservations[i]->location.tvdmsl +
                   t * ( sortedObservations[i + 1]->location.tvdmsl - sortedObservations[i]->location.tvdmsl );
        }
    }

    return sortedObservations.back()->location.tvdmsl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>>
    RifReaderFmuRft::convertMdRangeToTvd( const QString& wellName, const QDateTime& timeStep, double mdMin, double mdMax )
{
    // A single observation point still gives a valid (degenerate) TVD value: interpolateTvdFromMd()
    // clamps to that single point for any requested MD, so both ends of the returned range collapse
    // to the same TVD, consistent with the MD padding fallback used for single-point formations.
    std::vector<const Observation*> observationsForWellDate = sortedObservationsForWellDate( wellName, timeStep );
    if ( observationsForWellDate.empty() ) return std::nullopt;

    double tvdMin = interpolateTvdFromMd( observationsForWellDate, mdMin );
    double tvdMax = interpolateTvdFromMd( observationsForWellDate, mdMax );
    if ( tvdMin > tvdMax ) std::swap( tvdMin, tvdMax );

    return std::make_pair( tvdMin, tvdMax );
}

//--------------------------------------------------------------------------------------------------
/// Computes the mean observed pressure and mean observed pressure error for the given well/time
/// step, optionally restricted to observation points within [mdMin, mdMax] (MD RKB).
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>>
    RifReaderFmuRft::observedPressureAndError( const QString& wellName, const QDateTime& timeStep, bool useDepthRange, double mdMin, double mdMax )
{
    std::vector<const Observation*> observationsForWellDate = sortedObservationsForWellDate( wellName, timeStep );
    if ( observationsForWellDate.empty() ) return std::nullopt;

    double minMd = mdMin;
    double maxMd = mdMax;
    if ( minMd > maxMd ) std::swap( minMd, maxMd );

    double sumPressure      = 0.0;
    double sumPressureError = 0.0;
    int    count            = 0;

    for ( const auto* observation : observationsForWellDate )
    {
        if ( useDepthRange && ( observation->location.mdrkb < minMd || observation->location.mdrkb > maxMd ) ) continue;

        sumPressure += observation->pressure;
        sumPressureError += observation->pressureError;
        ++count;
    }

    if ( count == 0 ) return std::nullopt;

    return std::make_pair( sumPressure / count, sumPressureError / count );
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
        RiaLogging::error( errorMsg.toStdString() );
        return;
    }

    QDir dir( m_filePath );

    auto wellDates = importWellDates( dir.absoluteFilePath( RifReaderFmuRft::wellPathFileName() ) );
    if ( wellDates.empty() )
    {
        RiaLogging::error( std::format( "'{}' contains no valid FMU RFT data", m_filePath ) );
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
            QString txtFile = findFileName( wellName, "txt", measurementId, dir );
            if ( !QFile::exists( txtFile ) ) continue;

            auto locations = importLocations( dir.absoluteFilePath( txtFile ) );
            if ( locations.empty() ) continue;

            // The observation file name can be either <wellName>_<measurementId>.obs or <wellName>.obs
            QString observationFileName = findFileName( wellName, "obs", measurementId, dir );
            if ( !QFile::exists( observationFileName ) ) continue;

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
        RiaLogging::error( std::format( "{} cannot be found at '%s'", RifReaderFmuRft::wellPathFileName(), fileName ) );
        return {};
    }

    QFile wellDateFile( fileName );
    if ( !wellDateFile.open( QIODevice::Text | QIODevice::ReadOnly ) )
    {
        RiaLogging::error( std::format( "Could not read '{}'", fileName ) );
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
                RiaLogging::error( std::format( "Failed to parse '{}'", fileName ) );
                return {};
            }

            year  = dateWords[0].toInt();
            month = dateWords[1].toInt();
            day   = dateWords[2].toInt();

            measurementIndex = words[2].toInt();
        }
        else
        {
            RiaLogging::error( std::format( "Failed to parse '{}'", fileName ) );
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
        RiaLogging::error( std::format( "Could not open '{}'", fileName ) );
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
            RiaLogging::error( std::format( "Failed to parse '{}'", fileName ) );
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
        RiaLogging::error( std::format( "Could not open '{}'", fileName ) );
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
            RiaLogging::error( std::format( "'{}' has more lines than corresponding txt file", fileName ) );
            return {};
        }

        QTextStream lineStream( &line );

        double pressure, pressureError;

        lineStream >> pressure >> pressureError;

        if ( lineStream.status() != QTextStream::Ok )
        {
            RiaLogging::error( std::format( "Failed to parse line {} of '{}'", lineNumber + 1, fileName ) );
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
