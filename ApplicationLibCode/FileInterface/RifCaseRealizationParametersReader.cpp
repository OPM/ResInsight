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

#include "RifCaseRealizationParametersReader.h"
#include "RifFileParseTools.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include <QDir>
#include <QString>
#include <QStringList>

#include <functional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::RifCaseRealizationReader( const QString& fileName )
{
    m_parameters = std::shared_ptr<RigCaseRealizationParameters>( new RigCaseRealizationParameters() );
    m_fileName   = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::~RifCaseRealizationReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::shared_ptr<RigCaseRealizationParameters> RifCaseRealizationReader::parameters() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RifCaseRealizationReader> RifCaseRealizationReader::createReaderFromFileName( const QString& fileName )
{
    std::shared_ptr<RifCaseRealizationReader> reader;

    if ( fileName.endsWith( parametersFileName() ) )
    {
        reader.reset( new RifCaseRealizationParametersReader( fileName ) );
    }
    else if ( fileName.endsWith( runSpecificationFileName() ) )
    {
        reader.reset( new RifCaseRealizationRunspecificationReader( fileName ) );
    }
    return reader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationReader::parametersFileName()
{
    return "parameters.txt";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationReader::runSpecificationFileName()
{
    return "runspecification.xml";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::RifCaseRealizationParametersReader( const QString& fileName )
    : RifCaseRealizationReader( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::~RifCaseRealizationParametersReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::parse()
{
    QFile file( m_fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    QTextStream dataStream( &file );

    int         lineNo = 0;
    QStringList errors;

    while ( !dataStream.atEnd() )
    {
        QString line = dataStream.readLine();

        lineNo++;
        QStringList cols = RifFileParseTools::splitLineAndTrim( line, QRegExp( "[ \t]" ), true );

        if ( cols.size() != 2 )
        {
            errors << QString( "RifEnsembleParametersReader: Invalid file format in line %1" ).arg( lineNo );

            continue;
        }

        QString& name     = cols[0];
        QString& strValue = cols[1];

        if ( RiaStdStringTools::isNumber( strValue.toStdString(), QLocale::c().decimalPoint().toLatin1() ) )
        {
            bool   parseOk = true;
            double value   = QLocale::c().toDouble( strValue, &parseOk );
            if ( parseOk )
            {
                m_parameters->addParameter( name, value );
            }
            else
            {
                errors << QString( "RifEnsembleParametersReader: Invalid number format in line %1" ).arg( lineNo );
            }
        }
        else
        {
            m_parameters->addParameter( name, strValue );
        }
    }

    for ( const auto& s : errors )
    {
        RiaLogging::warning( s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationRunspecificationReader::RifCaseRealizationRunspecificationReader( const QString& fileName )
    : RifCaseRealizationReader( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationRunspecificationReader::~RifCaseRealizationRunspecificationReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationRunspecificationReader::parse()
{
    QFile file( m_fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    QXmlStreamReader xml( &file );

    QStringList errors;

    QString paramName;

    while ( !xml.atEnd() )
    {
        xml.readNext();

        if ( xml.isStartElement() )
        {
            if ( xml.name() == "modifier" )
            {
                paramName = "";
            }

            if ( xml.name() == "id" )
            {
                paramName = xml.readElementText();
            }

            if ( xml.name() == "value" )
            {
                QString paramStrValue = xml.readElementText();

                if ( paramName.isEmpty() ) continue;

                if ( RiaStdStringTools::isNumber( paramStrValue.toStdString(), QLocale::c().decimalPoint().toLatin1() ) )
                {
                    bool   parseOk = true;
                    double value   = QLocale::c().toDouble( paramStrValue, &parseOk );
                    if ( parseOk )
                    {
                        m_parameters->addParameter( paramName, value );
                    }
                    else
                    {
                        errors
                            << QString( "RifCaseRealizationRunspecificationReader: Invalid number format in line %1" )
                                   .arg( xml.lineNumber() );
                    }
                }
                else
                {
                    m_parameters->addParameter( paramName, paramStrValue );
                }
            }
        }
        else if ( xml.isEndElement() )
        {
            if ( xml.name() == "modifier" )
            {
                paramName = "";
            }
        }
    }

    for ( const auto& s : errors )
    {
        RiaLogging::warning( s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationParametersFileLocator::locate( const QString& modelPath )
{
    int MAX_LEVELS_UP = 3;
    int dirLevel      = 0;

    QDir qdir( modelPath );

    const QFileInfo dir( modelPath );
    if ( dir.isFile() )
        qdir.cdUp();
    else if ( !dir.isDir() )
        return "";

    do
    {
        QStringList files = qdir.entryList( QDir::Files | QDir::NoDotAndDotDot );
        for ( const QString& file : files )
        {
            if ( QString::compare( file, RifCaseRealizationReader::parametersFileName(), Qt::CaseInsensitive ) == 0 ||
                 QString::compare( file, RifCaseRealizationReader::runSpecificationFileName(), Qt::CaseInsensitive ) == 0 )
            {
                return qdir.absoluteFilePath( file );
            }
        }
        qdir.cdUp();

    } while ( dirLevel++ < MAX_LEVELS_UP );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifCaseRealizationParametersFileLocator::realizationNumber( const QString& modelPath )
{
    QDir    dir( modelPath );
    QString absolutePath = dir.absolutePath();

    int resultIndex = -1;

    // Use parenthesis to indicate capture of sub string
    QString pattern = "(realization-\\d+)";

    QRegExp regexp( pattern, Qt::CaseInsensitive );
    if ( regexp.indexIn( absolutePath ) )
    {
        QString tempText = regexp.cap( 1 );

        QRegExp rx( "(\\d+)" ); // Find number
        int     digitPos = rx.indexIn( tempText );
        if ( digitPos > -1 )
        {
            resultIndex = rx.cap( 0 ).toInt();
        }
    }

    return resultIndex;
}
