/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include "RifFormationNamesReader.h"

#include "RiaLogging.h"
#include "RiaTextStringTools.h"
#include "RigFormationNames.h"

#include "cafAssert.h"
#include "cafPdmUiFilePathEditor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "cvfColor3.h"

namespace
{
// Typical FMU location, relative to the folder of the grid case file (e.g. realization-0/iter-0/eclipse/model):
//   ../../rms/output/zone/layer_zone_table.txt
//   ../../rms/output/zone/stratigraphy.json
// stratigraphy.json is read from the same folder as the layer_zone_table.txt that is imported.
const QString layerZoneTableFileName = "layer_zone_table.txt";
const QString stratigraphyFileName   = "stratigraphy.json";

//--------------------------------------------------------------------------------------------------
/// Entries with subzones are parents, all other entries are leaves. Within each group the first
/// occurrence of a name wins. Invalid entries are skipped with a warning.
//--------------------------------------------------------------------------------------------------
void collectStratigraphyColors( const QJsonArray&                entries,
                                const QString&                   fileName,
                                std::map<QString, cvf::Color3f>& leafColors,
                                std::map<QString, cvf::Color3f>& parentColors )
{
    for ( const QJsonValue& value : entries )
    {
        if ( !value.isObject() ) continue;

        const QJsonObject obj  = value.toObject();
        const QString     name = obj.value( "name" ).toString().trimmed();
        if ( name.isEmpty() )
        {
            RiaLogging::warning( fileName.toStdString() + ": skipping stratigraphy entry without name" );
            continue;
        }

        const QJsonArray subzones  = obj.value( "subzones" ).toArray();
        const bool       isParent  = !subzones.isEmpty();
        auto&            targetMap = isParent ? parentColors : leafColors;
        const QString    colorText = obj.value( "color" ).toString().trimmed();
        const QColor     color( colorText );

        if ( !color.isValid() )
        {
            RiaLogging::warning( fileName.toStdString() + ": invalid color '" + colorText.toStdString() + "' for '" + name.toStdString() + "'" );
        }
        else
        {
            targetMap.emplace( name, cvf::Color3f::fromByteColor( color.red(), color.green(), color.blue() ) );
        }

        if ( isParent ) collectStratigraphyColors( subzones, fileName, leafColors, parentColors );
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::map<QString, cvf::Color3f>, QString> RifFormationNamesReader::readStratigraphyColors( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( QFile::ReadOnly ) )
    {
        return std::unexpected( "Could not open file: " + fileName );
    }

    QJsonParseError     parseError;
    const QJsonDocument doc = QJsonDocument::fromJson( file.readAll(), &parseError );
    if ( doc.isNull() )
    {
        return std::unexpected( QString( "Invalid JSON in '%1': %2" ).arg( fileName, parseError.errorString() ) );
    }
    if ( !doc.isArray() )
    {
        return std::unexpected( QString( "Expected a JSON array at the root of '%1'" ).arg( fileName ) );
    }

    std::map<QString, cvf::Color3f> leafColors;
    std::map<QString, cvf::Color3f> parentColors;
    collectStratigraphyColors( doc.array(), fileName, leafColors, parentColors );

    // Zone colors are used only as fallback when no subzone has the same name
    leafColors.insert( parentColors.begin(), parentColors.end() );

    return leafColors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString> RifFormationNamesReader::readFormationNamesFile( const QString& fileName )
{
    QFileInfo fileInfo( fileName );

    if ( fileInfo.fileName() == layerZoneTableFileName )
    {
        std::map<QString, cvf::Color3f> colors;

        const QString colorFile = fileInfo.dir().absoluteFilePath( stratigraphyFileName );
        if ( QFile::exists( colorFile ) )
        {
            if ( auto result = readStratigraphyColors( colorFile ); result )
            {
                colors = std::move( *result );
            }
            else
            {
                RiaLogging::warning( "Ignoring zone colors: " + result.error().toStdString() );
            }
        }

        return RifFormationNamesReader::readFmuFormationNameFile( fileName, colors );
    }
    else
    {
        return RifFormationNamesReader::readLyrFormationNameFile( fileName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString> RifFormationNamesReader::readLyrFormationNameFile( const QString& fileName )
{
    QFile dataFile( fileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        return std::unexpected( "Could not open file: " + fileName );
    }

    RigFormationNames formationNames;
    QTextStream       stream( &dataFile );

    int lineNumber = 1;
    while ( !stream.atEnd() )
    {
        QString     line     = stream.readLine();
        QStringList lineSegs = line.split( "'" );

        if ( lineSegs.empty() ) continue; // Empty line
        if ( lineSegs.size() == 1 ) continue; // No name present. Comment line ?
        if ( lineSegs.size() == 2 )
        {
            RiaLogging::warning( ( fileName + ": missing quote on line " + QString::number( lineNumber ) ).toStdString() );
            continue; // One quote present
        }

        if ( lineSegs.size() == 3 ) // Normal case
        {
            if ( lineSegs[0].contains( "--" ) ) continue; // Comment line
            QString formationName  = lineSegs[1];
            int     commentMarkPos = lineSegs[2].indexOf( "--" );
            QString numberString   = lineSegs[2];
            if ( commentMarkPos >= 0 ) numberString.truncate( commentMarkPos );

            // extract last word which may contain formation color
            QString colorWord = RiaTextStringTools::splitSkipEmptyParts( numberString ).last();

            if ( QColor::isValidColorName( colorWord ) ) numberString.remove( colorWord ); // remove color if present as last word on line

            // extract words containing formation number(s)
            QStringList numberWords = RiaTextStringTools::splitSkipEmptyParts( numberString, QRegularExpression( "-" ) );

            if ( numberWords.size() == 2 ) // formation range with or without color at end of line
            {
                bool isNumber1 = false;
                bool isNumber2 = false;
                int  startK    = numberWords[0].toInt( &isNumber1 );
                int  endK      = numberWords[1].toInt( &isNumber2 );

                if ( !( isNumber2 && isNumber1 ) )
                {
                    RiaLogging::warning( ( fileName + ": format error on line " + QString::number( lineNumber ) ).toStdString() );
                    continue;
                }

                int tmp = startK;
                startK  = tmp < endK ? tmp : endK;
                endK    = tmp > endK ? tmp : endK;

                if ( QColor::isValidColorName( colorWord ) ) // formation color present at end of line
                {
                    cvf::Color3f formationColor;

                    convertStringToColor( colorWord, &formationColor );
                    formationNames.appendFormationRange( formationName, formationColor, startK - 1, endK - 1 );
                }
                else // no color present
                {
                    formationNames.appendFormationRange( formationName, startK - 1, endK - 1 );
                }
            }
            else if ( numberWords.size() == 1 )
            {
                bool isNumber1   = false;
                int  kLayerCount = numberWords[0].toInt( &isNumber1 );

                if ( !isNumber1 )
                {
                    RiaLogging::warning( ( fileName + ": format error on line " + QString::number( lineNumber ) ).toStdString() );
                    continue;
                }

                if ( QColor::isValidColorName( colorWord ) ) // formation color present at end of line
                {
                    cvf::Color3f formationColor;

                    convertStringToColor( colorWord, &formationColor );
                    formationNames.appendFormationRangeHeight( formationName, formationColor, kLayerCount );
                }
                else // no color present
                {
                    formationNames.appendFormationRangeHeight( formationName, kLayerCount );
                }
            }
            else
            {
                RiaLogging::warning( ( fileName + ": format error on line " + QString::number( lineNumber ) ).toStdString() );
            }
        }

        ++lineNumber;
    }

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString> RifFormationNamesReader::readFmuFormationNameFile( const QString& fileName,
                                                                                             const std::map<QString, cvf::Color3f>& colors )
{
    QFile dataFile( fileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        return std::unexpected( "Could not open file: " + fileName );
    }

    RigFormationNames formationNames;
    QTextStream       stream( &dataFile );

    auto appendRange = [&]( const QString& name, int start, int end )
    {
        if ( auto it = colors.find( name.trimmed() ); it != colors.end() )
            formationNames.appendFormationRange( name, it->second, start, end );
        else
            formationNames.appendFormationRange( name, start, end );
    };
    int lineNumber = 1;

    QString currentFormationName;
    int     startK = -1;
    int     endK   = -1;

    while ( !stream.atEnd() )
    {
        QString line = stream.readLine();
        if ( line.isNull() )
        {
            // Make sure we append the last formation
            if ( !currentFormationName.isEmpty() )
            {
                appendRange( currentFormationName, startK - 1, endK - 1 );
            }
            break;
        }
        else
        {
            QTextStream lineStream( &line );

            double  kLayer;
            QString formationName;

            lineStream >> kLayer >> formationName;

            if ( lineStream.status() != QTextStream::Ok )
            {
                return std::unexpected( QString( "Failed to parse line %1 of '%2'" ).arg( lineNumber ).arg( fileName ) );
            }

            if ( formationName != currentFormationName )
            {
                // Append previous formation
                if ( !currentFormationName.isEmpty() )
                {
                    appendRange( currentFormationName, startK - 1, endK - 1 );
                }

                // Start new formation
                currentFormationName = formationName;
                startK               = kLayer;
                endK                 = kLayer;
            }
            else
            {
                endK = kLayer;
            }
        }
    }

    // Append previous formation at the end of the stream
    if ( !currentFormationName.isEmpty() )
    {
        appendRange( currentFormationName, startK - 1, endK - 1 );
    }

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
/// Conversion of a color specification from string representation to Color3f.
/// String can be in various formats according to QColor capabilities, notably
/// SVG color keyword names, c.f. https://www.w3.org/TR/SVG11/types.html#ColorKeywords,
/// or #RRGGBB used on a LYR formation data file.
//--------------------------------------------------------------------------------------------------
bool RifFormationNamesReader::convertStringToColor( const QString& word, cvf::Color3f* color )
{
    if ( word.isEmpty() ) return false;

    QColor colorQ( word );

    if ( colorQ.isValid() )
    {
        *color = cvf::Color3f::fromByteColor( colorQ.red(), colorQ.green(), colorQ.blue() );

        return true;
    }

    return false;
}
