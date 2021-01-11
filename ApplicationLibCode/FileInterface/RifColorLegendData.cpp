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

#include "RifColorLegendData.h"

#include "RigFormationNames.h"

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"

#include "RiaColorTables.h"

#include "cafAssert.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFile>
#include <QFileInfo>

#include "cvfColor3.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
    RifColorLegendData::readFormationNamesFile( const QString& fileName, QString* errorMessage )
{
    QFileInfo fileInfo( fileName );

    if ( fileInfo.fileName() == "layer_zone_table.txt" )
    {
        return RifColorLegendData::readFmuFormationNameFile( fileName, errorMessage );
    }
    else
    {
        return RifColorLegendData::readLyrFormationNameFile( fileName, errorMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
    RifColorLegendData::readLyrFormationNameFile( const QString& fileName, QString* errorMessage )
{
    cvf::ref<RigFormationNames>     formationNames = new RigFormationNames;
    caf::PdmPointer<RimColorLegend> colorLegend    = new RimColorLegend;

    QFile dataFile( fileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        if ( errorMessage ) ( *errorMessage ) += "Could not open file: " + fileName + "\n";
        return std::make_pair( formationNames, colorLegend );
    }

    QString colorLegendName = QFileInfo( fileName ).baseName();
    colorLegend->setColorLegendName( colorLegendName );

    QTextStream stream( &dataFile );

    int colorIndex = 0;
    int lineNumber = 1;
    while ( !stream.atEnd() )
    {
        QString     line     = stream.readLine();
        QStringList lineSegs = line.split( "'", QString::KeepEmptyParts );

        if ( lineSegs.size() == 0 ) continue; // Empty line
        if ( lineSegs.size() == 1 ) continue; // No name present. Comment line ?
        if ( lineSegs.size() == 2 )
        {
            if ( errorMessage ) ( *errorMessage ) += "Missing quote on line : " + QString::number( lineNumber ) + "\n";
            continue; // One quote present
        }

        if ( lineSegs.size() == 3 ) // Normal case
        {
            if ( lineSegs[0].contains( "--" ) ) continue; // Comment line
            QString formationName  = lineSegs[1];
            int     commentMarkPos = lineSegs[2].indexOf( "--" );
            QString numberString   = lineSegs[2];
            if ( commentMarkPos >= 0 ) numberString.truncate( commentMarkPos );

            QString colorWord = numberString.split( " ", QString::SkipEmptyParts ).last(); // extract last word which may
                                                                                           // contain formation color

            if ( QColor::isValidColor( colorWord ) )
                numberString.remove( colorWord ); // remove color if present as last word on line

            QStringList numberWords = numberString.split( QRegExp( "-" ), QString::SkipEmptyParts ); // extract words
                                                                                                     // containing
                                                                                                     // formation
                                                                                                     // number(s)

            if ( numberWords.size() == 2 ) // formation range with or without color at end of line
            {
                bool isNumber1 = false;
                bool isNumber2 = false;
                int  startK    = numberWords[0].toInt( &isNumber1 );
                int  endK      = numberWords[1].toInt( &isNumber2 );

                if ( !( isNumber2 && isNumber1 ) )
                {
                    if ( errorMessage )
                        ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
                    continue;
                }

                int tmp = startK;
                startK  = tmp < endK ? tmp : endK;
                endK    = tmp > endK ? tmp : endK;

                cvf::Color3f formationColor = RiaColorTables::categoryPaletteColors().cycledColor3f( colorIndex );
                colorIndex++;

                // Formation color present at end of line
                if ( QColor::isValidColor( colorWord ) )
                {
                    cvf::Color3f fileColor;
                    bool         colorOk = convertStringToColor( colorWord, &fileColor );
                    if ( colorOk ) formationColor = fileColor;
                }
                formationNames->appendFormationRange( formationName, startK - 1, endK - 1 );

                RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
                colorLegendItem->setValues( formationName, colorIndex, formationColor );
                colorLegend->appendColorLegendItem( colorLegendItem );
            }
            else if ( numberWords.size() == 1 )
            {
                bool isNumber1   = false;
                int  kLayerCount = numberWords[0].toInt( &isNumber1 );

                if ( !isNumber1 )
                {
                    if ( errorMessage )
                        ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
                    continue;
                }

                cvf::Color3f formationColor = RiaColorTables::categoryPaletteColors().cycledColor3f( colorIndex );
                colorIndex++;
                // Formation color present at end of line
                if ( QColor::isValidColor( colorWord ) )
                {
                    cvf::Color3f fileColor;
                    bool         colorOk = convertStringToColor( colorWord, &fileColor );
                    if ( colorOk ) formationColor = fileColor;
                }
                formationNames->appendFormationRangeHeight( formationName, kLayerCount );

                RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
                colorLegendItem->setValues( formationName, colorIndex, formationColor );
                colorLegend->appendColorLegendItem( colorLegendItem );
            }
            else
            {
                if ( errorMessage )
                    ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
            }
        }

        ++lineNumber;
    }

    return std::make_pair( formationNames, colorLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
    RifColorLegendData::readFmuFormationNameFile( const QString& fileName, QString* errorMessage )
{
    cvf::ref<RigFormationNames>     formationNames = new RigFormationNames;
    caf::PdmPointer<RimColorLegend> colorLegend;

    QFile dataFile( fileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        if ( errorMessage ) ( *errorMessage ) += "Could not open file: " + fileName + "\n";
        return std::make_pair( formationNames, colorLegend );
    }

    QTextStream stream( &dataFile );

    int lineNumber = 1;

    QString currentFormationName;
    int     startK = -1;
    int     endK   = -1;
    int colorIndex = 0;
    
    while ( !stream.atEnd() )
    {
        QString line = stream.readLine();
        if ( line.isNull() )
        {
            // Make sure we append the last formation
            if ( !currentFormationName.isEmpty() )
            {
                formationNames->appendFormationRange( currentFormationName, startK - 1, endK - 1 );
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
                *errorMessage = QString( "Failed to parse line %1 of '%2'" ).arg( lineNumber ).arg( fileName );
                return std::make_pair( formationNames, colorLegend );
            }

            if ( formationName != currentFormationName )
            {
                // Append previous formation
                if ( !currentFormationName.isEmpty() )
                {
                    formationNames->appendFormationRange( currentFormationName, startK - 1, endK - 1 );

                    cvf::Color3f formationColor = RiaColorTables::categoryPaletteColors().cycledColor3f( colorIndex );
                    RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
                    colorLegendItem->setValues( currentFormationName, colorIndex, formationColor );
                    colorLegend->appendColorLegendItem( colorLegendItem );
                    colorIndex++;
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
        formationNames->appendFormationRange( currentFormationName, startK - 1, endK - 1 );

        cvf::Color3f        formationColor  = RiaColorTables::categoryPaletteColors().cycledColor3f( colorIndex );
        RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
        colorLegendItem->setValues( currentFormationName, colorIndex, formationColor );
        colorLegend->appendColorLegendItem( colorLegendItem );
        colorIndex++;
    }

    return std::make_pair( formationNames, colorLegend );
}

//--------------------------------------------------------------------------------------------------
/// Conversion of a color specification from string representation to Color3f.
/// String can be in various formats according to QColor capabilities, notably
/// SVG color keyword names, c.f. https://www.w3.org/TR/SVG11/types.html#ColorKeywords,
/// or #RRGGBB used on a LYR formation data file.
//--------------------------------------------------------------------------------------------------
bool RifColorLegendData::convertStringToColor( const QString& word, cvf::Color3f* color )
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
