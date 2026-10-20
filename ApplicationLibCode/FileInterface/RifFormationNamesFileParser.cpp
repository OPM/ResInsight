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

#include "RifFormationNamesFileParser.h"

#include "RiaLogging.h"
#include "RiaTextStringTools.h"
#include "RigFormationNames.h"

#include <QColor>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>

#include "cvfColor3.h"

namespace
{
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
std::expected<std::map<QString, cvf::Color3f>, QString> RifFormationNamesFileParser::readStratigraphyColors( const QString& fileName )
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
/// Reads the "zranges" section of a YAML file with one-based layer ranges per zone:
///
///   zranges:
///     - Valysar: [1, 20]
///     - Therys: [21, 54]
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString>
    RifFormationNamesFileParser::readYamlFormationNameFile( const QString& fileName, const std::map<QString, cvf::Color3f>& colors )
{
    QFile dataFile( fileName );
    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        return std::unexpected( "Could not open file: " + fileName );
    }

    static const QRegularExpression rangeExpression( R"re(^(?:-\s*)?(?:"([^"]+)"|'([^']+)'|([^:]+?))\s*:\s*\[\s*(\d+)\s*,\s*(\d+)\s*\]$)re" );

    RigFormationNames formationNames;
    QTextStream       stream( &dataFile );

    bool isInZranges  = false;
    bool foundZranges = false;
    int  lineNumber   = 0;

    while ( !stream.atEnd() )
    {
        QString line = stream.readLine();
        ++lineNumber;

        const int commentPos = line.indexOf( '#' );
        if ( commentPos >= 0 ) line.truncate( commentPos );

        const bool isIndented = line.startsWith( ' ' ) || line.startsWith( '\t' );
        line                  = line.trimmed();
        if ( line.isEmpty() ) continue;

        if ( !isIndented && !line.startsWith( '-' ) )
        {
            // Top level key
            isInZranges = line.startsWith( "zranges:" );
            if ( isInZranges ) foundZranges = true;
            continue;
        }

        if ( !isInZranges ) continue;

        const QRegularExpressionMatch match = rangeExpression.match( line );
        if ( !match.hasMatch() )
        {
            RiaLogging::warning( ( fileName + ": format error on line " + QString::number( lineNumber ) ).toStdString() );
            continue;
        }

        QString name;
        for ( int i = 1; i <= 3 && name.isEmpty(); i++ )
            name = match.captured( i ).trimmed();

        const int startK = match.captured( 4 ).toInt();
        const int endK   = match.captured( 5 ).toInt();
        if ( name.isEmpty() || startK < 1 || endK < startK )
        {
            RiaLogging::warning( ( fileName + ": invalid zone range on line " + QString::number( lineNumber ) ).toStdString() );
            continue;
        }

        if ( auto it = colors.find( name ); it != colors.end() )
            formationNames.appendFormationRange( name, it->second, startK - 1, endK - 1 );
        else
            formationNames.appendFormationRange( name, startK - 1, endK - 1 );
    }

    if ( !foundZranges )
    {
        return std::unexpected( QString( "No 'zranges' section found in '%1'" ).arg( fileName ) );
    }

    if ( formationNames.formationNames().empty() )
    {
        return std::unexpected( QString( "No valid zone ranges found in '%1'" ).arg( fileName ) );
    }

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString> RifFormationNamesFileParser::readLyrFormationNameFile( const QString& fileName )
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
                    if ( auto formationColor = convertStringToColor( colorWord ) )
                        formationNames.appendFormationRange( formationName, *formationColor, startK - 1, endK - 1 );
                    else
                        formationNames.appendFormationRange( formationName, startK - 1, endK - 1 );
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
                    if ( auto formationColor = convertStringToColor( colorWord ) )
                        formationNames.appendFormationRangeHeight( formationName, *formationColor, kLayerCount );
                    else
                        formationNames.appendFormationRangeHeight( formationName, kLayerCount );
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
std::expected<RigFormationNames, QString>
    RifFormationNamesFileParser::readFmuFormationNameFile( const QString& fileName, const std::map<QString, cvf::Color3f>& colors )
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
std::expected<cvf::Color3f, QString> RifFormationNamesFileParser::convertStringToColor( const QString& word )
{
    if ( word.isEmpty() ) return std::unexpected( "Empty color string" );

    QColor colorQ( word );

    if ( colorQ.isValid() )
    {
        return cvf::Color3f::fromByteColor( colorQ.red(), colorQ.green(), colorQ.blue() );
    }

    return std::unexpected( "Invalid color: " + word );
}
