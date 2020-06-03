/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RifWellPathImporter.h"

#include "RifJsonEncodeDecode.h"

#include <fstream>

#include "cafUtils.h"

#include <QFileInfo>

#include <algorithm>
#include <cmath>

#define ASCII_FILE_DEFAULT_START_INDEX 0

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellData RifWellPathImporter::readWellData( const QString& filePath, size_t indexInFile )
{
    CVF_ASSERT( caf::Utils::fileExists( filePath ) );

    if ( isJsonFile( filePath ) )
    {
        return readJsonWellData( filePath );
    }
    else
    {
        return readAsciiWellData( filePath, indexInFile );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellData RifWellPathImporter::readWellData( const QString& filePath )
{
    return readWellData( filePath, ASCII_FILE_DEFAULT_START_INDEX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellMetaData RifWellPathImporter::readWellMetaData( const QString& filePath, size_t indexInFile )
{
    CVF_ASSERT( caf::Utils::fileExists( filePath ) );

    if ( isJsonFile( filePath ) )
    {
        return readJsonWellMetaData( filePath );
    }
    else
    {
        return readAsciiWellMetaData( filePath, indexInFile );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellMetaData RifWellPathImporter::readWellMetaData( const QString& filePath )
{
    return readWellMetaData( filePath, ASCII_FILE_DEFAULT_START_INDEX );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifWellPathImporter::wellDataCount( const QString& filePath )
{
    if ( isJsonFile( filePath ) )
    {
        // Only support JSON files with single well data currently
        return 1;
    }
    else
    {
        std::map<QString, std::vector<RifWellPathImporter::WellData>>::iterator it =
            m_fileNameToWellDataGroupMap.find( filePath );

        // If we have the file in the map, assume it is already read.
        if ( it != m_fileNameToWellDataGroupMap.end() )
        {
            return it->second.size();
        }

        readAllAsciiWellData( filePath );
        it = m_fileNameToWellDataGroupMap.find( filePath );
        CVF_ASSERT( it != m_fileNameToWellDataGroupMap.end() );

        return it->second.size();
        ;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifWellPathImporter::isJsonFile( const QString& filePath )
{
    QFileInfo fileInfo( filePath );

    if ( fileInfo.suffix().compare( "json" ) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellMetaData RifWellPathImporter::readJsonWellMetaData( const QString& filePath )
{
    ResInsightInternalJson::JsonReader jsonReader;
    QMap<QString, QVariant>            jsonMap = jsonReader.decodeFile( filePath );
    WellMetaData                       metadata;

    metadata.m_id           = jsonMap["id"].toString();
    metadata.m_name         = jsonMap["name"].toString();
    metadata.m_sourceSystem = jsonMap["sourceSystem"].toString();
    metadata.m_utmZone      = jsonMap["utmZone"].toString();
    metadata.m_updateUser   = jsonMap["updateUser"].toString();
    metadata.m_surveyType   = jsonMap["surveyType"].toString();

    // Convert updateDate from the following format:
    // "Number of milliseconds elapsed since midnight Coordinated Universal Time (UTC)
    // of January 1, 1970, not counting leap seconds"
    QString updateDateStr = jsonMap["updateDate"].toString().trimmed();
    uint updateDateUint   = updateDateStr.toULongLong() / 1000; // Should be within 32 bit, maximum number is 4294967295
                                                              // which corresponds to year 2106
    metadata.m_updateDate.setTime_t( updateDateUint );

    return metadata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellData RifWellPathImporter::readJsonWellData( const QString& filePath )
{
    ResInsightInternalJson::JsonReader jsonReader;
    QMap<QString, QVariant>            jsonMap = jsonReader.decodeFile( filePath );

    double          datumElevation = jsonMap["datumElevation"].toDouble();
    QList<QVariant> pathList       = jsonMap["path"].toList();
    WellData        wellData;
    wellData.m_wellPathGeometry = new RigWellPath;
    wellData.m_wellPathGeometry->setDatumElevation( datumElevation );
    wellData.m_name = jsonMap["name"].toString();

    foreach ( QVariant point, pathList )
    {
        QMap<QString, QVariant> coordinateMap = point.toMap();
        cvf::Vec3d              vec3d( coordinateMap["east"].toDouble(),
                          coordinateMap["north"].toDouble(),
                          -( coordinateMap["tvd"].toDouble() - datumElevation ) );
        wellData.m_wellPathGeometry->m_wellPathPoints.push_back( vec3d );
        double measuredDepth = coordinateMap["md"].toDouble();
        wellData.m_wellPathGeometry->m_measuredDepths.push_back( measuredDepth );
    }
    return wellData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathImporter::readAllAsciiWellData( const QString& filePath )
{
    std::map<QString, std::vector<RifWellPathImporter::WellData>>::iterator it =
        m_fileNameToWellDataGroupMap.find( filePath );

    // If we have the file in the map, assume it is already read.
    if ( it != m_fileNameToWellDataGroupMap.end() )
    {
        return;
    }

    // Create the data container
    std::vector<RifWellPathImporter::WellData>& fileWellDataArray = m_fileNameToWellDataGroupMap[filePath];

    std::ifstream stream( filePath.toLatin1().data() );

    bool hasReadWellPointInCurrentWell = false;

    while ( stream.good() )
    {
        double x( HUGE_VAL ), y( HUGE_VAL ), tvd( HUGE_VAL ), md( HUGE_VAL );

        // First check if we can read a number
        stream >> x;
        if ( stream.good() ) // If we can, assume this line is a well point entry
        {
            stream >> y >> tvd >> md;

            if ( x != HUGE_VAL && y != HUGE_VAL && tvd != HUGE_VAL && md != HUGE_VAL )
            {
                if ( !fileWellDataArray.size() )
                {
                    fileWellDataArray.push_back( RifWellPathImporter::WellData() );
                    fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();
                }

                cvf::Vec3d wellPoint( x, y, -tvd );
                fileWellDataArray.back().m_wellPathGeometry->m_wellPathPoints.push_back( wellPoint );
                fileWellDataArray.back().m_wellPathGeometry->m_measuredDepths.push_back( md );

                hasReadWellPointInCurrentWell = true;
            }

            if ( !stream.good() )
            {
                // -999 or otherwise to few numbers before some word
                if ( x != -999 )
                {
                    // Error in file: missing numbers at this line
                }
                stream.clear();
            }
        }
        else
        {
            // Could not read one double.
            // we assume there is a comment line or a well path description
            stream.clear();

            std::string line;
            std::getline( stream, line, '\n' );
            // Skip possible comment lines (-- is used in eclipse, so Haakon Høgstøl considered it smart to skip these
            // here as well) The first "-" is eaten by the stream >> x above
            if ( line.find( "-" ) == 0 || line.find( "#" ) == 0 )
            {
                // Comment line, just ignore
            }
            else
            {
                // Find the first and the last position of any quotes (and do not care to match quotes)
                size_t quoteStartIdx = line.find_first_of( "'`´’‘" );
                size_t quoteEndIdx   = line.find_last_of( "'`´’‘" );

                std::string wellName;
                bool        haveAPossibleWellStart = false;

                if ( quoteStartIdx < line.size() - 1 )
                {
                    // Extract the text between the quotes
                    wellName               = line.substr( quoteStartIdx + 1, quoteEndIdx - 1 - quoteStartIdx );
                    haveAPossibleWellStart = true;
                }
                else if ( quoteStartIdx > line.length() )
                {
                    // We did not find any quotes

                    // Supported alternatives are
                    // name <WellNameA>
                    // wellname: <WellNameA>
                    std::string lineLowerCase = line;
                    transform( lineLowerCase.begin(), lineLowerCase.end(), lineLowerCase.begin(), []( const char c ) -> char {
                        return (char)::tolower( c );
                    } );

                    std::string tokenName    = "name";
                    std::size_t foundNameIdx = lineLowerCase.find( tokenName );
                    if ( foundNameIdx != std::string::npos )
                    {
                        std::string tokenColon    = ":";
                        std::size_t foundColonIdx = lineLowerCase.find( tokenColon, foundNameIdx );
                        if ( foundColonIdx != std::string::npos )
                        {
                            wellName = line.substr( foundColonIdx + tokenColon.length() );
                        }
                        else
                        {
                            wellName = line.substr( foundNameIdx + tokenName.length() );
                        }

                        haveAPossibleWellStart = true;
                    }
                    else
                    {
                        // Interpret the whole line as the well name.

                        QString name = line.c_str();
                        if ( !name.trimmed().isEmpty() )
                        {
                            wellName               = name.trimmed().toStdString();
                            haveAPossibleWellStart = true;
                        }
                    }
                }

                if ( haveAPossibleWellStart )
                {
                    // Create a new Well data if we have read some data into the previous one.
                    // if not, just overwrite the name
                    if ( hasReadWellPointInCurrentWell || fileWellDataArray.size() == 0 )
                    {
                        fileWellDataArray.push_back( RifWellPathImporter::WellData() );
                        fileWellDataArray.back().m_wellPathGeometry = new RigWellPath();
                    }

                    QString name = wellName.c_str();
                    if ( !name.trimmed().isEmpty() )
                    {
                        // Do not overwrite the name acquired from a line above, if this line is empty
                        fileWellDataArray.back().m_name = name.trimmed();
                    }
                    hasReadWellPointInCurrentWell = false;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellData RifWellPathImporter::readAsciiWellData( const QString& filePath, size_t indexInFile )
{
    readAllAsciiWellData( filePath );

    std::map<QString, std::vector<RifWellPathImporter::WellData>>::iterator it =
        m_fileNameToWellDataGroupMap.find( filePath );

    CVF_ASSERT( it != m_fileNameToWellDataGroupMap.end() );

    if ( indexInFile < it->second.size() )
    {
        return it->second[indexInFile];
    }
    else
    {
        // Error : The ascii well path file does not contain that many well paths
        return RifWellPathImporter::WellData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellMetaData RifWellPathImporter::readAsciiWellMetaData( const QString& filePath, size_t indexInFile )
{
    // No metadata in ASCII files
    return WellMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathImporter::clear()
{
    m_fileNameToWellDataGroupMap.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathImporter::removeFilePath( const QString& filePath )
{
    m_fileNameToWellDataGroupMap.erase( filePath );
}
